#include "NetworkController.h"
#include "SettingsManager.h"
#include "StorageController.h"
#include "CacheManager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>

NetworkController::NetworkController(StorageController *storage, QObject *parent)
    : QObject(parent)
    , m_storageController(storage)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_webSocket(new QWebSocket())
{
    connect(m_webSocket, &QWebSocket::connected, this, &NetworkController::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &NetworkController::onDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &NetworkController::onTextMessageReceived);
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &NetworkController::onError);
}

NetworkController::~NetworkController()
{
    disconnectFromBoard();
}

void NetworkController::connectToBoard(const QString &boardId)
{
    if (m_currentBoardId == boardId) return;
    
    disconnectFromBoard();
    m_currentBoardId = boardId;
    
    if (!boardId.isEmpty()) {
        QString token = SettingsManager::instance().getJwtToken();
        QString urlString = QString("%1/boards/%2?token=%3").arg(WS_URL).arg(boardId).arg(token);
                            
        m_webSocket->open(QUrl(urlString));
        fetchMetadataAndMissingImages();
    }
}

void NetworkController::disconnectFromBoard()
{
    if (m_webSocket->isValid()) {
        m_webSocket->close();
    }
    m_currentBoardId.clear();
    m_pendingUploads = 0;
    m_uploadFailed = false;
}

void NetworkController::onConnected()
{
    qDebug() << "WebSocket connected for board:" << m_currentBoardId;
}

void NetworkController::onDisconnected()
{
    qDebug() << "WebSocket disconnected";
}

void NetworkController::onError(QAbstractSocket::SocketError error)
{
    qWarning() << "WebSocket Error:" << error << m_webSocket->errorString();
}

void NetworkController::onTextMessageReceived(const QString &message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) return;
    
    QJsonObject msgObj = doc.object();
    QString action = msgObj["action"].toString();
    
    // ТЕПЕРЬ СОКЕТ ПРОСТО УВЕДОМЛЯЕТ НАС, ЧТО ДОСКА ИЗМЕНИЛАСЬ
    if (action == "BOARD_UPDATED") {
        qDebug() << "Received BOARD_UPDATED from server. Fetching changes...";
        fetchMetadataAndMissingImages();
    }
}

// =====================================================================
// ПАКЕТНАЯ СИНХРОНИЗАЦИЯ (ВЫЗЫВАЕТСЯ ПРИ CTRL+S)
// =====================================================================

void NetworkController::syncBoardToServer()
{
    if (m_currentBoardId.isEmpty()) return;

    // 1. Собираем все изменения из локальной БД
    QJsonObject state = m_storageController->getUnsyncedBoardState(m_currentBoardId);
    QJsonArray updatedItems = state["updated_items"].toArray();
    QJsonArray deletedItems = state["deleted_items"].toArray();

    if (updatedItems.isEmpty() && deletedItems.isEmpty()) {
        qDebug() << "Nothing to sync. Board is up to date.";
        emit syncFinished(true);
        return;
    }

    emit syncStarted();
    m_uploadFailed = false;

    // 2. Собираем хэши всех картинок, которые мы хотим отправить
    QSet<QString> hashesToUpload;
    for (const QJsonValue& val : updatedItems) {
        QJsonObject item = val.toObject();
        QJsonObject payload = item["payload"].toObject();
        QString hash = payload["imageHash"].toString();
        if (!hash.isEmpty()) {
            hashesToUpload.insert(hash);
        }
    }

    // 3. Узнаем у сервера, какие картинки ему нужно догрузить
    if (!hashesToUpload.isEmpty()) {
        checkMissingImagesAndUpload(state, hashesToUpload);
    } else {
        pushStateToServer(state);
    }
}

