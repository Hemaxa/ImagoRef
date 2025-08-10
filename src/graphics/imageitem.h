#pragma once
#include <QGraphicsPixmapItem> //класс Qt для отображения картинки
#include <QObject> //базовый класс для большинсва объектов в Qt

//предварительное объявления класса для Undo & Redo
class QUndoStack;

class ImageItem : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    //конструктор принимает QPixmap (объект кртинки) для отображения и указатель на QUndoStack
    explicit ImageItem(const QPixmap &pixmap, QUndoStack *undoStack, QGraphicsItem *parent = nullptr);

    //метод для включения/выключения режима изменения размера
    void setResizeMode(bool enabled);

    //сеттер для геометрии, который будет использоваться командой ResizeCommand
    void setGeometry(const QRectF &bounds, const QPointF &pos);

    //переопределение для контроля над границами элемента
    QRectF boundingRect() const override;

    //поля для доступа из команд
    const QPixmap m_originalPixmap; //оригинальное изображение для качественного масштабирования
    QRectF m_currentBounds; //прямоугольник, описывающий текущие границы элемента

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

    //стек команд для Undo & Redo
    QUndoStack *m_undoStack;

    Handle m_activeHandle = None; //выбранный маркер сейчас тащим
    QRectF m_initialRect;
    QRectF m_initialSceneRect;
    QPointF m_initialMousePos; //позиция мыши в момент начала перетаскивания
    QPointF m_initialPos;

    QVector<QRectF> m_handles; //массив с прямоугольниками маркеров для отрисовки и проверки попадания
    const int m_handleSize = 15; //размер маркера в пикселях
};
