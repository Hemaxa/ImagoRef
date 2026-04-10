#include "NetworkController.h"
#include "SettingsManager.h"
#include "StorageController.h"
#include "CacheManager.h"
#include "ImageModel.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QFile>
#include <QSqlQuery>
#include <QSqlDatabase>

NetworkController::NetworkController(StorageController *storage, QObject *parent)
    : QObject(parent)
    , m_storageController(storage)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_webSocket(new QWebSocket())
    , m_syncTimer(new QTimer(this))
    , m_isUploading(false)
{
    connect(m_webSocket, &QWebSocket::connected, this, &NetworkController::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &NetworkController::onDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &NetworkController::onTextMessageReceived);
    // Игнорируем SSL или выводим ошибку:
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &NetworkController::onError);

    m_syncTimer->setInterval(2000); // 2 second polling for tasks
    connect(m_syncTimer, &QTimer::timeout, this, &NetworkController::processSyncQueue);
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
        // 1. Берем токен из настроек
        QString token = SettingsManager::instance().getJwtToken();

        // 2. Формируем правильный URL с множественным числом (boards) и токеном
        QString urlString = QString("%1/boards/%2?token=%3")
                            .arg(WS_URL)
                            .arg(boardId)
                            .arg(token);
                            
        QUrl wsUrl(urlString);
        m_webSocket->open(wsUrl);
        m_syncTimer->start();

        // 3. Запрашиваем ссылки на картинки при открытии доски
        fetchMetadataAndMissingImages();
    }
}

void NetworkController::disconnectFromBoard()
{
    if (m_webSocket->isValid()) {
        m_webSocket->close();
    }
    m_syncTimer->stop();
    m_currentBoardId.clear();
    m_isUploading = false;
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
    QJsonObject payload = msgObj["payload"].toObject();
    
    if (action == "ADD_ITEM" || action == "UPDATE_ITEM" || action == "DELETE_ITEM") {
        if (m_storageController->applyNetworkDelta(action, payload)) {
            QString itemId = payload["id"].toString();
            if (itemId.isEmpty()) itemId = payload["item_id"].toString();
            if (!itemId.isEmpty()) {
                emit itemUpdatedFromNetwork(itemId);
            }
        }
    }
}

void NetworkController::processSyncQueue()
{
    if (m_currentBoardId.isEmpty()) return;

    // Берем сразу пачку задач (например, 50), чтобы мгновенно раскидать WS-ивенты
    QVariantList tasks = m_storageController->getSyncTasks(50);
    if (tasks.isEmpty()) return;
    
    for (const QVariant& taskVar : tasks) {
        QVariantMap task = taskVar.toMap();
        int taskId = task["id"].toInt();
        QString actionType = task["action_type"].toString();
        QJsonObject payload = QJsonDocument::fromJson(task["payload"].toString().toUtf8()).object();
        
        if (actionType == "UPLOAD_IMAGE") {
            if (!m_isUploading) {
                handleUploadTask(taskId, payload);
                // Прерываем цикл, так как загрузка асинхронная. 
                // Остальные задачи обработаем на следующем тике таймера.
                break; 
            } else {
                break; // Ждем окончания текущей загрузки
            }
        } else if (actionType == "ADD_ITEM" || actionType == "UPDATE_ITEM" || actionType == "DELETE_ITEM" || actionType == "UPDATE_BOARD") {
            handleWebSocketTask(taskId, actionType, payload);
        } else {
            // Неизвестные задачи удаляем, чтобы не было затора
            m_storageController->removeSyncTask(taskId);
        }
    }
}

void NetworkController::handleUploadTask(int taskId, const QJsonObject &payload)
{
    QString itemId = payload["image_id"].toString();
    ImagoImageData item = m_storageController->getItemFromDb(itemId);
    
    if (item.id.isEmpty() || item.imageHash.isEmpty()) {
        m_storageController->removeSyncTask(taskId);
        return;
    }
    
    uploadImageToS3(taskId, itemId, item.imageHash);
}

