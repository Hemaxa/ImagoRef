#include "CloudController.h"
#include "ImageModel.h"
#include "CacheManager.h"
#include "BoardController.h"
#include "SettingsManager.h"
#include <QUndoStack>

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QDebug>

CloudController::CloudController(ImagoImageModel *model, QUndoStack *undoStack, QObject *parent)
    : QObject(parent), m_model(model), m_undoStack(undoStack), m_networkManager(new QNetworkAccessManager(this))
{
}

void CloudController::syncUp(const QString &boardId)
{
    emit cloudSyncStarted();
    m_currentUploadBoardId = boardId;

    QStringList hashes;
    for (const ImagoImageData &item : m_model->getAllItems()) {
        if (!item.imageHash.isEmpty() && !hashes.contains(item.imageHash)) {
            hashes.append(item.imageHash);
        }
    }

    QJsonObject reqObj;
    QJsonArray hashArray;
    for (const QString &h : hashes) {
        hashArray.append(h);
    }
    reqObj["hashes"] = hashArray;

    QNetworkRequest request(QUrl(API_BASE_URL + "/boards/" + boardId + "/sync_hashes"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QString token = SettingsManager::instance().getJwtToken();
    if (!token.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    }

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(reqObj).toJson());
    connect(reply, &QNetworkReply::finished, this, &CloudController::onSyncHashesFinished);
}

void CloudController::onSyncHashesFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit cloudSyncFinished();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject obj = doc.object();
    QJsonObject urls = obj["urls"].toObject();

    m_uploadCount = 0;
    m_uploadTotal = urls.keys().size();

    if (m_uploadTotal == 0) {
        uploadMetadata(m_currentUploadBoardId);
        return;
    }

    emit cloudSyncProgress(0, m_uploadTotal);

    for (const QString &hash : urls.keys()) {
        QString presignedUrl = urls[hash].toString();
        
        QPixmap cachedPix = CacheManager::instance().loadFromCache(hash);
        if (cachedPix.isNull()) {
            qDebug() << "ВНИМАНИЕ: Картинка" << hash << "не найдена в кэше! Пропуск.";
            m_uploadCount++;
            
            // ИСПРАВЛЕНИЕ 1: Проверяем, не зависли ли мы на последней пустой картинке
            if (m_uploadCount >= m_uploadTotal) {
                uploadMetadata(m_currentUploadBoardId);
            }
            continue; 
        }

        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        cachedPix.save(&buffer, "PNG");

        // ИСПРАВЛЕНИЕ 2: QUrl::StrictMode не дает Qt сломать подпись от S3
        QNetworkRequest request(QUrl(presignedUrl, QUrl::StrictMode));
        
        // ИСПРАВЛЕНИЕ 3: Сырые заголовки вместо стандартных, чтобы избежать добавления "charset=utf-8"
        request.setRawHeader("Content-Type", "image/png");
        request.setRawHeader("Content-Length", QByteArray::number(data.size()));

        QNetworkReply *putReply = m_networkManager->put(request, data);
        connect(putReply, &QNetworkReply::finished, this, &CloudController::onS3UploadFinished);
    }
}

void CloudController::onS3UploadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "\n!!! ОШИБКА ЗАГРУЗКИ В S3 !!!";
        qDebug() << "Код ошибки:" << reply->error();
        qDebug() << "Текст:" << reply->errorString();
        qDebug() << "Ответ от TimeWeb:" << reply->readAll();
        qDebug() << "============================\n";
    } else {
        qDebug() << "Успешная загрузка картинки в S3!";
    }

    m_uploadCount++;
    emit cloudSyncProgress(m_uploadCount, m_uploadTotal);

    if (m_uploadCount >= m_uploadTotal) {
        uploadMetadata(m_currentUploadBoardId);
    }
}

void CloudController::uploadMetadata(const QString &boardId)
{
    QJsonObject rootObj;
    rootObj["version"] = "1.0";

    QJsonObject canvasObj;
    BoardController* board = qobject_cast<BoardController*>(parent());
    if (board) {
        canvasObj["gridSize"] = board->getGridSize();
        canvasObj["cameraX"] = board->getCameraX();
        canvasObj["cameraY"] = board->getCameraY();
        canvasObj["cameraZoom"] = board->getCameraZoom();
    }
    rootObj["canvas"] = canvasObj;

    QJsonArray itemsArray;
    for (const ImagoImageData &item : m_model->getAllItems()) {
        QJsonObject itemObj;
        itemObj["id"] = item.id;
        itemObj["pos_x"] = item.x;
        itemObj["pos_y"] = item.y;
        itemObj["width"] = item.width;
        itemObj["height"] = item.height;
        itemObj["rotation"] = item.rotation;
        itemObj["zValue"] = item.zValue;
        itemObj["label"] = item.label;
        itemObj["cropX"] = item.cropX;
        itemObj["cropY"] = item.cropY;
        itemObj["cropWidth"] = item.cropWidth;
        itemObj["cropHeight"] = item.cropHeight;
        itemObj["opacity"] = item.opacity;
        itemObj["imageHash"] = item.imageHash;

        itemsArray.append(itemObj);
    }
    rootObj["items"] = itemsArray;

    QNetworkRequest request(QUrl(API_BASE_URL + "/boards/" + boardId + "/metadata"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QString token = SettingsManager::instance().getJwtToken();
    if (!token.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    }

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(rootObj).toJson());
    connect(reply, &QNetworkReply::finished, this, &CloudController::onMetadataUploadFinished);
}

void CloudController::onMetadataUploadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) reply->deleteLater();
    emit cloudSyncFinished();
}

