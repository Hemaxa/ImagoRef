#pragma once
#include <QGraphicsView>

class QGraphicsScene;
class QKeyEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QUndoStack;

class CanvasView : public QGraphicsView {
    Q_OBJECT

public:
    explicit CanvasView(QUndoStack *undoStack, QWidget *parent = nullptr);
    ~CanvasView();

    int getGridSize() const;

    // ⛔️ СЛОТЫ ИНСТРУМЕНТОВ УДАЛЕНЫ ⛔️

public slots:
    void setGridSize(int size);
    void setGridColor(const QColor &color);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QGraphicsScene *m_scene;
    QUndoStack *m_undoStack;
    // ⛔️ m_supportedFormats УДАЛЕН ⛔️
    bool m_isPanning;
    QPoint m_panStartPos;
    int m_gridSize;
    QColor m_gridDotColor;
};
