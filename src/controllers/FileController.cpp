#include "FileController.h"
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

FileController::FileController(ImageItemModel *model, QUndoStack *undoStack, QObject *parent) : QObject(parent)
    , m_model(model)
    , m_undoStack(undoStack)
{}

QString FileController::getCurrentFilePath() const
{
    return m_currentFilePath;
}

//метод формирования заголовка окна
QString FileController::getWindowTitle() const
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

void FileController::setGridSize(int gridSize)
{
    m_gridSize = gridSize;
}

void FileController::newBoard()
{
    m_model->clear();
    m_undoStack->clear();
    m_currentFilePath.clear();
    emit filePathChanged();
}

bool FileController::openBoard(const QUrl &fileUrl)
{
    QString filePath = fileUrl.toLocalFile();
    if (!QFile::exists(filePath)) return false;

    QTemporaryDir tempDir;
    if (!tempDir.isValid()) return false;

    if (JlCompress::extractDir(filePath, tempDir.path()).isEmpty()) {
        // JlCompress returns a list of files; if empty, it might have failed to extract
        // (but we will still try to load data.json just in case)
    }

    QByteArray docData;
    QFile jsonFile(tempDir.path() + "/data.json");
    if (jsonFile.exists() && jsonFile.open(QIODevice::ReadOnly)) {
        docData = jsonFile.readAll();
        jsonFile.close();
    } else {
        // Fallback to reading the file directly if it wasn't a zip (old format compatibility)
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

    if (rootObj.contains("canvas")) {
        QJsonObject canvasObj = rootObj["canvas"].toObject();
        int loadedGridSize = canvasObj["gridSize"].toInt(25);
        m_gridSize = loadedGridSize;
        emit gridSizeLoaded(loadedGridSize);
        
        if (canvasObj.contains("cameraX") && canvasObj.contains("cameraY")) {
            qreal camX = canvasObj["cameraX"].toDouble();
            qreal camY = canvasObj["cameraY"].toDouble();
            qreal camZoom = canvasObj["cameraZoom"].toDouble(0.3); // default zoom is 0.3
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
                ImageData data;
                data.id = itemObj["id"].toString();
                data.source = QUrl();
                data.pixmap = QPixmap::fromImage(image);
                data.x = itemObj["pos_x"].toDouble();
                data.y = itemObj["pos_y"].toDouble();
                data.width = itemObj["width"].toDouble();
                data.height = itemObj["height"].toDouble();
                data.rotation = itemObj["rotation"].toDouble();
                data.zValue = itemObj["zValue"].toDouble();
                data.label = itemObj["label"].toString();
                
                // Data loss fix (crop parameters)
                data.cropX = itemObj["cropX"].toDouble();
                data.cropY = itemObj["cropY"].toDouble();
                data.cropWidth = itemObj["cropWidth"].toDouble();
                data.cropHeight = itemObj["cropHeight"].toDouble();
                
                m_model->addImage(data);
            }
        }
    }

    m_currentFilePath = filePath;
    emit filePathChanged();
    emit boardLoaded();
    return true;
}

bool FileController::saveBoard()
{
    if (m_currentFilePath.isEmpty()) {
        return false;
    }
    return saveBoardAs(QUrl::fromLocalFile(m_currentFilePath));
}

bool FileController::saveBoardAs(const QUrl &fileUrl)
{
    QString filePath = fileUrl.toLocalFile();
    
    QJsonObject rootObj;
    rootObj["version"] = "1.0";
    
    QJsonObject canvasObj;
    canvasObj["gridSize"] = m_gridSize;
    
    BoardController* board = qobject_cast<BoardController*>(parent());
    if (board) {
        // Save camera properties
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
    
    for (const ImageData &item : m_model->allItems()) {
        QJsonObject itemObj;
        itemObj["id"] = item.id;
        itemObj["pos_x"] = item.x;
        itemObj["pos_y"] = item.y;
        itemObj["width"] = item.width;
        itemObj["height"] = item.height;
        itemObj["rotation"] = item.rotation;
        itemObj["zValue"] = item.zValue;
        itemObj["label"] = item.label;
        
        // Data loss fix (crop parameters)
        itemObj["cropX"] = item.cropX;
        itemObj["cropY"] = item.cropY;
        itemObj["cropWidth"] = item.cropWidth;
        itemObj["cropHeight"] = item.cropHeight;

        QString imageRelPath = QString("images/%1.png").arg(item.id);
        itemObj["imagePath"] = imageRelPath;
        
        QString imageAbsPath = dir.filePath(imageRelPath);
        item.pixmap.save(imageAbsPath, "PNG");

        itemsArray.append(itemObj);
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