void CloudController::syncDown(const QString &boardId)
{
    emit cloudSyncStarted();
    m_currentDownloadBoardId = boardId;

    QNetworkRequest request(QUrl(API_BASE_URL + "/boards/" + boardId + "/metadata"));
    QString token = SettingsManager::instance().getJwtToken();
    if (!token.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    }
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &CloudController::onMetadataDownloadFinished);
}

void CloudController::onMetadataDownloadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit cloudSyncFinished();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject rootResp = doc.object();

    // 1. ИСПРАВЛЕНИЕ: Вытаскиваем саму структуру доски (canvas, items) из ключа "data"
    m_downloadedMetadata = rootResp["data"].toObject();

    // 2. А вот ссылки на скачивание S3 лежат в корне ответа сервера
    QJsonObject urlsObj = rootResp["download_urls"].toObject();

    // 3. Теперь itemsArray корректно прочитает картинки
    QJsonArray itemsArray = m_downloadedMetadata["items"].toArray();
    QMap<QString, QString> missingUrls; // hash -> url

    for (const QJsonValue &value : itemsArray) {
        QJsonObject itemObj = value.toObject();
        QString hash = itemObj["imageHash"].toString();
        
        if (!hash.isEmpty() && !CacheManager::instance().isCached(hash)) {
            QString dlUrl = urlsObj[hash].toString();
            if (!dlUrl.isEmpty() && !missingUrls.contains(hash)) {
                missingUrls.insert(hash, dlUrl);
            }
        }
    }

    m_downloadCount = 0;
    m_downloadTotal = missingUrls.size();

    if (m_downloadTotal == 0) {
        finalizeSyncDown(); // Функция finalizeSyncDown теперь отработает идеально!
        return;
    }

    emit cloudSyncProgress(0, m_downloadTotal);

    for (auto it = missingUrls.begin(); it != missingUrls.end(); ++it) {
        QString hash = it.key();
        QString url = it.value();

        QNetworkRequest request((QUrl(url)));
        QNetworkReply *dlReply = m_networkManager->get(request);
        dlReply->setProperty("hash", hash);
        connect(dlReply, &QNetworkReply::finished, this, &CloudController::onS3DownloadFinished);
    }
}

void CloudController::onS3DownloadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();

    QString hash = reply->property("hash").toString();
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        // Проверяем, что это правильная PNG
        QImage img;
        if (img.loadFromData(data)) {
            CacheManager::instance().saveToCache(hash, data);
        }
    }

    m_downloadCount++;
    emit cloudSyncProgress(m_downloadCount, m_downloadTotal);

    if (m_downloadCount >= m_downloadTotal) {
        finalizeSyncDown();
    }
}

void CloudController::finalizeSyncDown()
{
    m_model->clear();
    m_undoStack->clear();

    QJsonObject rootObj = m_downloadedMetadata;

    if (rootObj.contains("canvas")) {
        QJsonObject canvasObj = rootObj["canvas"].toObject();
        BoardController* board = qobject_cast<BoardController*>(parent());
        if (board) {
            int loadedGridSize = canvasObj["gridSize"].toInt(25);
            board->setGridSize(loadedGridSize);

            if (canvasObj.contains("cameraX") && canvasObj.contains("cameraY")) {
                board->setCameraX(canvasObj["cameraX"].toDouble());
                board->setCameraY(canvasObj["cameraY"].toDouble());
                board->setCameraZoom(canvasObj["cameraZoom"].toDouble(0.3));
            }
        }
    }

    QJsonArray itemsArray = rootObj["items"].toArray();
    for (const QJsonValue &value : itemsArray) {
        QJsonObject itemObj = value.toObject();

        QString hash = itemObj["imageHash"].toString();
        QPixmap pixmap;
        if (!hash.isEmpty()) {
            pixmap = CacheManager::instance().loadFromCache(hash);
        }

        ImagoImageData data;
        data.id = itemObj["id"].toString();
        data.source = QUrl();
        data.pixmap = pixmap;
        data.imageHash = hash;
        data.x = itemObj["pos_x"].toDouble();
        data.y = itemObj["pos_y"].toDouble();
        data.width = itemObj["width"].toDouble();
        data.height = itemObj["height"].toDouble();
        data.rotation = itemObj["rotation"].toDouble();
        data.zValue = itemObj["zValue"].toDouble();
        data.label = itemObj["label"].toString();
        data.cropX = itemObj["cropX"].toDouble();
        data.cropY = itemObj["cropY"].toDouble();
        data.cropWidth = itemObj["cropWidth"].toDouble();
        data.cropHeight = itemObj["cropHeight"].toDouble();
        data.opacity = itemObj.contains("opacity") ? itemObj["opacity"].toDouble() : 1.0;
        
        m_model->addImage(data);
    }

    emit cloudSyncFinished();
}
