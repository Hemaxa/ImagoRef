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

public slots: //слоты могут вызываться в ответ на сигналы
    void deleteSelectedItems();
    void snapAllToGrid();
    void zoomIn();
    void zoomOut();

protected:
    //отрисовка фона
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    //методы для добавления файлов (Drag & Drop)
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    //метод изменения масштаба
    void wheelEvent(QWheelEvent *event) override;

    //событие нажатие клавиши для локальных действий
    void keyPressEvent(QKeyEvent *event) override;

private:
    QGraphicsScene *m_scene; //создания экземпляра сцены
    const QList<QByteArray> m_supportedFormats; //получение поддерживаемых форматов изображений Qt
};