void NetworkController::handleWebSocketTask(int taskId, const QString &action, const QJsonObject &payload)
{
    if (m_webSocket->isValid()) {
        QJsonObject msg;
        msg["action"] = action;
        QString itemId = payload["image_id"].toString();
        
        if (action == "UPDATE_ITEM" || action == "ADD_ITEM") {
            ImagoImageData data = m_storageController->getItemFromDb(itemId);
            if (!data.id.isEmpty()) {
                QJsonObject itemPayload;
                itemPayload["id"] = data.id;
                itemPayload["board_id"] = m_currentBoardId;
                itemPayload["x"] = data.x;
                itemPayload["y"] = data.y;
                itemPayload["width"] = data.width;
                itemPayload["height"] = data.height;
                itemPayload["z_index"] = data.zValue;
                
                // Add exact json payload string back if needed for updates
                QJsonObject innerPayload;
                innerPayload["rotation"] = data.rotation;
                innerPayload["label"] = data.label;
                innerPayload["cropX"] = data.cropX;
                innerPayload["cropY"] = data.cropY;
                innerPayload["cropWidth"] = data.cropWidth;
                innerPayload["cropHeight"] = data.cropHeight;
                innerPayload["opacity"] = data.opacity;
                innerPayload["imageHash"] = data.imageHash;
                itemPayload["payload"] = innerPayload;
                
                msg["payload"] = itemPayload;
            } else {
                msg["payload"] = payload; 
            }
        } else {
            msg["payload"] = payload;
        }

        m_webSocket->sendTextMessage(QString(QJsonDocument(msg).toJson(QJsonDocument::Compact)));
        m_storageController->removeSyncTask(taskId);
    }
}

void NetworkController::uploadImageToS3(int taskId, const QString &itemId, const QString &imageHash)
{
    m_isUploading = true;

    // Убедись, что API_BASE_URL содержит /api (например: http://твой_впс:8000/api)
    // Либо жестко пропиши здесь: API_BASE_URL + "/api/boards/" ...
    QString apiUrl = API_BASE_URL + "/boards/" + m_currentBoardId + "/sync_hashes";
    QNetworkRequest request((QUrl(apiUrl)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QString token = SettingsManager::instance().getJwtToken();
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QJsonObject payload;
    QJsonArray hashesArray;
    hashesArray.append(imageHash);
    payload["hashes"] = hashesArray;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(payload).toJson());
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, taskId, itemId, imageHash]() {
        reply->deleteLater();
        
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Failed to get presigned URL from API:" << reply->errorString();
            m_isUploading = false;
            // КРИТИЧНО: Удаляем задачу при ошибке 404 (доски нет и т.д.), 
            // иначе очередь заблокируется навсегда!
            m_storageController->removeSyncTask(taskId);
            return;
        }

        QJsonObject response = QJsonDocument::fromJson(reply->readAll()).object();
        QJsonObject urls = response["urls"].toObject();
        
        if (!urls.contains(imageHash)) {
            QJsonObject wsPayload;
            wsPayload["image_id"] = itemId;
            handleWebSocketTask(taskId, "ADD_ITEM", wsPayload); 
            m_isUploading = false;
            return;
        }

        QString presignedUrl = urls[imageHash].toString();
        QString imageCachePath = CacheManager::instance().getCacheFilePath(imageHash);
        
        QFile *file = new QFile(imageCachePath);
        if (!file->open(QIODevice::ReadOnly)) {
            file->deleteLater();
            m_isUploading = false;
            m_storageController->removeSyncTask(taskId); // Защита от блокировки
            return;
        }

        QNetworkRequest s3Request((QUrl(presignedUrl)));
        s3Request.setHeader(QNetworkRequest::ContentTypeHeader, "image/png");
        
        QNetworkReply *s3Reply = m_networkManager->put(s3Request, file);
        connect(s3Reply, &QNetworkReply::finished, this, [this, s3Reply, file, taskId, itemId]() {
            s3Reply->deleteLater();
            file->deleteLater();
            m_isUploading = false;

            if (s3Reply->error() == QNetworkReply::NoError) {
                qDebug() << "Successfully uploaded to S3!";
                QJsonObject wsPayload;
                wsPayload["image_id"] = itemId;
                handleWebSocketTask(taskId, "ADD_ITEM", wsPayload); 
            } else {
                // ДОБАВЬТЕ ЭТИ СТРОКИ ДЛЯ ДЕБАГА
                qWarning() << "S3 Upload failed:" << s3Reply->errorString();
                qWarning() << "HTTP Status:" << s3Reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                qWarning() << "S3 URL was:" << s3Reply->url().toString();
                qWarning() << "Response from S3:" << s3Reply->readAll();
                
                m_storageController->removeSyncTask(taskId);
            }
        });
    });
}

