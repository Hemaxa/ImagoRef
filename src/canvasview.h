#pragma once
#include <QGraphicsView> //виджет отображения 2d-сцены в Qt

//прямые объявления классов
class QGraphicsScene;
class QKeyEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;

class CanvasView : public QGraphicsView {
    Q_OBJECT

public:
    explicit CanvasView(QWidget *parent = nullptr);

    //геттеры
    int gridSize() const;

//слоты могут вызываться в ответ на сигналы
public slots:
    void deleteSelectedItems();
    void pasteImage();
    void snapAllToGrid();
    void zoomIn();
    void zoomOut();

    void setGridSize(int size);

protected:
    //отрисовка фона
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    //методы для добавления файлов (Drag & Drop)
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    //метод изменения масштаба
    void wheelEvent(QWheelEvent *event) override;

    //методы для перемещения колесиком мыши
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    //событие нажатие клавиши для локальных действий
    void keyPressEvent(QKeyEvent *event) override;

private:
    QGraphicsScene *m_scene; //создания экземпляра сцены
    const QList<QByteArray> m_supportedFormats; //получение поддерживаемых форматов изображений Qt
    bool m_isPanning;
    QPoint m_panStartPos;

    int m_gridSize; //параметр шага сетки
};
