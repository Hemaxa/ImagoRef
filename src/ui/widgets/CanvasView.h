#pragma once

#include <QGraphicsView>
#include <QColor>

//прямые объявления классов
class QGraphicsScene;
class QKeyEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QUndoStack;

//класс QGraphicsView в Qt отвечает за отображение и управляемое представление графических объектов, находящихся в сцене QGraphicsScene
class CanvasView : public QGraphicsView {
    Q_OBJECT

public:
    //конструктор с указателем на QUndoStack
    explicit CanvasView(QUndoStack *undoStack, QWidget *parent = nullptr);
    ~CanvasView();

    //геттер для шага сетки
    int getGridSize() const;

public slots:
    //слот утсановки нового размера сетки
    void setGridSize(int size);

    //слот установки нового цвета сетки
    void setGridColor(const QColor &color);

protected:
    //отрисовка фона
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    //методы для добавления файлов (Drag & Drop)
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    //методы для перемещения колесиком мыши
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    //событие нажатие клавиши для локальных действий
    void keyPressEvent(QKeyEvent *event) override;

private:
    QGraphicsScene *m_scene; //создания экземпляра сцены
    QUndoStack *m_undoStack; //создание стека
    bool m_isPanning;
    QPoint m_panStartPos;
    int m_gridSize;
    QColor m_gridDotColor;
};
