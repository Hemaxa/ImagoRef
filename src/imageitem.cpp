#include "imageitem.h"
#include <QPen>     //модуль для рисования контуров, определяет свойсво пера
#include <QPainter> //модуль для рисования

ImageItem::ImageItem(const QPixmap &pixmap, QGraphicsItem *parent)
    : QObject(nullptr), QGraphicsPixmapItem(pixmap, parent) {

    //установка флагов, которые делают элемент интерактивным (то, что можно делать с изображением)
    //1. ItemIsSelectable - можно выделять
    //2. ItemIsMovable - можно перемещать мышью
    //3. ItemSendsGeometryChanges - отправляет сигнал при изменении геометрии
    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);

    //обработка событий наведения мыши
    setAcceptHoverEvents(true);
}

void ImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    //вызывается родительский метод, чтобы само изображение нарисовалось
    QGraphicsPixmapItem::paint(painter, option, widget);

    //если курсор находится над элементом - русиется рамку
    if (m_isHovered) {
        painter->setPen(QPen(Qt::white, 2)); //белое перо, 2px
        painter->drawRect(boundingRect());
    }
}

//срабатывает, когда курсор мыши входит в границы элемента
void ImageItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    m_isHovered = true; //установка флага
    update(); //запрашивается перерисовку элемента, чтобы paint() был вызван снова
    QGraphicsPixmapItem::hoverEnterEvent(event);
}

//срабатывает, когда курсор мыши покидает границы элемента
void ImageItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    m_isHovered = false;
    update();
    QGraphicsPixmapItem::hoverLeaveEvent(event);
}
