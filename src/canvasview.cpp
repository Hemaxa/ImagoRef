#include "canvasview.h"
#include "imageitem.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QKeyEvent>
#include <QFileInfo>
#include <QImageReader>
#include <QWheelEvent>
#include <QScrollBar>
#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>
#include <QUrl>

CanvasView::CanvasView(QWidget *parent) :
    QGraphicsView(parent),
    m_supportedFormats(QImageReader::supportedImageFormats()),
    m_isPanning(false)
{
    //создание сцену и связываем ее с просмотром
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    //разрешение на принятие "брошенных" на виджет файлов (для Drag & Drop)
    setAcceptDrops(true);

    //сглаживание для более красивой графики
    setRenderHint(QPainter::Antialiasing);

    //устанавливается режим перетаскивания, RubberBandDrag позволяет выделять несколько элементов рамкой
    setDragMode(QGraphicsView::RubberBandDrag);

    //задание шага сетки по умолчанию
    m_gridSize = 25;

    //установка фокуса на виджет
    setFocusPolicy(Qt::StrongFocus);
}

void CanvasView::drawBackground(QPainter *painter, const QRectF &rect) {
    //вызывается родительский метод, чтобы он нарисовал фон (цвет из конструктора)
    QGraphicsView::drawBackground(painter, rect);

    //настройки сетки, нужно добавить возможность изменения пользователем
    const QColor dotColor = QColor(60, 60, 60); //цвет точки
    QPen pen(dotColor);
    pen.setWidth(3); //толщина точки
    painter->setPen(pen);

    //вычисление, с какой точки начать рисовать, чтобы покрыть всю видимую область
    qreal left = int(rect.left()) - (int(rect.left()) % m_gridSize);
    qreal top = int(rect.top()) - (int(rect.top()) % m_gridSize);

    //отрисовка точек
    for (qreal x = left; x < rect.right(); x += m_gridSize) {
        for (qreal y = top; y < rect.bottom(); y += m_gridSize) {
            painter->drawPoint(int(x), int(y));
        }
    }
}

void CanvasView::dragEnterEvent(QDragEnterEvent *event) {
    //проверка, являются ли перетаскиваемые данные локальными по url
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
    //если нажата клавиша Delete или Backspace
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        deleteSelectedItems(); //вызывается функция удаления
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
    //проходимся по всем элементам на сцене
    for (QGraphicsItem *item : m_scene->items()) {
        //убеждаемся, что это ImageItem, хотя на сцене других и нет
        if (dynamic_cast<ImageItem*>(item)) {
            //получаем текущую позицию элемента
            QPointF currentPos = item->pos();

            //вычисляем новую позицию, ближайшую к узлу сетки
            qreal newX = round(currentPos.x() / m_gridSize) * m_gridSize;
            qreal newY = round(currentPos.y() / m_gridSize) * m_gridSize;

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
    //проверка, зажата ли клавиша Ctrl
    if (event->modifiers() & Qt::ControlModifier) {
        //определяется направление прокрутки
        const int delta = event->angleDelta().y();

        qreal scaleFactor;
        if (delta > 0) {
            scaleFactor = 1.15;
        } else {
            scaleFactor = 1.0 / 1.15;
        }

        //масштабирование сцены
        scale(scaleFactor, scaleFactor);

        //"поглощение" события, чтобы оно не обрабатывалось дальше
        event->accept();
    } else {
        //если Ctrl не зажат, передается событие родительскому классу для стандартной прокрутки
        QGraphicsView::wheelEvent(event);
    }
}

//перемещение на центральную кнопку мыши
void CanvasView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        m_isPanning = true;
        m_panStartPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    QGraphicsView::mousePressEvent(event);
}

void CanvasView::mouseMoveEvent(QMouseEvent *event) {
    if (m_isPanning) {
        QPoint delta = event->pos() - m_panStartPos;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        m_panStartPos = event->pos();
        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void CanvasView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

//метод изменения шага сетки
void CanvasView::setGridSize(int size) {
    if (size > 0) {
        m_gridSize = size;
        //обновление фона, чтобы сетка перерисовалась
        scene()->invalidate(scene()->sceneRect(), QGraphicsScene::BackgroundLayer);
    }
}

int CanvasView::gridSize() const {
    return m_gridSize;
}

void CanvasView::pasteImage() {
    //получение доступа к системному буферу обмена
    const QClipboard *clipboard = QGuiApplication::clipboard();

    //проверка, были ли скопированы файлы (ссылки)
    if (clipboard->mimeData()->hasUrls()) {
        bool imagePasted = false;
        for (const QUrl &url : clipboard->mimeData()->urls()) {
            //проверка, что это локальный файл и что его формат поддерживается
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                QString extension = QFileInfo(filePath).suffix().toLower();
                if (m_supportedFormats.contains(extension.toUtf8())) {
                    QPixmap pixmap(filePath);
                    if (!pixmap.isNull()) {
                        ImageItem *imageItem = new ImageItem(pixmap);
                        m_scene->addItem(imageItem);
                        imageItem->setPos(mapToScene(viewport()->rect().center()));
                        imagePasted = true;
                    }
                }
            }
        }
        //если успешно вставлено хотя бы одно изображение из файла, выходим из функции
        if (imagePasted) {
            return;
        }
    }

    //если файлы не найдены, проверка на наличие "сырых" данных изображения
    //позволит копировать из браузера
    const QImage image = clipboard->image();
    if (!image.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(image);
        ImageItem *imageItem = new ImageItem(pixmap);
        m_scene->addItem(imageItem);
        imageItem->setPos(mapToScene(viewport()->rect().center()));
    }
}
