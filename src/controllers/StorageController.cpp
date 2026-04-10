#include "StorageController.h"
#include "ImageModel.h"
#include "BoardController.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QImage>
#include <QBuffer>
#include <JlCompress.h>
#include <QTemporaryDir>
#include <QDir>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QSqlError>
#include <QUuid>
#include <QDateTime>
#include "CacheManager.h"
#include <QDebug>
#include <QVariantMap>

StorageController::StorageController(ImagoImageModel *model, QUndoStack *undoStack, QObject *parent) : QObject(parent)
    , m_model(model)
    , m_undoStack(undoStack)
{
}

StorageController::~StorageController()
{
}

void StorageController::initDatabase()
{
    if (QSqlDatabase::database().isValid()) {
        return;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataDir);
    QString dbPath = QDir(appDataDir).filePath("imago_local.db");
    
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qWarning() << "Failed to open local database:" << db.lastError().text();
        return;
    }

    QSqlQuery q;
    q.exec("CREATE TABLE IF NOT EXISTS boards ("
           "id TEXT PRIMARY KEY, "
           "name TEXT, "
           "updated_at INTEGER, "
           "is_dirty INTEGER DEFAULT 1)");
           
    q.exec("CREATE TABLE IF NOT EXISTS items ("
           "id TEXT PRIMARY KEY, "
           "board_id TEXT, "
           "type TEXT, "
           "x REAL, "
           "y REAL, "
           "width REAL, "
           "height REAL, "
           "z_index INTEGER, "
           "payload TEXT, "
           "updated_at INTEGER, "
           "is_dirty INTEGER DEFAULT 1, "
           "is_deleted INTEGER DEFAULT 0)");
}

QVariantList StorageController::getLocalBoards()
{
    QVariantList boards;
    QSqlQuery q;
    if (q.exec("SELECT id, name FROM boards ORDER BY updated_at DESC")) {
        while (q.next()) {
            QVariantMap map;
            map["id"] = q.value("id").toString();
            map["name"] = q.value("name").toString();
            boards.append(map);
        }
    } else {
        qWarning() << "Failed to fetch boards:" << q.lastError().text();
    }
    return boards;
}

void StorageController::createLocalBoard(const QString& id, const QString& title)
{
    QSqlQuery q;
    q.prepare("INSERT INTO boards (id, name, updated_at, is_dirty) VALUES (:id, :name, :updated_at, 1)");
    q.bindValue(":id", id);
    q.bindValue(":name", title);
    q.bindValue(":updated_at", QDateTime::currentSecsSinceEpoch());
    if (!q.exec()) {
        qWarning() << "Failed to create board:" << q.lastError().text();
    }
}

void StorageController::renameLocalBoard(const QString& id, const QString& newTitle)
{
    QSqlQuery q;
    q.prepare("UPDATE boards SET name = :name, updated_at = :updated_at, is_dirty = 1 WHERE id = :id");
    q.bindValue(":name", newTitle);
    q.bindValue(":updated_at", QDateTime::currentSecsSinceEpoch());
    q.bindValue(":id", id);
    if (!q.exec()) {
        qWarning() << "Failed to rename board:" << q.lastError().text();
    }
}

void StorageController::deleteLocalBoard(const QString& id)
{
    QSqlQuery q;
    q.prepare("DELETE FROM items WHERE board_id = :id");
    q.bindValue(":id", id);
    q.exec();

    q.prepare("DELETE FROM boards WHERE id = :id");
    q.bindValue(":id", id);
    if (!q.exec()) {
        qWarning() << "Failed to delete board:" << q.lastError().text();
    }
}

QString StorageController::getCurrentFilePath() const
{
    return m_currentFilePath;
}

QString StorageController::getWindowTitle() const
{
    QString title;
    if (m_currentFilePath.isEmpty()) {
        title = "Новая доска";
    }
    else {
        title = QFileInfo(m_currentFilePath).fileName();
    }
    return title;
}

