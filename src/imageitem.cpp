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

//срабатывает, когда курсор мыши входит в границы элемента
void ImageItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    //создается перо для рисования рамки: белый цвет, толщина 2 пикселя
    QPen pen(Qt::white, 2);
    setPen(pen); //устанавливает перо для элемента

    //вызывается родительская реализация на всякий случай
    QGraphicsPixmapItem::hoverEnterEvent(event);
}

//срабатывает, когда курсор мыши покидает границы элемента
void ImageItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    //убирается рамка, устанавливается "пустое" перо
    setPen(Qt::NoPen);

    QGraphicsPixmapItem::hoverLeaveEvent(event);
}
