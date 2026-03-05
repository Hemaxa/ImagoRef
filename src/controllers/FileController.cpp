#include "FileController.h"
#include "ImageModel.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QImage>
#include <QBuffer>

FileController::FileController(ImageItemModel *model, QUndoStack *undoStack, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_undoStack(undoStack)
{
}

QString FileController::currentFilePath() const
{
    return m_currentFilePath;
}

QString FileController::windowTitle() const
{
    QString title = "ImagoRef - ";
    if (m_currentFilePath.isEmpty()) {
        title += "Новая доска";
    } else {
        title += QFileInfo(m_currentFilePath).fileName();
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
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    m_model->clear();
    m_undoStack->clear();

    QJsonObject rootObj = doc.object();

    if (rootObj.contains("canvas")) {
        QJsonObject canvasObj = rootObj["canvas"].toObject();
        int loadedGridSize = canvasObj["gridSize"].toInt(25);
        m_gridSize = loadedGridSize;
        emit gridSizeLoaded(loadedGridSize);
    }

    if (rootObj.contains("items")) {
        QJsonArray itemsArray = rootObj["items"].toArray();
        for (const QJsonValue &value : itemsArray) {
            QJsonObject itemObj = value.toObject();

            QByteArray byteArray = QByteArray::fromBase64(itemObj["imageData"].toString().toLatin1());
            QImage image;
            if (image.loadFromData(byteArray, "PNG")) {
                ImageData data;
                data.source = QUrl();
                data.pixmap = QPixmap::fromImage(image);
                data.x = itemObj["pos_x"].toDouble();
                data.y = itemObj["pos_y"].toDouble();
                data.width = itemObj["width"].toDouble();
                data.height = itemObj["height"].toDouble();
                data.rotation = itemObj["rotation"].toDouble();
                data.zValue = itemObj["zValue"].toDouble();
                data.label = itemObj["label"].toString();
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
    rootObj["canvas"] = canvasObj;

    QJsonArray itemsArray;
    for (const ImageData &item : m_model->allItems()) {
        QJsonObject itemObj;
        itemObj["pos_x"] = item.x;
        itemObj["pos_y"] = item.y;
        itemObj["width"] = item.width;
        itemObj["height"] = item.height;
        itemObj["rotation"] = item.rotation;
        itemObj["zValue"] = item.zValue;
        itemObj["label"] = item.label;

        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        item.pixmap.save(&buffer, "PNG");
        itemObj["imageData"] = QString::fromLatin1(byteArray.toBase64());

        itemsArray.append(itemObj);
    }
    rootObj["items"] = itemsArray;

    QJsonDocument doc(rootObj);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        m_currentFilePath = filePath;
        emit filePathChanged();
        emit boardSaved();
        return true;
    }
    return false;
}
