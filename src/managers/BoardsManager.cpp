#include "BoardsManager.h"
#include "StorageController.h"
#include "SettingsManager.h"

#include <QUuid>
#include <QJSEngine>
#include <QNetworkRequest>  // ДОБАВЛЕНО
#include <QNetworkReply>    // ДОБАВЛЕНО
#include <QJsonDocument>    // ДОБАВЛЕНО
#include <QJsonObject>      // ДОБАВЛЕНО
#include <QJsonArray>

namespace {

QString boardString(const QJsonObject &object, const QStringList &keys)
{
    for (const QString &key : keys) {
        const QJsonValue value = object.value(key);
        if (value.isString()) {
            const QString text = value.toString().trimmed();
            if (!text.isEmpty()) {
                return text;
            }
        }
    }

    return {};
}

QJsonArray extractBoardsArray(const QJsonDocument &document)
{
    if (document.isArray()) {
        return document.array();
    }

    if (!document.isObject()) {
        return {};
    }

    const QJsonObject root = document.object();
    if (root.value("boards").isArray()) {
        return root.value("boards").toArray();
    }

    if (root.value("data").isArray()) {
        return root.value("data").toArray();
    }

    if (root.value("data").isObject()) {
        const QJsonObject data = root.value("data").toObject();
        if (data.value("boards").isArray()) {
            return data.value("boards").toArray();
        }
    }

    return {};
}

}

BoardsManager* BoardsManager::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    auto *inst = &instance();
    QJSEngine::setObjectOwnership(inst, QJSEngine::CppOwnership);
    return inst;
}

BoardsManager& BoardsManager::instance()
{
    static BoardsManager manager;
    return manager;
}

BoardsManager::BoardsManager(QObject* parent) : QObject(parent)
{
}

QVariantList BoardsManager::getBoards() const
{
    return m_boards;
}

void BoardsManager::loadBoards()
{
    const QString token = SettingsManager::instance().getJwtToken();
    if (token.isEmpty()) {
        m_boards = StorageController::getLocalBoards();
        emit boardsChanged();
        emit boardsFetched(true);
        return;
    }

    QNetworkRequest request(QUrl("https://imagoref.ru/api/boards/"));
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QNetworkReply *reply = m_networkManager.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Failed to fetch cloud boards:" << reply->errorString();
            m_boards = StorageController::getLocalBoards();
            emit boardsChanged();
            emit boardsFetched(false);
            return;
        }

        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        const QJsonArray boardsArray = extractBoardsArray(document);
        QVariantList boards;

        for (const QJsonValue &value : boardsArray) {
            const QJsonObject object = value.toObject();
            const QString id = boardString(object, {"id", "board_id"});
            if (id.isEmpty()) {
                continue;
            }

            QVariantMap map;
            map["id"] = id;
            map["name"] = boardString(object, {"title", "name"});
            boards.append(map);
        }

        m_boards = boards;
        emit boardsChanged();
        emit boardsFetched(true);
    });
}

void BoardsManager::createBoard(const QString& title)
{
    // 1. Создаем локально
    QString boardId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    StorageController::createLocalBoard(boardId, title);
    
    // 2. Отправляем на сервер (Шаг 2)
    QString apiUrl = "https://imagoref.ru/api/boards/"; // Убедись, что путь верный
    QNetworkRequest request((QUrl(apiUrl)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QString token = SettingsManager::instance().getJwtToken();
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QJsonObject payload;
    payload["id"] = boardId;
    payload["title"] = title;

    QNetworkReply *reply = m_networkManager.post(request, QJsonDocument(payload).toJson());
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, boardId]() {
        reply->deleteLater();
        
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Board successfully registered on server!";
            // Только после ответа сервера даем QML команду открывать доску и подключать WebSocket
            emit boardCreated(boardId, true);
            loadBoards();
        } else {
            qWarning() << "Failed to create board on server:" << reply->errorString();
            // Если нет сети, можно выдать ошибку в UI
            emit boardCreated(boardId, false);
        }
    });
}

void BoardsManager::renameBoard(const QString& id, const QString& newTitle)
{
    const QString trimmedTitle = newTitle.trimmed();
    if (id.isEmpty() || trimmedTitle.isEmpty()) {
        emit boardRenamed(false);
        return;
    }

    const QString token = SettingsManager::instance().getJwtToken();
    if (token.isEmpty()) {
        StorageController::renameLocalBoard(id, trimmedTitle);
        SettingsManager::instance().renameRecentBoard(id, trimmedTitle);
        emit boardRenamed(true);
        loadBoards();
        return;
    }

    QNetworkRequest request(QUrl(QString("https://imagoref.ru/api/boards/%1").arg(id)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QJsonObject payload;
    payload["title"] = trimmedTitle;

    QNetworkReply *reply = m_networkManager.put(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply, id, trimmedTitle]() {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
            StorageController::renameLocalBoard(id, trimmedTitle);
            SettingsManager::instance().renameRecentBoard(id, trimmedTitle);
            emit boardRenamed(true);
            loadBoards();
        } else {
            qWarning() << "Failed to rename board on server:" << reply->errorString();
            emit boardRenamed(false);
        }
    });
}

void BoardsManager::deleteBoard(const QString& id)
{
    if (id.isEmpty()) {
        emit boardDeleted(false);
        return;
    }

    const QString token = SettingsManager::instance().getJwtToken();
    if (token.isEmpty()) {
        StorageController::deleteLocalBoard(id);
        SettingsManager::instance().removeRecentBoard(id);
        emit boardDeleted(true);
        loadBoards();
        return;
    }

    QNetworkRequest request(QUrl(QString("https://imagoref.ru/api/boards/%1").arg(id)));
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QNetworkReply *reply = m_networkManager.deleteResource(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, id]() {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
            StorageController::deleteLocalBoard(id);
            SettingsManager::instance().removeRecentBoard(id);
            emit boardDeleted(true);
            loadBoards();
        } else {
            qWarning() << "Failed to delete board on server:" << reply->errorString();
            emit boardDeleted(false);
        }
    });
}