void StorageController::setGridSize(int gridSize)
{
    m_gridSize = gridSize;
}

void StorageController::newBoard()
{
    m_model->clear();
    m_undoStack->clear();
    m_currentFilePath.clear();
    emit filePathChanged();
}

bool StorageController::openBoard(const QUrl &fileUrl)
{
    QString filePath = fileUrl.toLocalFile();
    return importFromIref(filePath);
}

bool StorageController::saveBoard()
{
    if (m_currentFilePath.isEmpty()) {
        return false;
    }
    return saveBoardAs(QUrl::fromLocalFile(m_currentFilePath));
}

bool StorageController::saveBoardAs(const QUrl &fileUrl)
{
    QString filePath = fileUrl.toLocalFile();
    BoardController* board = qobject_cast<BoardController*>(parent());
    QString boardId = board ? board->getCurrentBoardId() : "";
    return exportToIref(boardId, filePath);
}

bool StorageController::importFromIref(const QString& filePath)
{
    if (!QFile::exists(filePath)) return false;

    QTemporaryDir tempDir;
    if (!tempDir.isValid()) return false;

    if (JlCompress::extractDir(filePath, tempDir.path()).isEmpty()) {
        //игнорирование ошибки распаковки
    }

    QByteArray docData;
    QFile jsonFile(tempDir.path() + "/data.json");
    if (jsonFile.exists() && jsonFile.open(QIODevice::ReadOnly)) {
        docData = jsonFile.readAll();
        jsonFile.close();
    }
    else {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            return false;
        }
        docData = file.readAll();
        file.close();
    }

    QJsonDocument doc = QJsonDocument::fromJson(docData);
    if (doc.isNull()) return false;
    
    m_model->clear();
    m_undoStack->clear();

    QJsonObject rootObj = doc.object();

    BoardController* boardController = qobject_cast<BoardController*>(parent());
    QString boardId = boardController ? boardController->getCurrentBoardId() : "";
    if (boardId.isEmpty()) {
        boardId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        if (boardController) {
            boardController->setCurrentBoardId(boardId);
        }
    }

    QSqlQuery q;
    q.prepare("INSERT OR REPLACE INTO boards (id, name, updated_at, is_dirty) VALUES (:id, :name, :updated, 1)");
    q.bindValue(":id", boardId);
    q.bindValue(":name", QFileInfo(filePath).fileName());
    q.bindValue(":updated", QDateTime::currentSecsSinceEpoch());
    q.exec();

    if (rootObj.contains("canvas")) {
        QJsonObject canvasObj = rootObj["canvas"].toObject();
        int loadedGridSize = canvasObj["gridSize"].toInt(25);
        m_gridSize = loadedGridSize;
        emit gridSizeLoaded(loadedGridSize);
        
        if (canvasObj.contains("cameraX") && canvasObj.contains("cameraY")) {
            qreal camX = canvasObj["cameraX"].toDouble();
            qreal camY = canvasObj["cameraY"].toDouble();
            qreal camZoom = canvasObj["cameraZoom"].toDouble(0.3);
            emit cameraLoaded(camX, camY, camZoom);
        }
    }

    if (rootObj.contains("items")) {
        QJsonArray itemsArray = rootObj["items"].toArray();
        for (const QJsonValue &value : itemsArray) {
            QJsonObject itemObj = value.toObject();

            QByteArray imageData;
            bool isOldFormat = false;
            if (itemObj.contains("imagePath")) {
                QString imagePath = itemObj["imagePath"].toString();
                QFile imgFile(tempDir.path() + "/" + imagePath);
                if (imgFile.open(QIODevice::ReadOnly)) {
                    imageData = imgFile.readAll();
                    imgFile.close();
                }
            } else if (itemObj.contains("imageData")) {
                imageData = QByteArray::fromBase64(itemObj["imageData"].toString().toLatin1());
                isOldFormat = true;
            }

            QImage image;
            if (image.loadFromData(imageData, "PNG")) {
                ImagoImageData data;
                data.id = itemObj["id"].toString();
                data.imageHash = QString(QCryptographicHash::hash(imageData, QCryptographicHash::Sha256).toHex());
                CacheManager::instance().saveToCache(data.imageHash, imageData);

                data.source = QUrl();
                data.pixmap = QPixmap::fromImage(image);
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

                QJsonObject payloadObj;
                payloadObj["rotation"] = data.rotation;
                payloadObj["label"] = data.label;
                payloadObj["cropX"] = data.cropX;
                payloadObj["cropY"] = data.cropY;
                payloadObj["cropWidth"] = data.cropWidth;
                payloadObj["cropHeight"] = data.cropHeight;
                payloadObj["opacity"] = data.opacity;
                payloadObj["imageHash"] = data.imageHash;

                q.prepare("INSERT OR REPLACE INTO items (id, board_id, type, x, y, width, height, z_index, payload, updated_at, is_dirty, is_deleted) "
                          "VALUES (:id, :board_id, :type, :x, :y, :width, :height, :z_index, :payload, :updated, 1, 0)");
                q.bindValue(":id", data.id);
                q.bindValue(":board_id", boardId);
                q.bindValue(":type", "image");
                q.bindValue(":x", data.x);
                q.bindValue(":y", data.y);
                q.bindValue(":width", data.width);
                q.bindValue(":height", data.height);
                q.bindValue(":z_index", data.zValue);
                q.bindValue(":payload", QString(QJsonDocument(payloadObj).toJson(QJsonDocument::Compact)));
                q.bindValue(":updated", QDateTime::currentSecsSinceEpoch());
                q.exec();
            }
        }
    }

    m_currentFilePath = filePath;
    emit filePathChanged();
    emit boardLoaded();
    return true;
}

