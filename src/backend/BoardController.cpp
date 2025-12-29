#include "BoardController.h"
#include "UndoCommands.h"
#include "SettingsManager.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>
#include <QImage>
#include <QBuffer>
#include <QImageReader>
#include <cmath>

BoardController::BoardController(QObject *parent)
    : QObject(parent)
    , m_model(new ImageItemModel(this))
    , m_undoStack(new QUndoStack(this))
    , m_gridSize(SettingsManager::instance().gridSize())
{
    connectUndoSignals();
    
    // Синхронизация с SettingsManager
    connect(&SettingsManager::instance(), &SettingsManager::gridSizeChanged, this, [this]() {
        m_gridSize = SettingsManager::instance().gridSize();
        emit gridSizeChanged();
    });
}

BoardController::~BoardController()
{
}

void BoardController::connectUndoSignals()
{
    connect(m_undoStack, &QUndoStack::canUndoChanged, this, &BoardController::undoStateChanged);
    connect(m_undoStack, &QUndoStack::canRedoChanged, this, &BoardController::redoStateChanged);
}

ImageItemModel* BoardController::model() const { return m_model; }
QString BoardController::currentFilePath() const { return m_currentFilePath; }

QString BoardController::windowTitle() const
{
    QString title = "ImagoRef - ";
    if (m_currentFilePath.isEmpty()) {
        title += "Новая доска";
    } else {
        title += QFileInfo(m_currentFilePath).fileName();
    }
    return title;
}

bool BoardController::canUndo() const { return m_undoStack->canUndo(); }
bool BoardController::canRedo() const { return m_undoStack->canRedo(); }
int BoardController::gridSize() const { return m_gridSize; }

void BoardController::setGridSize(int size)
{
    if (m_gridSize != size && size > 0) {
        m_gridSize = size;
        SettingsManager::instance().setGridSize(size);
        emit gridSizeChanged();
    }
}

bool BoardController::hasSelection() const
{
    return !m_model->selectedIndices().isEmpty();
}

// ============ Файловые операции ============

void BoardController::newBoard()
{
    m_model->clear();
    m_undoStack->clear();
    m_currentFilePath.clear();
    emit filePathChanged();
}

bool BoardController::openBoard(const QUrl &fileUrl)
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
        setGridSize(canvasObj["gridSize"].toInt(25));
    }

    if (rootObj.contains("items")) {
        QJsonArray itemsArray = rootObj["items"].toArray();
        for (const QJsonValue &value : itemsArray) {
            QJsonObject itemObj = value.toObject();

            QByteArray byteArray = QByteArray::fromBase64(itemObj["imageData"].toString().toLatin1());
            QImage image;
            if (image.loadFromData(byteArray, "PNG")) {
                ImageData data;
                data.source = QUrl(); // Данные встроены
                data.pixmap = QPixmap::fromImage(image);
                data.x = itemObj["pos_x"].toDouble();
                data.y = itemObj["pos_y"].toDouble();
                data.width = itemObj["width"].toDouble();
                data.height = itemObj["height"].toDouble();
                data.rotation = itemObj["rotation"].toDouble();
                data.zValue = itemObj["zValue"].toDouble();
                m_model->addImage(data);
            }
        }
    }

    m_currentFilePath = filePath;
    emit filePathChanged();
    emit boardLoaded();
    return true;
}

bool BoardController::saveBoard()
{
    if (m_currentFilePath.isEmpty()) {
        return false; // Нужен saveBoardAs
    }
    return saveBoardAs(QUrl::fromLocalFile(m_currentFilePath));
}

bool BoardController::saveBoardAs(const QUrl &fileUrl)
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

        // Сохраняем изображение как base64
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

// ============ Операции с изображениями ============

void BoardController::addImage(const QUrl &imageUrl, qreal x, qreal y)
{
    QString filePath = imageUrl.toLocalFile();
    QPixmap pixmap(filePath);
    
    if (pixmap.isNull()) {
        return;
    }

    ImageData data;
    data.source = imageUrl;
    data.pixmap = pixmap;
    data.x = x;
    data.y = y;
    data.width = pixmap.width();
    data.height = pixmap.height();
    
    m_model->addImage(data);
    
    // Создаем Undo команду
    m_undoStack->push(new AddImageCommand(
        m_model, 
        m_model->getItem(m_model->count() - 1).id,
        imageUrl, x, y, data.width, data.height
    ));
}