void NetworkController::checkMissingImagesAndUpload(const QJsonObject& boardState, const QSet<QString>& hashes)
{
    QString apiUrl = API_BASE_URL + "/boards/" + m_currentBoardId + "/sync_hashes";
    QNetworkRequest request((QUrl(apiUrl)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QString token = SettingsManager::instance().getJwtToken();
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QJsonObject payload;
    QJsonArray hashesArray;
    for (const QString& h : hashes) {
        hashesArray.append(h);
    }
    payload["hashes"] = hashesArray;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(payload).toJson());
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, boardState]() {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Failed to check hashes:" << reply->errorString();
            emit syncFinished(false);
            reply->deleteLater();
            return;
        }

        QJsonObject response = QJsonDocument::fromJson(reply->readAll()).object();
        QJsonObject urls = response["urls"].toObject();
        
        // Если сервер знает все картинки, сразу пушим состояние
        if (urls.isEmpty()) {
            pushStateToServer(boardState);
        } else {
            // Иначе сохраняем стейт и начинаем счетчик асинхронных загрузок
            m_pendingBoardState = boardState;
            m_pendingUploads = urls.keys().size();

            for (auto it = urls.begin(); it != urls.end(); ++it) {
                uploadToS3(it.key(), it.value().toString());
            }
        }
        reply->deleteLater();
    });
}

void NetworkController::uploadToS3(const QString& hash, const QString& url)
{
    QString imageCachePath = CacheManager::instance().getCacheFilePath(hash);
    QFile file(imageCachePath);
    
    // Открываем локально, без выделения памяти через new
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot find image in cache for upload:" << hash;
        m_uploadFailed = true;
        m_pendingUploads--;
        if (m_pendingUploads == 0) {
            emit syncFinished(false);
        }
        return;
    }

    // Вычитываем все байты в память
    QByteArray fileData = file.readAll();
    file.close(); // Файл больше не нужен, закрываем сразу

    QNetworkRequest request((QUrl(url)));
    // Устанавливаем заголовки. S3 требует точного совпадения Content-Type с тем, что было при генерации presigned URL
    request.setHeader(QNetworkRequest::ContentTypeHeader, "image/png");
    request.setHeader(QNetworkRequest::ContentLengthHeader, fileData.size());
    
    // Передаем QByteArray вместо файла
    QNetworkReply *reply = m_networkManager->put(request, fileData);
    connect(reply, &QNetworkReply::finished, this, [this, reply, hash]() {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Failed to upload image to S3:" << hash << "Error:" << reply->errorString();
            m_uploadFailed = true;
        } else {
            qDebug() << "Successfully uploaded image to S3:" << hash;
        }

        reply->deleteLater();

        // Уменьшаем счетчик. Когда дойдет до 0, заливаем JSON конфигурацию доски
        m_pendingUploads--;
        if (m_pendingUploads == 0) {
            if (m_uploadFailed) {
                emit syncFinished(false);
            } else {
                pushStateToServer(m_pendingBoardState);
            }
        }
    });
}

void NetworkController::pushStateToServer(const QJsonObject& boardState)
{
    QString apiUrl = API_BASE_URL + "/boards/" + m_currentBoardId + "/sync"; // На бэке нужно сделать этот PUT эндпоинт
    QNetworkRequest request((QUrl(apiUrl)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QString token = SettingsManager::instance().getJwtToken();
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QNetworkReply *reply = m_networkManager->put(request, QJsonDocument(boardState).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Board state successfully synced to server!";
            
            // Снимаем флаги is_dirty и физически удаляем удаленные элементы из БД
            m_storageController->markAsSynced(m_currentBoardId);
            fetchMetadataAndMissingImages();
            emit syncFinished(true);
        } else {
            qWarning() << "Failed to sync board state:" << reply->errorString();
            emit syncFinished(false);
        }
        reply->deleteLater();
    });
}

// =====================================================================
// ПОЛУЧЕНИЕ ИЗМЕНЕНИЙ ИЗ СЕТИ
// =====================================================================

