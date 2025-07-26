#include "canvasview.h"
#include "imageitem.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QKeyEvent>
#include <QFileInfo>
#include <QImageReader>
#include <QWheelEvent>

CanvasView::CanvasView(QWidget *parent) : QGraphicsView(parent), m_supportedFormats(QImageReader::supportedImageFormats()) {
    //создание сцену и связываем ее с просмотром
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    //принятие "брошенных" на виджет файлов (для Drag & Drop)
    setAcceptDrops(true);

    //сглаживание для более красивой графики
    setRenderHint(QPainter::Antialiasing);

    //устанавливается режим перетаскивания, RubberBandDrag позволяет выделять несколько элементов рамкой
    setDragMode(QGraphicsView::RubberBandDrag);

    //задание цвет фона
    setBackgroundBrush(QColor(42, 42, 42));

    setFocusPolicy(Qt::StrongFocus);
}

void CanvasView::drawBackground(QPainter *painter, const QRectF &rect) {
    //вызывается родительский метод, чтобы он нарисовал фон (цвет из конструктора)
    QGraphicsView::drawBackground(painter, rect);

    //настройки сетки, нужно добавить возможность изменения пользователем
    const int gridSize = 25; //размер сетки
    const QColor dotColor = QColor(60, 60, 60); //цвет точки
    QPen pen(dotColor);
    pen.setWidth(3); //толщина точки
    painter->setPen(pen);

    //вычисление, с какой точки начать рисовать, чтобы покрыть всю видимую область
    qreal left = int(rect.left()) - (int(rect.left()) % gridSize);
    qreal top = int(rect.top()) - (int(rect.top()) % gridSize);

    //отрисовка точек
    for (qreal x = left; x < rect.right(); x += gridSize) {
        for (qreal y = top; y < rect.bottom(); y += gridSize) {
            painter->drawPoint(int(x), int(y));
        }
    }
}

void CanvasView::dragEnterEvent(QDragEnterEvent *event) {
    //проверка, являются ли перетаскиваемые данные локальными и содержат ли они url
    if (event->mimeData()->hasUrls()) {
        for (const QUrl &url : event->mimeData()->urls()) {
            if (url.isLocalFile()) {
                //проверка по расширению
                QString extension = QFileInfo(url.toLocalFile()).suffix().toLower();
                if (m_supportedFormats.contains(extension.toUtf8())) {
                    //если нашли хотя бы один подходящий файл, разрешаем операцию
                    event->acceptProposedAction();
                    return;
                }
            }
        }
    }
    event->ignore(); //в остальных случаях - не разрешаем
}

void CanvasView::dragMoveEvent(QDragMoveEvent *event) {
    //разрешение перетаскивания
    event->acceptProposedAction();
}

void CanvasView::dropEvent(QDropEvent *event) {
    for (const QUrl &url : event->mimeData()->urls()) {
        if (url.isLocalFile()) {
            QString extension = QFileInfo(url.toLocalFile()).suffix().toLower();
            if (m_supportedFormats.contains(extension.toUtf8())) {
                QString filePath = url.toLocalFile();
                QPixmap pixmap(filePath); //если расширение поддерживается, идет загрузка в QPixmap

                if (!pixmap.isNull()) {
                    ImageItem *imageItem = new ImageItem(pixmap);
                    m_scene->addItem(imageItem); //если в QPixmap что-то есть, то оно уходит на сцену
                    imageItem->setPos(mapToScene(event->position().toPoint()));
                }
            }
        }
    }
    event->acceptProposedAction();
}

void CanvasView::keyPressEvent(QKeyEvent *event) {
    //если нажата клавиша Delete
    if (event->key() == Qt::Key_Delete) {
        deleteSelectedItems(); //вызывается функцию удаления
    } else {
        //для всех остальных клавиш передается управление родительскому классу
        QGraphicsView::keyPressEvent(event);
    }
}

void CanvasView::deleteSelectedItems() {
    //получает список всех выделенных элементов со сцены
    QList<QGraphicsItem*> selected = m_scene->selectedItems();

    //удаление каждого элемента
    //qDeleteAll - функция Qt для удаления всех объектов в контейнере
    qDeleteAll(selected);
}

void CanvasView::snapAllToGrid() {
    const int gridSize = 50; //жестко задаем шаг сетки, позже можно сделать настраиваемым

    //проходимся по всем элементам на сцене
    for (QGraphicsItem *item : m_scene->items()) {
        //убеждаемся, что это ImageItem, хотя на сцене других и нет
        if (dynamic_cast<ImageItem*>(item)) {
            //получаем текущую позицию элемента
            QPointF currentPos = item->pos();

            //вычисляем новую позицию, ближайшую к узлу сетки
            qreal newX = round(currentPos.x() / gridSize) * gridSize;
            qreal newY = round(currentPos.y() / gridSize) * gridSize;

            //устанавливаем новую позицию
            item->setPos(newX, newY);
        }
    }
}

void CanvasView::zoomIn() {
    scale(1.15, 1.15);
}

void CanvasView::zoomOut() {
    scale(1.0 / 1.15, 1.0 / 1.15);
}

void CanvasView::wheelEvent(QWheelEvent *event) {
    // Проверяем, зажата ли клавиша Ctrl
    if (event->modifiers() & Qt::ControlModifier) {
        // Определяем направление прокрутки
        const int delta = event->angleDelta().y();

        qreal scaleFactor;
        if (delta > 0) {
            // Приближаем (крутим колесо "от себя")
            scaleFactor = 1.15;
        } else {
            // Отдаляем (крутим колесо "на себя")
            scaleFactor = 1.0 / 1.15;
        }

        // Масштабируем сцену
        scale(scaleFactor, scaleFactor);

        // "Поглощаем" событие, чтобы оно не обрабатывалось дальше
        event->accept();
    } else {
        // Если Ctrl не зажат, передаем событие родительскому классу для стандартной прокрутки
        QGraphicsView::wheelEvent(event);
    }
}