bool StorageController::exportToIref(const QString& boardId, const QString& filePath)
{
    QString exportBoardId = boardId;
    BoardController* board = qobject_cast<BoardController*>(parent());
    
    if (exportBoardId.isEmpty() && board) {
        exportBoardId = board->getCurrentBoardId();
    }

    QJsonObject rootObj;
    rootObj["version"] = "1.0";
    
    QJsonObject canvasObj;
    canvasObj["gridSize"] = m_gridSize;
    
    if (board) {
        canvasObj["cameraX"] = board->getCameraX();
        canvasObj["cameraY"] = board->getCameraY();
        canvasObj["cameraZoom"] = board->getCameraZoom();
    }
    
    rootObj["canvas"] = canvasObj;

    QTemporaryDir tempDir;
    if (!tempDir.isValid()) return false;

    QDir dir(tempDir.path());
    dir.mkdir("images");

    QJsonArray itemsArray;

    if (!exportBoardId.isEmpty()) {
        QSqlQuery q;
        // ЭКСПОРТИРУЕМ ТОЛЬКО НЕ УДАЛЕННЫЕ ЭЛЕМЕНТЫ
        q.prepare("SELECT * FROM items WHERE board_id = :board_id AND is_deleted = 0");
        q.bindValue(":board_id", exportBoardId);
        if (q.exec()) {
            while (q.next()) {
                QJsonObject itemObj;
                QString id = q.value("id").toString();
                itemObj["id"] = id;
                itemObj["pos_x"] = q.value("x").toDouble();
                itemObj["pos_y"] = q.value("y").toDouble();
                itemObj["width"] = q.value("width").toDouble();
                itemObj["height"] = q.value("height").toDouble();
                itemObj["zValue"] = q.value("z_index").toDouble();

                QString payloadStr = q.value("payload").toString();
                QJsonObject payloadObj = QJsonDocument::fromJson(payloadStr.toUtf8()).object();
                
                itemObj["rotation"] = payloadObj["rotation"].toDouble();
                itemObj["label"] = payloadObj["label"].toString();
                itemObj["cropX"] = payloadObj["cropX"].toDouble();
                itemObj["cropY"] = payloadObj["cropY"].toDouble();
                itemObj["cropWidth"] = payloadObj["cropWidth"].toDouble();
                itemObj["cropHeight"] = payloadObj["cropHeight"].toDouble();
                itemObj["opacity"] = payloadObj.contains("opacity") ? payloadObj["opacity"].toDouble() : 1.0;

                QString imageRelPath = QString("images/%1.png").arg(id);
                itemObj["imagePath"] = imageRelPath;

                QString imageHash = payloadObj["imageHash"].toString();
                QString imageCachePath = CacheManager::instance().getCacheFilePath(imageHash);
                QFile hashFile(imageCachePath);
                QByteArray imageData;
                if (hashFile.open(QIODevice::ReadOnly)) {
                    imageData = hashFile.readAll();
                    hashFile.close();
                }
                
                if (!imageData.isEmpty()) {
                    QString imageAbsPath = dir.filePath(imageRelPath);
                    QFile imgFile(imageAbsPath);
                    if (imgFile.open(QIODevice::WriteOnly)) {
                        imgFile.write(imageData);
                        imgFile.close();
                    }
                }

                itemsArray.append(itemObj);
            }
        }
    }
    
    if (itemsArray.isEmpty()) {
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

            QString imageRelPath = QString("images/%1.png").arg(item.id);
            itemObj["imagePath"] = imageRelPath;
            
            QString imageAbsPath = dir.filePath(imageRelPath);
            item.pixmap.save(imageAbsPath, "PNG");

            itemsArray.append(itemObj);
        }
    }

    rootObj["items"] = itemsArray;

    QJsonDocument doc(rootObj);
    QFile jsonFile(dir.filePath("data.json"));

    if (jsonFile.open(QIODevice::WriteOnly)) {
        jsonFile.write(doc.toJson());
        jsonFile.close();
    } else {
        return false;
    }

    if (QFile::exists(filePath)) {
        QFile::remove(filePath);
    }

    bool success = JlCompress::compressDir(filePath, tempDir.path());

    if (!success) {
        return false;
    }

    m_currentFilePath = filePath;
    emit filePathChanged();
    emit boardSaved();
    return true;
}