void BoardController::addImageFromPixmap(const QByteArray &imageData, qreal x, qreal y)
{
    QImage image;
    if (!image.loadFromData(imageData)) {
        return;
    }
    
    QPixmap pixmap = QPixmap::fromImage(image);
    
    ImageData data;
    data.pixmap = pixmap;
    data.x = x;
    data.y = y;
    data.width = pixmap.width();
    data.height = pixmap.height();
    
    m_model->addImage(data);
    
    m_undoStack->push(new AddImageCommand(
        m_model,
        m_model->getItem(m_model->count() - 1).id,
        QUrl(), x, y, data.width, data.height
    ));
}

void BoardController::pasteFromClipboard(qreal x, qreal y)
{
    const QClipboard *clipboard = QGuiApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();
    
    static const QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();

    if (mimeData->hasUrls()) {
        for (const QUrl &url : mimeData->urls()) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                QString extension = QFileInfo(filePath).suffix().toLower();
                if (supportedFormats.contains(extension.toUtf8())) {
                    addImage(url, x, y);
                    return;
                }
            }
        }
    }

    const QImage image = clipboard->image();
    if (!image.isNull()) {
        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        addImageFromPixmap(data, x, y);
    }
}

// ============ Выделение ============

void BoardController::selectItem(int index, bool addToSelection)
{
    if (!addToSelection) {
        m_model->clearSelection();
    }
    m_model->setSelected(index, true);
    emit selectionChanged();
}

void BoardController::selectAll()
{
    for (int i = 0; i < m_model->count(); ++i) {
        m_model->setSelected(i, true);
    }
    emit selectionChanged();
}

void BoardController::clearSelection()
{
    m_model->clearSelection();
    emit selectionChanged();
}

// ============ Инструменты ============

void BoardController::deleteSelected()
{
    QVariantList indices = m_model->selectedIndices();
    if (indices.isEmpty()) return;

    QList<int> intIndices;
    for (const QVariant &v : indices) {
        intIndices.append(v.toInt());
    }

    m_undoStack->push(new RemoveImageCommand(m_model, intIndices));
    emit selectionChanged();
}

void BoardController::snapToGrid()
{
    if (m_gridSize <= 0) return;

    m_undoStack->beginMacro("Привязка к сетке");

    for (int i = 0; i < m_model->count(); ++i) {
        ImageData item = m_model->getItem(i);
        qreal newX = std::round(item.x / m_gridSize) * m_gridSize;
        qreal newY = std::round(item.y / m_gridSize) * m_gridSize;

        if (item.x != newX || item.y != newY) {
            m_undoStack->push(new MoveImageCommand(
                m_model, i, 
                QPointF(item.x, item.y),
                QPointF(newX, newY)
            ));
        }
    }

    m_undoStack->endMacro();
}

void BoardController::rotateSelected(qreal angleDelta)
{
    QVariantList indices = m_model->selectedIndices();
    if (indices.isEmpty()) return;

    m_undoStack->beginMacro(angleDelta > 0 ? "Вращение по часовой" : "Вращение против часовой");

    for (const QVariant &v : indices) {
        int idx = v.toInt();
        m_undoStack->push(new RotateImageCommand(m_model, idx, angleDelta));
    }

    m_undoStack->endMacro();
}

// ============ Undo/Redo ============

void BoardController::undo()
{
    m_undoStack->undo();
}

void BoardController::redo()
{
    m_undoStack->redo();
}

// ============ Отслеживание перемещения/ресайза ============

void BoardController::beginMove(int index)
{
    ImageData item = m_model->getItem(index);
    m_moveStartPos = QPointF(item.x, item.y);
}

void BoardController::endMove(int index, qreal newX, qreal newY)
{
    if (m_moveStartPos != QPointF(newX, newY)) {
        m_undoStack->push(new MoveImageCommand(
            m_model, index,
            m_moveStartPos,
            QPointF(newX, newY)
        ));
    }
}

void BoardController::beginResize(int index)
{
    ImageData item = m_model->getItem(index);
    m_resizeStartRect = QRectF(0, 0, item.width, item.height);
    m_resizeStartPos = QPointF(item.x, item.y);
}

void BoardController::endResize(int index, qreal newX, qreal newY, qreal newWidth, qreal newHeight)
{
    QRectF newRect(0, 0, newWidth, newHeight);
    QPointF newPos(newX, newY);
    
    if (m_resizeStartRect != newRect || m_resizeStartPos != newPos) {
        m_undoStack->push(new ResizeImageCommand(
            m_model, index,
            m_resizeStartRect, m_resizeStartPos,
            newRect, newPos
        ));
    }
}
