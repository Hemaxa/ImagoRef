#pragma once
#include <QGraphicsPixmapItem> //класс Qt для отображения картинки
#include <QObject> //базовый класс для большинсва объектов в Qt

class ImageItem : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    //конструктор принимает QPixmap (объект кртинки) для отображения
    explicit ImageItem(const QPixmap &pixmap, QGraphicsItem *parent = nullptr);

    //метод для включения/выключения режима изменения размера
    void setResizeMode(bool enabled);

    //переопределение для контроля над границами элемента
    QRectF boundingRect() const override;

protected:
    //метод отрисовки для добавления рамки
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    //методы для эффекта обводки изображения
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    //методы для реализации логики перетаскивания маркеров
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    //перечисление маркеров взаимодействия с изображением
    enum Handle {
        None, TopLeft, Top, TopRight, Right, BottomRight, Bottom, Left, BottomLeft
    };

    //вспомогательные функции для взаимодействия с курсором
    void updateHandlesPos();
    Handle getHandleAt(const QPointF &pos);
    void setCursorForHandle(Handle handle);

    //флаги
    bool m_isHovered = false;
    bool m_isResizing = false;
    bool m_isScalingInProgress = false;

    //оригинальное изображение для качественного масштабирования
    QPixmap m_originalPixmap;

    Handle m_activeHandle = None; //выбранный маркер сейчас тащим
    QRectF m_initialSceneRect; //прямоугольник изображения в момент начала перетаскивания
    QRectF m_currentBounds; //прямоугольник, описывающий текущие границы элемента
    QPointF m_initialMousePos; //позиция мыши в момент начала перетаскивания

    QVector<QRectF> m_handles; //массив с прямоугольниками маркеров для отрисовки и проверки попадания
    const int m_handleSize = 15; //размер маркера в пикселях
};
