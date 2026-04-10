#include "BoardController.h"
#include "StackController.h"
#include "SettingsManager.h"
#include "ImageProvider.h"
#include "ModelsManager.h"
#include "NetworkController.h"
#include "CacheManager.h"

#include <QPainter>
#include <QImage>
#include <QStandardPaths>
#include <QDir>
#include <QBuffer>
#include <QCryptographicHash>
#include <QJsonObject>

BoardController::BoardController(QObject *parent) : QObject(parent)
    , m_model(new ImagoImageModel(this))
    , m_undoStack(new QUndoStack(this))
    , m_storageController(new StorageController(m_model, m_undoStack, this))
    , m_selectionController(new SelectionController(m_model, this))
    , m_clipboardController(new ClipboardController(m_model, m_undoStack, this))
    , m_toolController(new ToolController(m_model, m_undoStack, this))
    , m_upscaleController(new UpscaleController(m_model, &ModelsManager::instance(), m_undoStack, this))
    , m_networkController(new NetworkController(m_storageController, this))
    , m_gridSize(SettingsManager::instance().getGridSize())
    , m_cameraX(-1)
    , m_cameraY(-1)
    , m_cameraZoom(0.3)
{
    //вызов вспомогательного метода
    connectSignals();
    
    //синхронизировать начальное значение
    m_storageController->setGridSize(m_gridSize);
    
    //регистрация модели в глобальном провайдере изображений ImagoImageProvider
    if (ImagoImageProvider::instance()) {
        ImagoImageProvider::instance()->registerModel(m_model);
    }
}

BoardController::~BoardController() {
    if (!m_model->getAllItems().isEmpty()) {
        generateBoardPreview();
    }

    if (ImagoImageProvider::instance()) {
        ImagoImageProvider::instance()->unregisterModel(m_model); //убираем модель из регистрации
    }
}

//метод связывания сигналов
void BoardController::connectSignals()
{
    //синхронизация стека с сигналами об изменении
    connect(m_undoStack, &QUndoStack::canUndoChanged, this, &BoardController::undoStateChanged);
    connect(m_undoStack, &QUndoStack::canRedoChanged, this, &BoardController::redoStateChanged);
    
    //синхронизация gridSize с SettingsManager
    connect(&SettingsManager::instance(), &SettingsManager::gridSizeChanged, this, [this]() {
        m_gridSize = SettingsManager::instance().getGridSize();
        m_storageController->setGridSize(m_gridSize);
        m_storageController->updateBoardMetadata(m_cameraX, m_cameraY, m_cameraZoom);
        emit gridSizeChanged(); //оповещаем QML
    });
    
    //синхронизация gridSize из файла при загрузке доски
    connect(m_storageController, &StorageController::gridSizeLoaded, this, [this](int loadedGridSize) {
        setGridSize(loadedGridSize);
    });

    //синхронизация камеры из файла при загрузке доски
    connect(m_storageController, &StorageController::cameraLoaded, this, [this](qreal x, qreal y, qreal zoom) {
        m_cameraX = x;
        m_cameraY = y;
        m_cameraZoom = zoom;
        emit cameraChanged();
    });

    // Сохранение новых картинок в локальную базу данных и постановка в очередь
    connect(m_model, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex&, int first, int last) {
        
        if (m_storageController->isLoading()) return; // Защита от бесконечного цикла, которую мы добавили ранее

        for (int i = first; i <= last; ++i) {
            ImagoImageData item = m_model->getItem(i);
            
            // --- НОВЫЙ БЛОК: СОХРАНЯЕМ ФАЙЛ В ЛОКАЛЬНЫЙ КЭШ ---
            // Теперь, даже если выключить интернет и закрыть приложение, картинка загрузится с диска!
            if (!item.pixmap.isNull() && !item.imageHash.isEmpty()) {
                QByteArray bytes;
                QBuffer buffer(&bytes);
                buffer.open(QIODevice::WriteOnly);
                item.pixmap.save(&buffer, "PNG");
                CacheManager::instance().saveToCache(item.imageHash, bytes);
            }
            // ---------------------------------------------------

            m_storageController->upsertItem(item);
            
            QJsonObject payload;
            payload["image_id"] = item.id;
            m_storageController->enqueueSyncTask("UPLOAD_IMAGE", payload);
        }
    });

    // Очередь на удаление
    connect(m_model, &QAbstractItemModel::rowsAboutToBeRemoved, this, [this](const QModelIndex&, int first, int last) {
        
        // ДОБАВЬ ЭТУ СТРОКУ: Иначе при смене доски приложение будет удалять все картинки с сервера!
        if (m_storageController->isLoading()) return;

        for (int i = first; i <= last; ++i) {
            ImagoImageData item = m_model->getItem(i);
            m_storageController->deleteItem(item.id);
            
            QJsonObject payload;
            payload["image_id"] = item.id;
            m_storageController->enqueueSyncTask("DELETE_ITEM", payload);
        }
    });

    // Обработка входящих обновлений по сети (LWW)
    connect(m_networkController, &NetworkController::itemUpdatedFromNetwork, this, [this](const QString& itemId) {
        ImagoImageData data = m_storageController->getItemFromDb(itemId);
        if (!data.id.isEmpty()) {
            m_model->updateItemData(itemId, data);
        } else {
            m_model->removeImageById(itemId);
        }
    });
}

