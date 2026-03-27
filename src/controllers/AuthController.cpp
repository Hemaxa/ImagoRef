#include "AuthController.h"
#include "SettingsManager.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJSEngine>

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

    if (!token.isEmpty()) {
        SettingsManager::instance().setJwtToken(token);
        if (!email.isEmpty()) {
            SettingsManager::instance().setUserEmail(email);
        } else {
            // Если сервер не возвращает email, можно распарсить токен или использовать переданный при логине
            // Но мы пока просто обновим стейт
        }
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

    if (!token.isEmpty()) {
        SettingsManager::instance().setJwtToken(token);
        if (!email.isEmpty()) SettingsManager::instance().setUserEmail(email);
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
    emit authStateChanged();
}
