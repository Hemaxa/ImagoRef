#include "BoardController.h"
#include "StackController.h"
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
        itemObj["label"] = item.label;

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

void BoardController::toggleSelection(int index)
{
    ImageData item = m_model->getItem(index);
    m_model->setSelected(index, !item.selected);
    emit selectionChanged();
}

void BoardController::deselectItem(int index)
{
    m_model->setSelected(index, false);
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

void BoardController::selectInRect(qreal x, qreal y, qreal width, qreal height, bool addToSelection)
{
    QRectF selectionRect(x, y, width, height);
    selectionRect = selectionRect.normalized();
    
    if (!addToSelection) {
        m_model->clearSelection();
    }
    
    for (int i = 0; i < m_model->count(); ++i) {
        ImageData item = m_model->getItem(i);
        QRectF itemRect(item.x, item.y, item.width, item.height);
        
        if (selectionRect.intersects(itemRect)) {
            m_model->setSelected(i, true);
        }
    }
    
    emit selectionChanged();
}

int BoardController::hitTest(qreal x, qreal y) const
{
    // Обходим элементы с конца (верхние слои сначала)
    for (int i = m_model->count() - 1; i >= 0; --i) {
        ImageData item = m_model->getItem(i);
        QRectF rect(item.x, item.y, item.width, item.height);
        if (rect.contains(x, y)) {
            return i;
        }
    }
    return -1;
}

qreal BoardController::getItemX(int index) const
{
    if (index >= 0 && index < m_model->count())
        return m_model->getItem(index).x;
    return 0;
}

qreal BoardController::getItemY(int index) const
{
    if (index >= 0 && index < m_model->count())
        return m_model->getItem(index).y;
    return 0;
}

bool BoardController::isItemSelected(int index) const
{
    if (index >= 0 && index < m_model->count())
        return m_model->getItem(index).selected;
    return false;
}

qreal BoardController::getItemWidth(int index) const
{
    if (index >= 0 && index < m_model->count())
        return m_model->getItem(index).width;
    return 0;
}

qreal BoardController::getItemHeight(int index) const
{
    if (index >= 0 && index < m_model->count())
        return m_model->getItem(index).height;
    return 0;
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

    QVariantList indices = m_model->selectedIndices();
    if (indices.isEmpty()) return;

    m_undoStack->beginMacro("Привязка к сетке");

    for (const QVariant &v : indices) {
        int i = v.toInt();
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

void BoardController::cropImage(int index, qreal cropX, qreal cropY, qreal cropWidth, qreal cropHeight)
{
    if (index < 0 || index >= m_model->count()) return;
    
    ImageData item = m_model->getItem(index);
    if (item.pixmap.isNull()) return;
    
    // Координаты пришли в системе отображения (item.width x item.height)
    // Non-descructive crop logic
    
    // 1. Calculate scale between Visual Item and Source Image
    qreal sourceWidth = (item.cropWidth > 0) ? item.cropWidth : item.pixmap.width();
    qreal sourceHeight = (item.cropHeight > 0) ? item.cropHeight : item.pixmap.height();
    
    qreal scaleX = sourceWidth / item.width;
    qreal scaleY = sourceHeight / item.height;
    
    // 2. Calculate new crop rect in Source Coordinates
    qreal currentSourceX = (item.cropWidth > 0) ? item.cropX : 0;
    qreal currentSourceY = (item.cropHeight > 0) ? item.cropY : 0;
    
    qreal newSourceCropX = currentSourceX + (cropX * scaleX);
    qreal newSourceCropY = currentSourceY + (cropY * scaleY);
    qreal newSourceCropW = cropWidth * scaleX;
    qreal newSourceCropH = cropHeight * scaleY;
    
    // 3. Calculate new position (учитывая transformOrigin = Item.Center в QML)
    // Центр crop-прямоугольника в локальных координатах элемента
    qreal cropCenterLocalX = cropX + cropWidth / 2.0;
    qreal cropCenterLocalY = cropY + cropHeight / 2.0;
    
    // Центр элемента до обрезки (transformOrigin)
    qreal oldCenterX = item.width / 2.0;
    qreal oldCenterY = item.height / 2.0;
    
    // Смещение центра crop относительно центра элемента
    qreal relX = cropCenterLocalX - oldCenterX;
    qreal relY = cropCenterLocalY - oldCenterY;
    
    // Применяем поворот к смещению
    qreal rad = item.rotation * M_PI / 180.0;
    qreal rotatedRelX = relX * std::cos(rad) - relY * std::sin(rad);
    qreal rotatedRelY = relX * std::sin(rad) + relY * std::cos(rad);
    
    // Центр crop-прямоугольника в координатах сцены
    qreal sceneCropCenterX = (item.x + oldCenterX) + rotatedRelX;
    qreal sceneCropCenterY = (item.y + oldCenterY) + rotatedRelY;
    
    // Новая позиция (x, y) = top-left нового элемента, пересчитанная из центра
    qreal newX = sceneCropCenterX - cropWidth / 2.0;
    qreal newY = sceneCropCenterY - cropHeight / 2.0;
    
    // Push undo command
    m_undoStack->push(new CropImageCommand(
        m_model, index,
        QPointF(item.x, item.y), QSizeF(item.width, item.height),
        QRectF(item.cropX, item.cropY, item.cropWidth, item.cropHeight),
        QPointF(newX, newY), QSizeF(cropWidth, cropHeight),
        QRectF(newSourceCropX, newSourceCropY, newSourceCropW, newSourceCropH)
    ));
}

void BoardController::setLabelForSelected(const QString &label)
{
    QVariantList indices = m_model->selectedIndices();
    if (indices.isEmpty()) return;

    m_undoStack->beginMacro("Подписать изображения");

    for (const QVariant &v : indices) {
        int idx = v.toInt();
        ImageData item = m_model->getItem(idx);
        if (item.label != label) {
            m_undoStack->push(new SetLabelCommand(m_model, idx, item.label, label));
        }
    }

    m_undoStack->endMacro();
}

void BoardController::arrangeAll()
{
    int count = m_model->count();
    if (count == 0) return;

    int spacing = SettingsManager::instance().arrangeSpacing();

    // Собираем все элементы с учётом повёрнутых bounding box
    struct ItemInfo {
        int index;
        qreal itemWidth;   // Оригинальная ширина элемента
        qreal itemHeight;  // Оригинальная высота элемента
        qreal bbWidth;     // Ширина bounding box с учётом поворота
        qreal bbHeight;    // Высота bounding box с учётом поворота
    };
    
    QVector<ItemInfo> items;
    items.reserve(count);
    qreal totalArea = 0;
    
    for (int i = 0; i < count; ++i) {
        ImageData data = m_model->getItem(i);
        
        // Вычисляем bounding box повёрнутого элемента
        qreal rad = std::fabs(data.rotation) * M_PI / 180.0;
        qreal cosA = std::fabs(std::cos(rad));
        qreal sinA = std::fabs(std::sin(rad));
        qreal bbW = data.width * cosA + data.height * sinA;
        qreal bbH = data.width * sinA + data.height * cosA;
        
        items.append({i, data.width, data.height, bbW, bbH});
        totalArea += (bbW + spacing) * (bbH + spacing);
    }

    // Сортируем по высоте bounding box (убывание) для shelf-packing
    std::sort(items.begin(), items.end(), [](const ItemInfo &a, const ItemInfo &b) {
        return a.bbHeight > b.bbHeight;
    });

    // Вычисляем оптимальную ширину ряда из общей площади — стремимся к квадратному расположению
    qreal maxRowWidth = std::sqrt(totalArea) * 1.3;
    if (maxRowWidth < 800) maxRowWidth = 800;

    // Начальная позиция (центрируем вокруг центра сцены)
    qreal startX = 10000.0 - maxRowWidth / 2.0;
    
    // Предвычисляем общую высоту для центрирования по Y
    qreal totalHeight = 0;
    {
        qreal cx = 0, rh = 0;
        for (const auto &item : items) {
            if (cx + item.bbWidth > maxRowWidth && cx > 0) {
                totalHeight += rh + spacing;
                cx = 0;
                rh = 0;
            }
            cx += item.bbWidth + spacing;
            rh = std::max(rh, item.bbHeight);
        }
        totalHeight += rh;
    }
    qreal startY = 10000.0 - totalHeight / 2.0;

    // Shelf-packing с bounding box размерами
    QVector<int> sortedIndices;
    QVector<QPointF> oldPositions;
    QVector<QPointF> newPositions;

    qreal currentX = startX;
    qreal currentY = startY;
    qreal rowHeight = 0;

    for (const auto &item : items) {
        if (currentX - startX + item.bbWidth > maxRowWidth && currentX > startX) {
            currentX = startX;
            currentY += rowHeight + spacing;
            rowHeight = 0;
        }

        // Позиция bounding box top-left = (currentX, currentY)
        // Центр bounding box = (currentX + bbW/2, currentY + bbH/2)
        // Центр элемента совпадает с центром bounding box (поворот вокруг центра)
        // Позиция элемента (x, y) = центр - (itemWidth/2, itemHeight/2)
        qreal newX = currentX + item.bbWidth / 2.0 - item.itemWidth / 2.0;
        qreal newY = currentY + item.bbHeight / 2.0 - item.itemHeight / 2.0;

        ImageData data = m_model->getItem(item.index);
        sortedIndices.append(item.index);
        oldPositions.append(QPointF(data.x, data.y));
        newPositions.append(QPointF(newX, newY));

        currentX += item.bbWidth + spacing;
        rowHeight = std::max(rowHeight, item.bbHeight);
    }

    m_undoStack->push(new ArrangeCommand(m_model, sortedIndices, oldPositions, newPositions));
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