//геттеры
ImagoImageModel* BoardController::getModel() const { return m_model; }
StorageController* BoardController::getStorageController() const { return m_storageController; }
SelectionController* BoardController::getSelectionController() const { return m_selectionController; }
ClipboardController* BoardController::getClipboardController() const { return m_clipboardController; }
ToolController* BoardController::getToolController() const { return m_toolController; }
UpscaleController* BoardController::getUpscaleController() const { return m_upscaleController; }
NetworkController* BoardController::getNetworkController() const { return m_networkController; }

bool BoardController::getCanUndo() const { return m_undoStack->canUndo(); }
bool BoardController::getCanRedo() const { return m_undoStack->canRedo(); }
int BoardController::getGridSize() const { return m_gridSize; }
qreal BoardController::getCameraX() const { return m_cameraX; }
qreal BoardController::getCameraY() const { return m_cameraY; }
qreal BoardController::getCameraZoom() const { return m_cameraZoom; }
QString BoardController::getCurrentBoardId() const { return m_currentBoardId; }

//сеттеры
void BoardController::setGridSize(int size)
{
    if (m_gridSize != size && size > 0) {
        m_gridSize = size;
        m_storageController->setGridSize(size);
        SettingsManager::instance().setGridSize(size);
        m_storageController->updateBoardMetadata(m_cameraX, m_cameraY, m_cameraZoom);
        emit gridSizeChanged();
    }
}

void BoardController::setCameraX(qreal x) {
    if (!qFuzzyCompare(m_cameraX, x)) {
        m_cameraX = x;
        m_storageController->updateBoardMetadata(m_cameraX, m_cameraY, m_cameraZoom);
        emit cameraChanged();
    }
}

void BoardController::setCameraY(qreal y) {
    if (!qFuzzyCompare(m_cameraY, y)) {
        m_cameraY = y;
        m_storageController->updateBoardMetadata(m_cameraX, m_cameraY, m_cameraZoom);
        emit cameraChanged();
    }
}

void BoardController::setCameraZoom(qreal zoom) {
    if (!qFuzzyCompare(m_cameraZoom, zoom)) {
        m_cameraZoom = zoom;
        m_storageController->updateBoardMetadata(m_cameraX, m_cameraY, m_cameraZoom);
        emit cameraChanged();
    }
}

void BoardController::setCurrentBoardId(const QString &id) {
    if (m_currentBoardId != id) {
        m_currentBoardId = id;
        emit currentBoardIdChanged();
    }
}

//Undo/Redo
void BoardController::undo()
{
    m_undoStack->undo();
}

void BoardController::redo()
{
    m_undoStack->redo();
}

//отслеживание перемещения
void BoardController::beginMove(int index)
{
    ImagoImageData item = m_model->getItem(index);
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
        
        
        ImagoImageData item = m_model->getItem(index);
        m_storageController->upsertItem(item);
        QJsonObject payload;
        payload["image_id"] = item.id;
        payload["deltaX"] = newX - m_moveStartPos.x();
        payload["deltaY"] = newY - m_moveStartPos.y();
        m_storageController->enqueueSyncTask("UPDATE_ITEM", payload);
    }
}

//отслеживание изменения размера
void BoardController::beginResize(int index)
{
    ImagoImageData item = m_model->getItem(index);
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
        
        
        ImagoImageData item = m_model->getItem(index);
        m_storageController->upsertItem(item);
        QJsonObject payload;
        payload["image_id"] = item.id;
        payload["newWidth"] = newWidth;
        payload["newHeight"] = newHeight;
        m_storageController->enqueueSyncTask("UPDATE_ITEM", payload);
    }
}

//отслеживание перемещения выделения
void BoardController::beginMoveSelection()
{
    m_moveSelectionIndices.clear();
    m_moveSelectionStartPos.clear();

    QVariantList selected = m_model->getSelectedIndices();
    for (const QVariant& v : selected) {
        int index = v.toInt();
        ImagoImageData item = m_model->getItem(index);
        m_moveSelectionIndices.append(index);
        m_moveSelectionStartPos.append(QPointF(item.x, item.y));
    }
}

