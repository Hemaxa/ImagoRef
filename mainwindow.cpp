#include "mainwindow.h"
#include "canvasview.h"
#include "floatingtoolbar.h"

#include <QToolBar> //класс для панели инструментов
#include <QAction> //класс для действий пользователя
#include <QKeyEvent> //класс для нажатий кнопок на клавиатуре
#include <QIcon> //иконки

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    //создание холста
    m_canvasView = new CanvasView(this); //объект CanvasView создается как дочерний для MainWindow
    //это заничт, что при уничтожении MainWindow, уничтожится и CanvasView

    //холст устанавливается как центральный виджет окна, он займет все доступное пространство
    setCentralWidget(m_canvasView);

    //создание действий и панели инструментов
    createActions();
    createToolBar();

    //настройка окна
    setWindowTitle("ImagoRef");
    resize(1280, 720);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    //обработка наэатия кнопки Tab
    if (event->key() == Qt::Key_Tab) {
        m_toolBar->setVisible(!m_toolBar->isVisible());
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::createActions() {
    //действие "Удалить"
    m_deleteAction = new QAction(QIcon::fromTheme("edit-delete"), "Удалить", this);
    m_deleteAction->setShortcut(QKeySequence::Delete); // Привязываем горячую клавишу Delete

    //соединение сигнала triggered() со слотом deleteSelectedItems() CanvasView
    //когда нажимается кнопка или шорткат, CanvasView получает команду на удаление
    connect(m_deleteAction, &QAction::triggered, m_canvasView, &CanvasView::deleteSelectedItems);

    //действие "Выровнять по сетке"
    m_snapToGridAction = new QAction(QIcon::fromTheme("grid"), "Привязать к сетке", this);
    m_snapToGridAction->setShortcut(tr("Ctrl+G"));
    connect(m_snapToGridAction, &QAction::triggered, m_canvasView, &CanvasView::snapAllToGrid);

    //действие "Увеличить"
    m_zoomInAction = new QAction(QIcon::fromTheme("zoom-in"), "Приблизить", this);
    m_zoomInAction->setShortcut(QKeySequence::ZoomIn); // Ctrl +
    connect(m_zoomInAction, &QAction::triggered, m_canvasView, &CanvasView::zoomIn);

    //действие "Уменьшить"
    m_zoomOutAction = new QAction(QIcon::fromTheme("zoom-out"), "Отдалить", this);
    m_zoomOutAction->setShortcut(QKeySequence::ZoomOut); // Ctrl -
    connect(m_zoomOutAction, &QAction::triggered, m_canvasView, &CanvasView::zoomOut);
}

void MainWindow::createToolBar() {
    //создание панели инструментов
    m_toolBar = new FloatingToolBar(m_canvasView);
    //добавление действий на панель инструментов
    m_toolBar->addAction(m_deleteAction);
    m_toolBar->addAction(m_snapToGridAction);
    m_toolBar->addAction(m_zoomInAction);
    m_toolBar->addAction(m_zoomOutAction);

    //позиционирование всплывающей панели
    m_toolBar->move(15, 15);
}