void StorageController::upsertItem(const ImagoImageData &item)
{
    BoardController* board = qobject_cast<BoardController*>(parent());
    QString boardId = board ? board->getCurrentBoardId() : "";
    if (boardId.isEmpty()) return;

    QJsonObject payloadObj;
    payloadObj["rotation"] = item.rotation;
    payloadObj["label"] = item.label;
    payloadObj["cropX"] = item.cropX;
    payloadObj["cropY"] = item.cropY;
    payloadObj["cropWidth"] = item.cropWidth;
    payloadObj["cropHeight"] = item.cropHeight;
    payloadObj["opacity"] = item.opacity;
    payloadObj["imageHash"] = item.imageHash;

    QSqlQuery q;
    q.prepare("INSERT OR REPLACE INTO items (id, board_id, type, x, y, width, height, z_index, payload, updated_at, is_dirty, is_deleted) "
              "VALUES (:id, :board_id, :type, :x, :y, :width, :height, :z_index, :payload, :updated, 1, 0)");
    q.bindValue(":id", item.id);
    q.bindValue(":board_id", boardId);
    q.bindValue(":type", "image");
    q.bindValue(":x", item.x);
    q.bindValue(":y", item.y);
    q.bindValue(":width", item.width);
    q.bindValue(":height", item.height);
    q.bindValue(":z_index", item.zValue);
    q.bindValue(":payload", QString(QJsonDocument(payloadObj).toJson(QJsonDocument::Compact)));
    q.bindValue(":updated", QDateTime::currentSecsSinceEpoch());
    
    if (!q.exec()) {
        qWarning() << "Failed to upsert item:" << q.lastError().text();
    }

    // Обновляем статус доски
    QSqlQuery qBoard;
    qBoard.prepare("UPDATE boards SET is_dirty = 1, updated_at = :updated WHERE id = :id");
    qBoard.bindValue(":updated", QDateTime::currentSecsSinceEpoch());
    qBoard.bindValue(":id", boardId);
    qBoard.exec();
}