void NetworkController::fetchMetadataAndMissingImages()
{
    QString token = SettingsManager::instance().getJwtToken();
    QString apiUrl = API_BASE_URL + "/boards/" + m_currentBoardId + "/metadata";
    
    QNetworkRequest request((QUrl(apiUrl)));
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QNetworkReply *reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        
        // 404 означает, что мы только что авторизовались и открыли локальную доску
        if (statusCode == 404) {
            qDebug() << "Board not found on server. Creating cloud board...";
            syncLocalBoardToServer();
            reply->deleteLater();
            return;
        }

        if (reply->error() != QNetworkReply::NoError) {
            reply->deleteLater();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject response = doc.object();
        
        QJsonObject dataObj = response["data"].toObject();
        QJsonArray itemsArray = dataObj["items"].toArray();
        bool localDbChanged = false;
        QSet<QString> serverItemIds;
        
        QSqlDatabase::database().transaction();
        for (const QJsonValue &val : itemsArray) {
            QJsonObject itemObj = val.toObject();
            const QString itemId = itemObj["id"].toString();
            if (!itemId.isEmpty()) {
                serverItemIds.insert(itemId);
            }
            if (m_storageController->applyNetworkDelta("ADD_ITEM", itemObj)) {
                localDbChanged = true;
            }
        }

        QSqlQuery localItemsQuery;
        localItemsQuery.prepare("SELECT id FROM items WHERE board_id = :board_id AND is_dirty = 0 AND is_deleted = 0");
        localItemsQuery.bindValue(":board_id", m_currentBoardId);
        if (localItemsQuery.exec()) {
            while (localItemsQuery.next()) {
                const QString localId = localItemsQuery.value("id").toString();
                if (!serverItemIds.contains(localId)) {
                    QSqlQuery deleteQuery;
                    deleteQuery.prepare("DELETE FROM items WHERE id = :id");
                    deleteQuery.bindValue(":id", localId);
                    if (deleteQuery.exec()) {
                        localDbChanged = true;
                    }
                }
            }
        }
        QSqlDatabase::database().commit();
        
        if (localDbChanged) {
            m_storageController->loadBoardFromDb(m_currentBoardId);
        }

        QJsonObject downloadUrls = response["download_urls"].toObject();
        for (auto it = downloadUrls.begin(); it != downloadUrls.end(); ++it) {
            QString hash = it.key();
            QString url = it.value().toString();
            
            if (!CacheManager::instance().isCached(hash)) {
                downloadImageFromS3(hash, url);
            }
        }
        reply->deleteLater();
    });
}

void NetworkController::downloadImageFromS3(const QString &hash, const QString &url)
{
    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, hash]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray imageData = reply->readAll();
            CacheManager::instance().saveToCache(hash, imageData);
            
            // Находим и обновляем UI элементы, связанные с этой картинкой
            QSqlQuery q;
            q.prepare("SELECT id FROM items WHERE board_id = :board_id AND payload LIKE :hash");
            q.bindValue(":board_id", m_currentBoardId);
            q.bindValue(":hash", "%" + hash + "%");
            if (q.exec()) {
                while (q.next()) {
                    emit itemUpdatedFromNetwork(q.value("id").toString());
                }
            }
        }
        reply->deleteLater();
    });
}

void NetworkController::syncLocalBoardToServer()
{
    QString title = m_storageController->getBoardTitle(m_currentBoardId); 
    if (title.isEmpty()) title = "Recovered Board";

    QString apiUrl = API_BASE_URL + "/boards/"; 
    QNetworkRequest request((QUrl(apiUrl)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QString token = SettingsManager::instance().getJwtToken();
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QJsonObject payload;
    payload["id"] = m_currentBoardId;
    payload["title"] = title;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(payload).toJson());
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Offline board successfully created on server! Starting full state push...";
            // Доска создана! Теперь выгружаем весь её накопленный контент (картинки + координаты)
            syncBoardToServer();
        } else {
            qWarning() << "Failed to create offline board on server:" << reply->errorString();
        }
        reply->deleteLater();
    });
}