// =====================================================================
// НОВЫЕ МЕТОДЫ ДЛЯ ЗАГРУЗКИ КАРТИНОК ИЗ S3 И СИНХРОНИЗАЦИИ БД
// =====================================================================

void NetworkController::fetchMetadataAndMissingImages()
{
    QString token = SettingsManager::instance().getJwtToken();
    QString apiUrl = API_BASE_URL + "/boards/" + m_currentBoardId + "/metadata";
    
    QNetworkRequest request((QUrl(apiUrl)));
    request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QNetworkReply *reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Failed to fetch metadata:" << reply->errorString();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject response = doc.object();
        
        // 1. СИНХРОНИЗАЦИЯ БАЗЫ ДАННЫХ
        QJsonObject dataObj = response["data"].toObject();
        QJsonArray itemsArray = dataObj["items"].toArray();
        bool localDbChanged = false;
        
        // Транзакция для мгновенного сохранения (чтобы UI не зависал)
        QSqlDatabase::database().transaction();
        for (const QJsonValue &val : itemsArray) {
            QJsonObject itemObj = val.toObject();
            if (m_storageController->applyNetworkDelta("ADD_ITEM", itemObj)) {
                localDbChanged = true;
            }
        }
        QSqlDatabase::database().commit();
        
        // Если база обновилась, ОДИН РАЗ загружаем пустые рамки на доску
        if (localDbChanged) {
            m_storageController->loadBoardFromDb(m_currentBoardId);
        }

        // 2. СКАЧИВАНИЕ НЕДОСТАЮЩИХ КАРТИНОК ИЗ S3
        QJsonObject downloadUrls = response["download_urls"].toObject();
        for (auto it = downloadUrls.begin(); it != downloadUrls.end(); ++it) {
            QString hash = it.key();
            QString url = it.value().toString();
            
            if (!CacheManager::instance().isCached(hash)) {
                downloadImageFromS3(hash, url);
            }
        }
    });
}

void NetworkController::downloadImageFromS3(const QString &hash, const QString &url)
{
    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, hash]() {
        reply->deleteLater();
        
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        
        if (reply->error() == QNetworkReply::NoError && statusCode == 200) {
            QByteArray imageData = reply->readAll();
            
            // Сохраняем байты в кэш
            CacheManager::instance().saveToCache(hash, imageData);
            qDebug() << "Successfully downloaded and cached missing image:" << hash;
            
            // Ищем все элементы с этой картинкой на доске
            QSqlQuery q;
            q.prepare("SELECT id FROM items WHERE board_id = :board_id AND payload LIKE :hash");
            q.bindValue(":board_id", m_currentBoardId);
            q.bindValue(":hash", "%" + hash + "%");
            
            if (q.exec()) {
                while (q.next()) {
                    QString itemId = q.value("id").toString();
                    // Аккуратно обновляем ТОЛЬКО скачанную картинку!
                    emit itemUpdatedFromNetwork(itemId);
                }
            }
            
            // ВНИМАНИЕ: m_storageController->loadBoardFromDb удален, 
            // так как именно он вызывал мерцание и зависание интерфейса!
            
        } else {
            qWarning() << "Failed to download image from S3. HTTP Status:" << statusCode;
        }
    });
}
