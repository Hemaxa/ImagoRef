#include "CanvasView.h"
#include "ImageItem.h"
#include "StackManager.h"
#include "ThemeManager.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QKeyEvent>
#include <QFileInfo>
#include <QImageReader>
#include <QWheelEvent>
#include <QScrollBar>
#include <QGuiApplication>
#include <QClipboard>
#include <QUrl>
#include <QUndoStack>

CanvasView::CanvasView(QUndoStack *undoStack, QWidget *parent) : QGraphicsView(parent), m_undoStack(undoStack), m_isPanning(false)
{
    //создание сцены и связывание ее с просмотром
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    //разрешение на принятие "брошенных" на виджет файлов (для Drag & Drop)
    setAcceptDrops(true);

    //сглаживание для более красивой графики
    setRenderHint(QPainter::Antialiasing);

    //устанавливается режим перетаскивания, RubberBandDrag позволяет выделять несколько элементов рамкой
    setDragMode(QGraphicsView::RubberBandDrag);

    //задание шага сетки по умолчанию
    m_gridSize = 25;
    m_gridDotColor = ThemeManager::instance().getColor("gridColor");

    //установка фокуса на виджет
    setFocusPolicy(Qt::StrongFocus);
}

CanvasView::~CanvasView()
{
}

void CanvasView::drawBackground(QPainter *painter, const QRectF &rect) {
    QGraphicsView::drawBackground(painter, rect);
    QPen pen(m_gridDotColor);
    pen.setWidth(3);
    painter->setPen(pen);
    qreal left = int(rect.left()) - (int(rect.left()) % m_gridSize);
    qreal top = int(rect.top()) - (int(rect.top()) % m_gridSize);
    for (qreal x = left; x < rect.right(); x += m_gridSize) {
        for (qreal y = top; y < rect.bottom(); y += m_gridSize) {
            painter->drawPoint(int(x), int(y));
        }
    }
}

// методы Drag & Drop (без m_supportedFormats)
void CanvasView::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        for (const QUrl &url : event->mimeData()->urls()) {
            if (url.isLocalFile()) {
                // Простая проверка на URL, реальная проверка будет в dropEvent
                event->acceptProposedAction();
                return;
            }
        }
    }
    event->ignore();
}

void CanvasView::dragMoveEvent(QDragMoveEvent *event) {
    event->acceptProposedAction();
}

void CanvasView::dropEvent(QDropEvent *event) {
    QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats(); // Локальная проверка
    for (const QUrl &url : event->mimeData()->urls()) {
        if (url.isLocalFile()) {
            QString extension = QFileInfo(url.toLocalFile()).suffix().toLower();
            if (supportedFormats.contains(extension.toUtf8())) {
                QString filePath = url.toLocalFile();
                QPixmap pixmap(filePath);

                if (!pixmap.isNull()) {
                    ImageItem *imageItem = new ImageItem(pixmap, m_undoStack);
                    imageItem->setPos(mapToScene(event->position().toPoint()));
                    m_undoStack->push(new AddCommand(imageItem, m_scene));
                }
            }
        }
    }
    event->acceptProposedAction();
}

void CanvasView::keyPressEvent(QKeyEvent *event) {
    // ⛔️ Key_Delete УДАЛЕН (теперь обрабатывается m_deleteAction в MainWindow) ⛔️
    // Вы можете вернуть его, если хотите, чтобы клавиша работала
    // напрямую, а не только через QAction. Если да, то:
    /*
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        // Вам нужно будет подключить сигнал к DeleteTool
        // или сделать DeleteTool доступным здесь.
        // Проще оставить обработку QAction в MainWindow.
    }
    */

    if (event->key() == Qt::Key_Escape) {
        for (QGraphicsItem *it : m_scene->selectedItems()) {
            if (ImageItem *imgItem = dynamic_cast<ImageItem*>(it)) {
                imgItem->setResizeMode(false);
            }
        }
        event->accept();
        return;
    }
    QGraphicsView::keyPressEvent(event);
}

// ⛔️ МЕТОДЫ ИНСТРУМЕНТОВ УДАЛЕНЫ ⛔️

void CanvasView::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() & Qt::ControlModifier) {
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        const int delta = event->angleDelta().y();
        const qreal scaleFactor = (delta > 0) ? 1.15 : 1.0 / 1.15;
        scale(scaleFactor, scaleFactor);
        setTransformationAnchor(QGraphicsView::AnchorViewCenter);
        event->accept();
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void CanvasView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        m_isPanning = true;
        m_panStartPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    if (!itemAt(event->pos())) {
        const auto selected = m_scene->selectedItems();
        for (QGraphicsItem *it : selected) {
            if (ImageItem *imgItem = dynamic_cast<ImageItem*>(it)) {
                imgItem->setResizeMode(false);
            }
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void CanvasView::mouseMoveEvent(QMouseEvent *event) {
    if (m_isPanning) {
        QPoint delta = event->pos() - m_panStartPos;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        m_panStartPos = event->pos();
        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void CanvasView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void CanvasView::setGridSize(int size) {
    if (size > 0) {
        m_gridSize = size;
        scene()->invalidate(scene()->sceneRect(), QGraphicsScene::BackgroundLayer);
    }
}

void CanvasView::setGridColor(const QColor &color)
{
    if (m_gridDotColor == color) return;
    m_gridDotColor = color;
    scene()->invalidate(scene()->sceneRect(), QGraphicsScene::BackgroundLayer);
}

int CanvasView::getGridSize() const {
    return m_gridSize;
}
