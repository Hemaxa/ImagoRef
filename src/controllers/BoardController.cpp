#include "BoardController.h"
#include "StackController.h"
#include "SettingsManager.h"
#include "ImageProvider.h"
#include "ModelsManager.h"
#include "CloudController.h"
#include "SyncController.h"

BoardController::BoardController(QObject *parent) : QObject(parent)
    , m_model(new ImagoImageModel(this))
    , m_undoStack(new QUndoStack(this))
    , m_fileController(new FileController(m_model, m_undoStack, this))
    , m_selectionController(new SelectionController(m_model, this))
    , m_clipboardController(new ClipboardController(m_model, m_undoStack, this))
    , m_toolController(new ToolController(m_model, m_undoStack, this))
    , m_upscaleController(new UpscaleController(m_model, &ModelsManager::instance(), m_undoStack, this))
    , m_cloudController(new CloudController(m_model, m_undoStack, this))
    , m_syncController(new SyncController(m_model, this))
    , m_metadataDebounceTimer(new QTimer(this))
    , m_gridSize(SettingsManager::instance().getGridSize())
    , m_cameraX(-1)
    , m_cameraY(-1)
    , m_cameraZoom(0.3)
{
    //вызов вспомогательного метода
    connectSignals();
    
    //синхронизировать начальное значение
    m_fileController->setGridSize(m_gridSize);
    
    //настройка таймера для дебаунса метаданных
    m_metadataDebounceTimer->setSingleShot(true);
    m_metadataDebounceTimer->setInterval(2000); // 2 секунды
    connect(m_metadataDebounceTimer, &QTimer::timeout, this, [this]() {
        if (!m_currentBoardId.isEmpty()) {
            m_cloudController->uploadMetadata(m_currentBoardId);
        }
    });
    
    //регистрация модели в глобальном провайдере изображений ImagoImageProvider
    if (ImagoImageProvider::instance()) {
        ImagoImageProvider::instance()->registerModel(m_model);
    }
}

BoardController::~BoardController() {
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
        m_fileController->setGridSize(m_gridSize);
        emit gridSizeChanged(); //оповещаем QML
    });
    
    //синхронизация gridSize из файла при загрузке доски
    connect(m_fileController, &FileController::gridSizeLoaded, this, [this](int loadedGridSize) {
        setGridSize(loadedGridSize);
    });

    //синхронизация камеры из файла при загрузке доски
    connect(m_fileController, &FileController::cameraLoaded, this, [this](qreal x, qreal y, qreal zoom) {
        m_cameraX = x;
        m_cameraY = y;
        m_cameraZoom = zoom;
        emit cameraChanged();
    });

    // Автоматическая загрузка новых картинок в S3 при добавлении их на холст
    connect(m_model, &QAbstractItemModel::rowsInserted, this, [this]() {
        if (!m_currentBoardId.isEmpty()) {
            m_cloudController->syncUp(m_currentBoardId);
        }
    });
}

//геттеры
ImagoImageModel* BoardController::getModel() const { return m_model; }
FileController* BoardController::getFileController() const { return m_fileController; }
SelectionController* BoardController::getSelectionController() const { return m_selectionController; }
ClipboardController* BoardController::getClipboardController() const { return m_clipboardController; }
ToolController* BoardController::getToolController() const { return m_toolController; }
UpscaleController* BoardController::getUpscaleController() const { return m_upscaleController; }
CloudController* BoardController::getCloudController() const { return m_cloudController; }
SyncController* BoardController::getSyncController() const { return m_syncController; }

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
        m_fileController->setGridSize(size);
        SettingsManager::instance().setGridSize(size);
        emit gridSizeChanged();
    }
}

void BoardController::setCameraX(qreal x) {
    if (!qFuzzyCompare(m_cameraX, x)) {
        m_cameraX = x;
        emit cameraChanged();
    }
}

void BoardController::setCameraY(qreal y) {
    if (!qFuzzyCompare(m_cameraY, y)) {
        m_cameraY = y;
        emit cameraChanged();
    }
}

void BoardController::setCameraZoom(qreal zoom) {
    if (!qFuzzyCompare(m_cameraZoom, zoom)) {
        m_cameraZoom = zoom;
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
        m_syncController->sendMoveEvent(item.id, newX, newY);
        scheduleMetadataUpload();
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
        m_syncController->sendResizeEvent(item.id, newX, newY, newWidth, newHeight);
        scheduleMetadataUpload();
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
        m_syncController->sendMoveEvent(item.id, newX, newY);
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
        scheduleMetadataUpload();
    }
    
    m_moveSelectionIndices.clear();
    m_moveSelectionStartPos.clear();
}

void BoardController::scheduleMetadataUpload()
{
    if (!m_currentBoardId.isEmpty()) {
        m_metadataDebounceTimer->start(); // (re)start the timer
    }
}

void BoardController::openCloudBoard(const QString &boardId)
{
    setCurrentBoardId(boardId);
    m_cloudController->syncDown(boardId);
    m_syncController->connectToBoard(boardId);
}

void BoardController::openLocalFile(const QUrl &fileUrl)
{
    setCurrentBoardId("");
    m_fileController->openBoard(fileUrl);
}