void StorageController::deleteItem(const QString &itemId)
{
    QSqlQuery q;
    q.prepare("UPDATE items SET is_deleted = 1, is_dirty = 1, updated_at = :updated WHERE id = :id");
    q.bindValue(":updated", QDateTime::currentSecsSinceEpoch());
    q.bindValue(":id", itemId);
    
    if (!q.exec()) {
        qWarning() << "Failed to soft delete item:" << q.lastError().text();
    }
}

void StorageController::updateBoardMetadata(qreal camX, qreal camY, qreal camZoom)
{
    BoardController* board = qobject_cast<BoardController*>(parent());
    QString boardId = board ? board->getCurrentBoardId() : "";
    if (boardId.isEmpty()) return;

    QSqlQuery q;
    q.prepare("UPDATE boards SET updated_at = :updated, is_dirty = 1 WHERE id = :id");
    q.bindValue(":updated", QDateTime::currentSecsSinceEpoch());
    q.bindValue(":id", boardId);
    q.exec();
}

bool StorageController::applyNetworkDelta(const QString& actionType, const QJsonObject& payload)
{
    // Этот метод мы позже адаптируем под Snapshot Sync, пока оставляем базовую вставку
    QString itemId = payload["id"].toString();
    if (itemId.isEmpty()) itemId = payload["item_id"].toString();
    if (itemId.isEmpty()) return false;

    qint64 networkUpdated = payload.contains("updated_at") ? payload["updated_at"].toVariant().toLongLong() : QDateTime::currentSecsSinceEpoch();

    if (actionType == "DELETE_ITEM") {
        deleteItem(itemId);
        return true;
    }

    QSqlQuery q;
    q.prepare("INSERT OR REPLACE INTO items (id, board_id, type, x, y, width, height, z_index, payload, updated_at, is_dirty, is_deleted) "
              "VALUES (:id, :board_id, :type, :x, :y, :width, :height, :z_index, :payload, :updated_at, 0, 0)");
    q.bindValue(":id", itemId);
    q.bindValue(":board_id", payload["board_id"].toString());
    q.bindValue(":type", payload["type"].toString("image"));
    q.bindValue(":x", payload["x"].toDouble());
    q.bindValue(":y", payload["y"].toDouble());
    q.bindValue(":width", payload["width"].toDouble());
    q.bindValue(":height", payload["height"].toDouble());
    q.bindValue(":z_index", payload["z_index"].toInt());
    
    QJsonObject innerPayload = payload["payload"].toObject();
    q.bindValue(":payload", QString(QJsonDocument(innerPayload).toJson(QJsonDocument::Compact)));
    q.bindValue(":updated_at", networkUpdated);
    
    return q.exec();
}

ImagoImageData StorageController::getItemFromDb(const QString& itemId)
{
    ImagoImageData data;
    QSqlQuery q;
    q.prepare("SELECT * FROM items WHERE id = :id AND is_deleted = 0");
    q.bindValue(":id", itemId);
    if (!q.exec() || !q.next()) return data;

    data.id = q.value("id").toString();
    data.x = q.value("x").toDouble();
    data.y = q.value("y").toDouble();
    data.width = q.value("width").toDouble();
    data.height = q.value("height").toDouble();
    data.zValue = q.value("z_index").toDouble();
    
    QJsonObject inner = QJsonDocument::fromJson(q.value("payload").toString().toUtf8()).object();
    data.rotation = inner["rotation"].toDouble();
    data.label = inner["label"].toString();
    data.cropX = inner["cropX"].toDouble();
    data.cropY = inner["cropY"].toDouble();
    data.cropWidth = inner["cropWidth"].toDouble();
    data.cropHeight = inner["cropHeight"].toDouble();
    data.opacity = inner.contains("opacity") ? inner["opacity"].toDouble() : 1.0;
    data.imageHash = inner["imageHash"].toString();
    
    QString imageCachePath = CacheManager::instance().getCacheFilePath(data.imageHash);
    data.pixmap.load(imageCachePath);
    
    return data;
}

