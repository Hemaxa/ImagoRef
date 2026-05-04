#include "AuthController.h"
#include "SettingsManager.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJSEngine>
#include <QFile>
#include <QImage>
#include <QBuffer>
#include <QCryptographicHash>
#include "CacheManager.h"
#include <QUrl>

AuthController* AuthController::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    auto *inst = &instance();
    QJSEngine::setObjectOwnership(inst, QJSEngine::CppOwnership);
    return inst;
}

AuthController& AuthController::instance()
{
    static AuthController controller;
    return controller;
}

AuthController::AuthController(QObject* parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this))
{
}

bool AuthController::getIsLoggedIn() const
{
    return !SettingsManager::instance().getJwtToken().isEmpty();
}

QString AuthController::getUserEmail() const
{
    return SettingsManager::instance().getUserEmail();
}

QString AuthController::getUserNickname() const
{
    return SettingsManager::instance().getUserNickname();
}

QString AuthController::getUserAvatarHash() const
{
    return SettingsManager::instance().getUserAvatarHash();
}

void AuthController::updateProfile(const QString &nickname, const QString &avatarFilePath)
{
    QString filePath = avatarFilePath;
    if (filePath.startsWith("file://")) {
        filePath = QUrl(avatarFilePath).toLocalFile();
    }

    if (filePath.isEmpty()) {
        QJsonObject reqObj;
        reqObj["nickname"] = nickname;
        reqObj["avatar_hash"] = getUserAvatarHash();

        QNetworkRequest request(QUrl(API_BASE_URL + "/users/me"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QString token = SettingsManager::instance().getJwtToken();
        if (!token.isEmpty()) {
            request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
        }

        QNetworkReply *reply = m_networkManager->put(request, QJsonDocument(reqObj).toJson());
        
        connect(reply, &QNetworkReply::finished, this, [this, reply, nickname]() {
            reply->deleteLater();
            if (reply->error() == QNetworkReply::NoError) {
                SettingsManager::instance().setUserNickname(nickname);
                emit authStateChanged();
            }
        });
        return;
    }

    QImage image;
    if (!image.load(filePath)) {
        return;
    }
    
    QByteArray pngData;
    QBuffer buffer(&pngData);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    
    QByteArray hashBytes = QCryptographicHash::hash(pngData, QCryptographicHash::Md5);
    QString hash = hashBytes.toHex();
    
    CacheManager::instance().saveToCache(hash, pngData);
    
    QNetworkRequest request(QUrl(API_BASE_URL + "/users/me/avatar_url"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QString token = SettingsManager::instance().getJwtToken();
    if (!token.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    }
    
    QJsonObject urlReqObj;
    // --- ВОТ ЭТА СТРОКА ИСПРАВЛЯЕТ ВСЁ ---
    urlReqObj["image_hash"] = hash; 
    // -------------------------------------
    
    QNetworkReply *urlReply = m_networkManager->post(request, QJsonDocument(urlReqObj).toJson());
    
    connect(urlReply, &QNetworkReply::finished, this, [this, urlReply, pngData, hash, nickname]() {
        urlReply->deleteLater();
        if (urlReply->error() != QNetworkReply::NoError) {
            return;
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(urlReply->readAll());
        QString uploadUrl = doc.object()["upload_url"].toString();
        
        auto finishProfilePut = [this, nickname, hash]() {
            QJsonObject reqObj;
            reqObj["nickname"] = nickname;
            reqObj["avatar_hash"] = hash;

            QNetworkRequest req(QUrl(API_BASE_URL + "/users/me"));
            req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            QString token = SettingsManager::instance().getJwtToken();
            if (!token.isEmpty()) {
                req.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
            }

            QNetworkReply *reply = m_networkManager->put(req, QJsonDocument(reqObj).toJson());
            connect(reply, &QNetworkReply::finished, this, [this, reply, nickname, hash]() {
                reply->deleteLater();
                if (reply->error() == QNetworkReply::NoError) {
                    SettingsManager::instance().setUserNickname(nickname);
                    SettingsManager::instance().setUserAvatarHash(hash);
                    emit authStateChanged();
                }
            });
        };

        if (!uploadUrl.isEmpty()) {
            QNetworkRequest s3Req((QUrl(uploadUrl))); // Убрали StrictMode
            s3Req.setHeader(QNetworkRequest::ContentTypeHeader, "image/png");
            s3Req.setHeader(QNetworkRequest::ContentLengthHeader, pngData.size());
            
            QNetworkReply *s3Reply = m_networkManager->put(s3Req, pngData);
            connect(s3Reply, &QNetworkReply::finished, this, [s3Reply, finishProfilePut]() {
                if (s3Reply->error() == QNetworkReply::NoError) {
                    qDebug() << "Avatar uploaded to S3 successfully!";
                } else {
                    qWarning() << "Failed to upload avatar to S3:" << s3Reply->errorString();
                }
                
                // ВАЖНО: Вызываем сохранение профиля на сервере в любом случае!
                // Даже если аватарка не залилась, никнейм должен обновиться.
                finishProfilePut(); 
                s3Reply->deleteLater();
            });
        }
        else {
            finishProfilePut();
        }
    });
}

void AuthController::login(const QString& email, const QString& password)
{
    QJsonObject reqObj;
    reqObj["email"] = email;
    reqObj["password"] = password;

    QNetworkRequest request(QUrl(API_BASE_URL + "/auth/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(reqObj).toJson());
    connect(reply, &QNetworkReply::finished, this, &AuthController::onLoginReply);
}

void AuthController::onLoginReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit loginFinished(false, "Ошибка входа. Проверьте данные.");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject obj = doc.object();
    
    QString token = obj["access_token"].toString();
    QString email = obj["email"].toString();
    QString nickname = obj["nickname"].toString();
    QString avatar_hash = obj["avatar_hash"].toString();

    if (!token.isEmpty()) {
        SettingsManager::instance().setJwtToken(token);
        if (!email.isEmpty()) {
            SettingsManager::instance().setUserEmail(email);
        }
        if (!nickname.isEmpty()) SettingsManager::instance().setUserNickname(nickname);
        if (!avatar_hash.isEmpty()) SettingsManager::instance().setUserAvatarHash(avatar_hash);
        emit authStateChanged();
        emit loginFinished(true, "Успешный вход!");
    } else {
        emit loginFinished(false, "Неверный ответ сервера.");
    }
}

void AuthController::registerUser(const QString& email, const QString& password)
{
    QJsonObject reqObj;
    reqObj["email"] = email;
    reqObj["password"] = password;

    QNetworkRequest request(QUrl(API_BASE_URL + "/auth/register"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(reqObj).toJson());
    connect(reply, &QNetworkReply::finished, this, &AuthController::onRegisterReply);
}

void AuthController::onRegisterReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        // Получить детали ошибки из тела, если возможно
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QString errorMsg = doc.object()["detail"].toString();
        if (errorMsg.isEmpty()) errorMsg = "Ошибка регистрации.";
        
        emit registerFinished(false, errorMsg);
        return;
    }

    // Допустим, регистрация также возвращает токен, или просит войти.
    // Если возвращает токен:
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject obj = doc.object();
    
    QString token = obj["access_token"].toString();
    QString email = obj["email"].toString();
    QString nickname = obj["nickname"].toString();
    QString avatar_hash = obj["avatar_hash"].toString();

    if (!token.isEmpty()) {
        SettingsManager::instance().setJwtToken(token);
        if (!email.isEmpty()) SettingsManager::instance().setUserEmail(email);
        if (!nickname.isEmpty()) SettingsManager::instance().setUserNickname(nickname);
        if (!avatar_hash.isEmpty()) SettingsManager::instance().setUserAvatarHash(avatar_hash);
        emit authStateChanged();
        emit registerFinished(true, "Успешная регистрация и вход!");
    } else {
        // Если только сообщение об успехе, можно попросить войти
        emit registerFinished(true, "Успешная регистрация! Теперь войдите.");
    }
}

void AuthController::logout()
{
    SettingsManager::instance().setJwtToken("");
    SettingsManager::instance().setUserEmail("");
    SettingsManager::instance().setUserNickname("");
    SettingsManager::instance().setUserAvatarHash("");
    emit authStateChanged();
}
