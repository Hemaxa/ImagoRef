#pragma once
#include <QGraphicsPixmapItem> //класс Qt для отображения картинки
#include <QObject> //базовый класс для большинсва объектов в Qt

class ImageItem : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    //конструктор принимает QPixmap (объект кртинки) для отображения
    explicit ImageItem(const QPixmap &pixmap, QGraphicsItem *parent = nullptr);

protected:
    //дополнительные события для эффекта обводки изображения
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
};