void StorageController::loadBoardFromDb(const QString& boardId)
{
    m_isLoading = true;

    m_model->clear();
    m_undoStack->clear();
    m_currentFilePath.clear();
    emit filePathChanged();

    QSqlQuery q;
    // ГРУЗИМ ТОЛЬКО АКТИВНЫЕ ЭЛЕМЕНТЫ
    q.prepare("SELECT id FROM items WHERE board_id = :board_id AND is_deleted = 0");
    q.bindValue(":board_id", boardId);
    
    if (q.exec()) {
        while (q.next()) {
            QString itemId = q.value("id").toString();
            ImagoImageData data = getItemFromDb(itemId);
            if (!data.id.isEmpty()) {
                m_model->addImage(data); 
            }
        }
    }
    
    m_isLoading = false;
    emit boardLoaded();
}

QString StorageController::getBoardTitle(const QString& boardId)
{
    QSqlQuery q;
    q.prepare("SELECT name FROM boards WHERE id = :id");
    q.bindValue(":id", boardId);
    
    if (q.exec() && q.next()) {
        return q.value("name").toString();
    }
    
    return "Recovered Board";
}

// ==========================================
// НОВЫЕ МЕТОДЫ ДЛЯ OFFLINE FIRST СИНХРОНИЗАЦИИ
// ==========================================

QJsonObject StorageController::getUnsyncedBoardState(const QString& boardId)
{
    QJsonObject state;
    QJsonArray updatedItems;
    QJsonArray deletedItems;

    QSqlQuery q;
    q.prepare("SELECT * FROM items WHERE board_id = :board_id AND is_dirty = 1");
    q.bindValue(":board_id", boardId);
    
    if (q.exec()) {
        while (q.next()) {
            if (q.value("is_deleted").toInt() == 1) {
                deletedItems.append(q.value("id").toString());
            } else {
                QJsonObject itemObj;
                itemObj["id"] = q.value("id").toString();
                itemObj["board_id"] = boardId;
                itemObj["type"] = q.value("type").toString();
                itemObj["x"] = q.value("x").toDouble();
                itemObj["y"] = q.value("y").toDouble();
                itemObj["width"] = q.value("width").toDouble();
                itemObj["height"] = q.value("height").toDouble();
                itemObj["z_index"] = q.value("z_index").toInt();
                itemObj["payload"] = QJsonDocument::fromJson(q.value("payload").toString().toUtf8()).object();
                
                updatedItems.append(itemObj);
            }
        }
    }
    
    state["updated_items"] = updatedItems;
    state["deleted_items"] = deletedItems;
    return state;
}

void StorageController::markAsSynced(const QString& boardId)
{
    // Физически удаляем из БД записи, которые были помечены как is_deleted
    QSqlQuery qDelete;
    qDelete.prepare("DELETE FROM items WHERE board_id = :board_id AND is_deleted = 1");
    qDelete.bindValue(":board_id", boardId);
    qDelete.exec();

    // Снимаем флаг is_dirty с остальных
    QSqlQuery qUpdate;
    qUpdate.prepare("UPDATE items SET is_dirty = 0 WHERE board_id = :board_id");
    qUpdate.bindValue(":board_id", boardId);
    qUpdate.exec();

    // Снимаем флаг с самой доски
    QSqlQuery qBoard;
    qBoard.prepare("UPDATE boards SET is_dirty = 0 WHERE id = :id");
    qBoard.bindValue(":id", boardId);
    qBoard.exec();
}