void BoardController::updateMoveSelection(qreal deltaX, qreal deltaY)
{
    for (int i = 0; i < m_moveSelectionIndices.size(); ++i) {
        int index = m_moveSelectionIndices[i];
        QPointF startPos = m_moveSelectionStartPos[i];
        
        qreal newX = startPos.x() + deltaX;
        qreal newY = startPos.y() + deltaY;
        
        //приклеивание к границам рабочей области
        ImagoImageData item = m_model->getItem(index);
        qreal itemW = item.width > 0 ? item.width : 100;
        qreal itemH = item.height > 0 ? item.height : 100;
        
        newX = qMax(0.0, qMin(30000.0 - itemW, newX));
        newY = qMax(0.0, qMin(30000.0 - itemH, newY));
        
        m_model->setPosition(index, newX, newY);
    }
}

void BoardController::endMoveSelection()
{
    if (m_moveSelectionIndices.isEmpty())
        return;
        
    QVector<QPointF> newPositions;
    bool hasChanges = false;
    
    for (int i = 0; i < m_moveSelectionIndices.size(); ++i) {
        int index = m_moveSelectionIndices[i];
        ImagoImageData item = m_model->getItem(index);
        QPointF newPos(item.x, item.y);
        newPositions.append(newPos);
        
        if (newPos != m_moveSelectionStartPos[i]) {
            hasChanges = true;
        }
    }
    
    if (hasChanges) {
        m_undoStack->push(new MoveImagesCommand(
            m_model, m_moveSelectionIndices,
            m_moveSelectionStartPos, newPositions
        ));
        
        for (int i = 0; i < m_moveSelectionIndices.size(); ++i) {
            ImagoImageData item = m_model->getItem(m_moveSelectionIndices[i]);
            m_storageController->upsertItem(item);
            
            QJsonObject payload;
            payload["image_id"] = item.id;
            payload["deltaX"] = newPositions[i].x() - m_moveSelectionStartPos[i].x();
            payload["deltaY"] = newPositions[i].y() - m_moveSelectionStartPos[i].y();
            m_storageController->enqueueSyncTask("UPDATE_ITEM", payload);
        }
    }
    
    m_moveSelectionIndices.clear();
    m_moveSelectionStartPos.clear();
}

void BoardController::openCloudBoard(const QString &boardId)
{
    if (!m_model->getAllItems().isEmpty()) {
        generateBoardPreview();
    }
    setCurrentBoardId(boardId);
    m_storageController->loadBoardFromDb(boardId);
    m_networkController->connectToBoard(boardId);
}

void BoardController::openLocalFile(const QUrl &fileUrl)
{
    if (!m_model->getAllItems().isEmpty()) {
        generateBoardPreview();
    }
    setCurrentBoardId("");
    m_networkController->disconnectFromBoard();
    m_storageController->openBoard(fileUrl);
}

QString BoardController::generateBoardPreview()
{
    if (m_model->getAllItems().isEmpty()) return "";

    QString identifier = m_currentBoardId.isEmpty() ? m_storageController->getCurrentFilePath() : m_currentBoardId;
    if (identifier.isEmpty()) return "";
    
    QRectF bounds;
    bool first = true;
    for (const auto& item : m_model->getAllItems()) {
        if (item.pixmap.isNull()) continue;
        
        QRectF rect(item.x, item.y, item.width, item.height);
        if (first) {
            bounds = rect;
            first = false;
        } else {
            bounds = bounds.united(rect);
        }
    }
    
    if (first) return "";

    double MARGIN = bounds.width() * 0.1;
    bounds.adjust(-MARGIN, -MARGIN, MARGIN, MARGIN);

    qreal side = qMax(bounds.width(), bounds.height());
    bounds = QRectF(bounds.center().x() - side/2.0, bounds.center().y() - side/2.0, side, side);

    QImage preview(512, 512, QImage::Format_ARGB32);
    preview.fill(Qt::transparent);

    QPainter p(&preview);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    
    p.scale(512.0 / bounds.width(), 512.0 / bounds.height());
    p.translate(-bounds.x(), -bounds.y());

    for (const auto& item : m_model->getAllItems()) {
        if (item.pixmap.isNull()) continue;
        p.save();
        p.translate(item.x + item.width/2.0, item.y + item.height/2.0);
        p.rotate(item.rotation);
        p.translate(-item.width/2.0, -item.height/2.0);
        
        p.setOpacity(item.opacity);
        p.drawPixmap(0, 0, item.width, item.height, item.pixmap);
        p.restore();
    }
    p.end();

    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir().mkpath(cacheDir);

    QString hash = QString(QCryptographicHash::hash(identifier.toUtf8(), QCryptographicHash::Md5).toHex());
    QString path = QDir(cacheDir).filePath("preview_" + hash + ".png");
    
    preview.save(path, "PNG");
    
    QString finalPath = "file://" + path;
    SettingsManager::instance().updateBoardPreview(identifier, finalPath);
    
    return finalPath;
}
