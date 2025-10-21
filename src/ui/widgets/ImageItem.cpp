#include "ImageItem.h"
#include "StackManager.h"

#include <QPen>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGuiApplication>
#include <QCursor>
#include <QStyleOptionGraphicsItem>
#include <QUndoStack>
#include <QTransform>

ImageItem::ImageItem(const QPixmap &pixmap, QUndoStack *undoStack, QGraphicsItem *parent)
    : QObject(nullptr),
    QGraphicsPixmapItem(pixmap, parent),
    m_originalPixmap(pixmap),
    m_undoStack(undoStack)
{
    m_currentBounds = pixmap.rect();
    setTransformOriginPoint(m_currentBounds.center());
    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    m_handles.resize(8);
}

ImageItem::~ImageItem()
{
}

QRectF ImageItem::boundingRect() const {
    return m_currentBounds;
}

void ImageItem::setResizeMode(bool enabled) {
    if (m_isResizing == enabled) return;
    m_isResizing = enabled;
    if (m_isResizing) {
        setFlag(ItemIsMovable, false);
        updateHandlesPos();
    } else {
        setFlag(ItemIsMovable, true);
    }
    update();
}

void ImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawPixmap(boundingRect(), pixmap(), pixmap().rect());

    if ((option->state & QStyle::State_Selected) || m_isHovered) {
        painter->setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawRect(boundingRect());
    }
    if (m_isResizing) {
        painter->setBrush(Qt::white);
        painter->setPen(QPen(Qt::black, 2));
        for (const auto &handleRect : m_handles) {
            painter->drawEllipse(handleRect);
        }
    }
}

void ImageItem::updateHandlesPos() {
    const QRectF br = boundingRect();
    const qreal hs = m_handleSize / 2.0;
    m_handles[0].setRect(br.topLeft().x() - hs, br.topLeft().y() - hs, m_handleSize, m_handleSize); //TopLeft
    m_handles[1].setRect(br.center().x() - hs, br.top() - hs, m_handleSize, m_handleSize); //Top
    m_handles[2].setRect(br.topRight().x() - hs, br.topRight().y() - hs, m_handleSize, m_handleSize); //TopRight
    m_handles[3].setRect(br.right() - hs, br.center().y() - hs, m_handleSize, m_handleSize); //Right
    m_handles[4].setRect(br.bottomRight().x() - hs, br.bottomRight().y() - hs, m_handleSize, m_handleSize); //BottomRight
    m_handles[5].setRect(br.center().x() - hs, br.bottom() - hs, m_handleSize, m_handleSize); //Bottom
    m_handles[6].setRect(br.left() - hs, br.center().y() - hs, m_handleSize, m_handleSize); //Left
    m_handles[7].setRect(br.bottomLeft().x() - hs, br.bottomLeft().y() - hs, m_handleSize, m_handleSize); //BottomLeft
}

ImageItem::Handle ImageItem::getHandleAt(const QPointF &pos) {
    for (int i = 0; i < m_handles.size(); ++i) {
        if (m_handles[i].contains(pos)) {
            return static_cast<Handle>(i + 1);
        }
    }
    return None;
}

void ImageItem::setCursorForHandle(Handle handle) {
    switch (handle) {
    case TopLeft:
    case BottomRight:
        setCursor(Qt::SizeFDiagCursor); break;
    case TopRight:
    case BottomLeft:
        setCursor(Qt::SizeBDiagCursor); break;
    case Top:
    case Bottom:
        setCursor(Qt::SizeVerCursor); break;
    case Right:
    case Left:
        setCursor(Qt::SizeHorCursor); break;
    default:
        setCursor(Qt::ArrowCursor); break;
    }
}

void ImageItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    m_isHovered = true;
    update();
    QGraphicsPixmapItem::hoverEnterEvent(event);
}

void ImageItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    m_isHovered = false;
    setCursor(Qt::ArrowCursor);
    update();
    QGraphicsPixmapItem::hoverLeaveEvent(event);
}

void ImageItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    m_initialPos = this->pos();
    m_initialRect = this->boundingRect();

    if (m_isResizing) {
        m_activeHandle = getHandleAt(event->pos());
        if (m_activeHandle != None) {
            m_isScalingInProgress = true;
            m_initialMousePos = event->scenePos();
            m_initialSceneRect = this->mapToScene(m_initialRect).boundingRect();
            event->accept();
            return;
        }
    }
    QGraphicsPixmapItem::mousePressEvent(event);
}

void ImageItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (m_isResizing && m_activeHandle != None) {
        QRectF newSceneRect = m_initialSceneRect;
        QPointF delta = event->scenePos() - m_initialMousePos;

        switch (m_activeHandle) {
        case TopLeft:     newSceneRect.setTopLeft(newSceneRect.topLeft() + delta); break;
        case Top:         newSceneRect.setTop(newSceneRect.top() + delta.y()); break;
        case TopRight:    newSceneRect.setTopRight(newSceneRect.topRight() + delta); break;
        case Right:       newSceneRect.setRight(newSceneRect.right() + delta.x()); break;
        case BottomRight: newSceneRect.setBottomRight(newSceneRect.bottomRight() + delta); break;
        case Bottom:      newSceneRect.setBottom(newSceneRect.bottom() + delta.y()); break;
        case Left:        newSceneRect.setLeft(newSceneRect.left() + delta.x()); break;
        case BottomLeft:  newSceneRect.setBottomLeft(newSceneRect.bottomLeft() + delta); break;
        default: break;
        }

        if (event->modifiers() & Qt::ShiftModifier) {
            qreal ratio = m_initialRect.height() > 0 ? m_initialRect.width() / m_initialRect.height() : 1.0;
            qreal newWidth = newSceneRect.width();
            qreal newHeight = newSceneRect.height();

            if (m_activeHandle == Top || m_activeHandle == Bottom) { newWidth = newHeight * ratio; }
            else if (m_activeHandle == Left || m_activeHandle == Right) { newHeight = newWidth / ratio; }
            else {
                if (abs(delta.x()) > abs(delta.y())) { newHeight = newWidth / ratio; }
                else { newWidth = newHeight * ratio; }
            }
            switch (m_activeHandle) {
            case TopLeft:     newSceneRect.setTopLeft(newSceneRect.bottomRight() - QPointF(newWidth, newHeight)); break;
            case TopRight:    newSceneRect.setTopRight(newSceneRect.bottomLeft() + QPointF(newWidth, -newHeight)); break;
            case BottomLeft:  newSceneRect.setBottomLeft(newSceneRect.topRight() + QPointF(-newWidth, newHeight)); break;
            case BottomRight: newSceneRect.setBottomRight(newSceneRect.topLeft() + QPointF(newWidth, newHeight)); break;
            default: break;
            }
        }

        prepareGeometryChange();
        setPos(newSceneRect.topLeft());
        m_currentBounds = QRectF(0, 0, newSceneRect.width(), newSceneRect.height());
        updateHandlesPos();
        event->accept();
        return;

    } else if (m_isResizing) {
        setCursorForHandle(getHandleAt(event->pos()));
    }
    QGraphicsPixmapItem::mouseMoveEvent(event);
}

void ImageItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (m_isResizing && m_activeHandle != None) {
        if (m_currentBounds != m_initialRect || pos() != m_initialPos) {
            m_undoStack->push(new ResizeCommand(this, m_initialRect, m_initialPos));
        }
        setPixmap(m_originalPixmap.scaled(m_currentBounds.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        setTransformOriginPoint(m_currentBounds.center());
        m_isScalingInProgress = false;
        m_activeHandle = None;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    } else {
        if (this->pos() != m_initialPos && !m_isResizing) {
            m_undoStack->push(new MoveCommand(this, m_initialPos));
        }
    }
    QGraphicsPixmapItem::mouseReleaseEvent(event);
}

void ImageItem::setGeometry(const QRectF &bounds, const QPointF &pos) {
    prepareGeometryChange();
    setPos(pos);
    m_currentBounds = bounds;
    setTransformOriginPoint(m_currentBounds.center());
    setPixmap(m_originalPixmap.scaled(bounds.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    updateHandlesPos();
    update();
}

void ImageItem::setRotation(qreal angle) {
    QGraphicsPixmapItem::setRotation(angle);
    update();
}

qreal ImageItem::rotation() const {
    return QGraphicsPixmapItem::rotation();
}
