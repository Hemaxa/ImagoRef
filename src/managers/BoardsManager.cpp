#include "BoardsManager.h"
#include "StorageController.h"
#include "SettingsManager.h"

#include <QUuid>
#include <QJSEngine>
#include <QNetworkRequest>  // ДОБАВЛЕНО
#include <QNetworkReply>    // ДОБАВЛЕНО
#include <QJsonDocument>    // ДОБАВЛЕНО
#include <QJsonObject>      // ДОБАВЛЕНО

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
    m_boards = StorageController::getLocalBoards();
    emit boardsChanged();
    emit boardsFetched(true);
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
    StorageController::renameLocalBoard(id, newTitle);
    emit boardRenamed(true);
    loadBoards();
}

void BoardsManager::deleteBoard(const QString& id)
{
    StorageController::deleteLocalBoard(id);
    emit boardDeleted(true);
    loadBoards();
}
