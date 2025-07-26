#include "src/canvasview.h"
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

CanvasView::CanvasView(QWidget *parent) : QGraphicsView(parent) {
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
        const QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats(); //поддерживаемые Qt расширения файлов картинок
        for (const QUrl &url : event->mimeData()->urls()) {
            if (url.isLocalFile()) {
                //проверка по расширению
                QString extension = QFileInfo(url.toLocalFile()).suffix().toLower();
                if (supportedFormats.contains(extension.toUtf8())) {
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
    const QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats(); //поддерживаемые Qt расширения файлов картинок
    for (const QUrl &url : event->mimeData()->urls()) {
        if (url.isLocalFile()) {
            QString extension = QFileInfo(url.toLocalFile()).suffix().toLower();
            if (supportedFormats.contains(extension.toUtf8())) {
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
    // Эта функция будет реализована позже.
    // Логика:
    // 1. const int gridSize = 50;
    // 2. Пройтись в цикле по всем QGraphicsItem на сцене (m_scene->items()).
    // 3. Для каждого item:
    //    - QPointF pos = item->pos();
    //    - qreal newX = round(pos.x() / gridSize) * gridSize;
    //    - qreal newY = round(pos.y() / gridSize) * gridSize;
    //    - item->setPos(newX, newY);
}
