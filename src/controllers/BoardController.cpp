#include "BoardController.h"
#include "StackController.h"
#include "SettingsManager.h"

BoardController::BoardController(QObject *parent) : QObject(parent)
    , m_model(new ImageItemModel(this))
    , m_undoStack(new QUndoStack(this))
    , m_gridSize(SettingsManager::instance().gridSize())
    , m_fileController(new FileController(m_model, m_undoStack, this))
    , m_selectionController(new SelectionController(m_model, this))
    , m_clipboardController(new ClipboardController(m_model, m_undoStack, this))
    , m_toolController(new ToolController(m_model, m_undoStack, this))
{
    //вызов вспомогательного метода
    connectUndoSignals();
    
    //синхронизация gridSize с SettingsManager
    connect(&SettingsManager::instance(), &SettingsManager::gridSizeChanged, this, [this]() {
        m_gridSize = SettingsManager::instance().gridSize();
        m_fileController->setGridSize(m_gridSize);
        emit gridSizeChanged(); //оповещаем QML
    });
    
    //синхронизация gridSize из файла при загрузке доски
    connect(m_fileController, &FileController::gridSizeLoaded, this, [this](int loadedGridSize) {
        setGridSize(loadedGridSize);
    });
    
    //синхронизировать начальное значение
    m_fileController->setGridSize(m_gridSize);
}

BoardController::~BoardController() {}

//метод связывания сигналов
void BoardController::connectUndoSignals()
{
    connect(m_undoStack, &QUndoStack::canUndoChanged, this, &BoardController::undoStateChanged);
    connect(m_undoStack, &QUndoStack::canRedoChanged, this, &BoardController::redoStateChanged);
}

//геттеры
ImageItemModel* BoardController::model() const { return m_model; }
FileController* BoardController::fileController() const { return m_fileController; }
SelectionController* BoardController::selectionController() const { return m_selectionController; }
ClipboardController* BoardController::clipboardController() const { return m_clipboardController; }
ToolController* BoardController::toolController() const { return m_toolController; }

bool BoardController::canUndo() const { return m_undoStack->canUndo(); }
bool BoardController::canRedo() const { return m_undoStack->canRedo(); }
int BoardController::gridSize() const { return m_gridSize; }

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

//Undo/Redo
void BoardController::redo()
{
    m_undoStack->redo();
}

//отслеживание перемещения/ресайза
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
