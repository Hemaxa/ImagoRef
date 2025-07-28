#include "mainwindow.h"
#include "canvasview.h"
#include "floatingtoolbar.h"
#include "settingsdialog.h"

#include <QAction> //класс для действий пользователя
#include <QKeyEvent> //класс для нажатий кнопок на клавиатуре
#include <QIcon> //класс иконок
#include <QPropertyAnimation> //класс для анимаций
#include <QFile> //класс для подключения файлов
#include <QSvgRenderer> //класс для работы с svg файлами
#include <QPainter> //класс для рисования на виджетах

//метод перекрашивания иконок
QIcon createRecolorableIcon(const QString& path, const QColor& color, const QSize& size = QSize(24, 24)) {
    //открытие svg файла из ресурсов
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QIcon(); //возвращается пустая иконка, если файл не выбран
    }

    //читается содержимое и заменяется цвет
    QTextStream in(&file);
    QString svgData = in.readAll();
    svgData.replace("currentColor", color.name());

    //создается QPixmap и на нём рисуется изменённый SVG
    QSvgRenderer renderer(svgData.toUtf8());
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);

    return QIcon(pixmap);
}

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
    applyTheme("dark");
}

void MainWindow::onAnimationFinished() {
    //свойство, хранящее указатель на уже завершенную анимацию, очищается
    m_toolBar->setProperty("animation", QVariant());
}

//появление и скрытие боковой панели
void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Tab) { //проверка нажатия клавиши Tab
        event->accept();

        //проверка, запущена ли уже какая-то анимация
        auto* currentAnimation = m_toolBar->property("animation").value<QPropertyAnimation*>();
        if (currentAnimation) {
            //если запущена, то она останавливается (прервет анимацию и позволит запустить новую)
            currentAnimation->stop();
        }

        const int duration = 300; //длительность анимации выхода/ухода (мс)
        const int endXOnScreen = 15; //позиция окна при выходе (px)

        //создание нового объекта анимации
        auto* animation = new QPropertyAnimation(m_toolBar, "pos", this);
        animation->setDuration(duration);
        animation->setEasingCurve(QEasingCurve::InOutCubic);

        //соединение сигнала завершения с слотом для очистки (onAnimationFinished вызывается после завершения анимации)
        connect(animation, &QPropertyAnimation::finished, this, &MainWindow::onAnimationFinished);

        //если панель видима, запускается анимация скрытия
        if (m_toolBar->isVisible()) {
            animation->setEndValue(QPoint(-m_toolBar->width(), m_toolBar->y()));
            connect(animation, &QPropertyAnimation::finished, m_toolBar, &QWidget::hide);
        }
        //если панель скрыта, запускается анимация появления
        else {
            m_toolBar->move(-m_toolBar->width(), m_toolBar->y());
            m_toolBar->show();
            animation->setEndValue(QPoint(endXOnScreen, m_toolBar->y()));
        }

        //сохраняется указатель на новую анимацию и происходит ее запуск
        m_toolBar->setProperty("animation", QVariant::fromValue(animation));
        animation->start(QAbstractAnimation::DeleteWhenStopped); //после завершения анимация удаляется

    } else {
        QMainWindow::keyPressEvent(event);
    }
}

//добавление кнопок и действий на них в боковую панель
void MainWindow::createActions() {
    QColor defaultIconColor("#e0e0e0"); //начальный цвет иконок
    //действие "Удалить"
    m_deleteAction = new QAction(createRecolorableIcon(":/icons/icons/delete.svg", defaultIconColor), "Удалить", this);
    m_deleteAction->setShortcut(QKeySequence::Delete);
    connect(m_deleteAction, &QAction::triggered, m_canvasView, &CanvasView::deleteSelectedItems);
    //соединение сигнала triggered() со слотом deleteSelectedItems() CanvasView
    //когда нажимается кнопка или шорткат, CanvasView получает команду на удаление и выполняет метод deleteSelectedItems()

    //действие "Вставить"
    m_pasteAction = new QAction(createRecolorableIcon(":/icons/icons/paste.svg", defaultIconColor), "Вставить", this);
    m_pasteAction->setShortcut(QKeySequence::Paste);
    connect(m_pasteAction, &QAction::triggered, m_canvasView, &CanvasView::pasteImage);

    //действие "Выровнять по сетке"
    m_snapToGridAction = new QAction(createRecolorableIcon(":/icons/icons/grid.svg", defaultIconColor), "Привязать к сетке", this);
    m_snapToGridAction->setShortcut(tr("Ctrl+G"));
    connect(m_snapToGridAction, &QAction::triggered, m_canvasView, &CanvasView::snapAllToGrid);

    //действие "Изменить размер"
    m_resizeAction = new QAction(createRecolorableIcon(":/icons/icons/resize.svg", defaultIconColor), "Изменить размер", this);
    m_resizeAction->setShortcut(tr("Ctrl+R"));
    connect(m_resizeAction, &QAction::triggered, m_canvasView, &CanvasView::enterResizeMode);

    //действие "Приблизить"
    m_zoomInAction = new QAction(createRecolorableIcon(":/icons/icons/zoom-in.svg", defaultIconColor), "Приблизить", this);
    m_zoomInAction->setShortcuts({QKeySequence::ZoomIn, QKeySequence(tr("Ctrl+=")), QKeySequence(tr("Ctrl++"))});
    connect(m_zoomInAction, &QAction::triggered, m_canvasView, &CanvasView::zoomIn);

    //действие "Отдалить"
    m_zoomOutAction = new QAction(createRecolorableIcon(":/icons/icons/zoom-out.svg", defaultIconColor), "Отдалить", this);
    m_zoomOutAction->setShortcuts({QKeySequence::ZoomOut, QKeySequence(tr("Ctrl--"))});
    connect(m_zoomOutAction, &QAction::triggered, m_canvasView, &CanvasView::zoomOut);

    //действие "Открыть окно настроек"
    m_settingsAction = new QAction(createRecolorableIcon(":/icons/icons/settings.svg", defaultIconColor), "Настройки", this);
    m_settingsAction->setShortcut(tr("Ctrl+,"));
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::openSettingsDialog);
}

void MainWindow::createToolBar() {
    m_toolBar = new FloatingToolBar(m_canvasView);
    //m_canvasView передается как родительский виджет
    //позволяет панели инструментов взаимодействовать с холстом

    //добавление действий на панель инструментов
    m_toolBar->addAction(m_deleteAction);
    m_toolBar->addAction(m_pasteAction);
    m_toolBar->addAction(m_snapToGridAction);
    m_toolBar->addAction(m_resizeAction);
    m_toolBar->addAction(m_zoomInAction);
    m_toolBar->addAction(m_zoomOutAction);
    m_toolBar->addAction(m_settingsAction);

    //позиционирование панели
    m_toolBar->move(15, 15);
}

void MainWindow::openSettingsDialog() {
    //передается текущий размер сетки и имя текущей темы
    SettingsDialog dialog(m_canvasView->getGridSize(), m_currentThemeName, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_canvasView->setGridSize(dialog.gridSize());
        applyTheme(dialog.theme());
    }
}

void MainWindow::applyTheme(const QString &themeName) {
    m_currentThemeName = themeName;
    QString path = QString(":/themes/themes/%1.qss").arg(themeName);

    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        QString styleSheet = stream.readAll();
        setStyleSheet(styleSheet);
        file.close();
    } else {
        setStyleSheet("");
    }

    QColor iconColor;
    QColor gridColor;

    if (themeName == "dark") {
        iconColor = QColor("#e0e0e0");
        gridColor = QColor("#1f1f1f");
    } else if (themeName == "light") {
        iconColor = QColor("#2a2a2a");
        gridColor = QColor("#cccccc");
    } else {
        // Цвет по умолчанию, если тема не найдена
        iconColor = QColor("#e0e0e0");
        gridColor = QColor("#1f1f1f");
    }

    m_canvasView->setGridColor(gridColor);

    m_deleteAction->setIcon(createRecolorableIcon(":/icons/icons/delete.svg", iconColor));
    m_pasteAction->setIcon(createRecolorableIcon(":/icons/icons/paste.svg", iconColor));
    m_snapToGridAction->setIcon(createRecolorableIcon(":/icons/icons/grid.svg", iconColor));
    m_resizeAction->setIcon(createRecolorableIcon(":/icons/icons/resize.svg", iconColor));
    m_zoomInAction->setIcon(createRecolorableIcon(":/icons/icons/zoom-in.svg", iconColor));
    m_zoomOutAction->setIcon(createRecolorableIcon(":/icons/icons/zoom-out.svg", iconColor));
    m_settingsAction->setIcon(createRecolorableIcon(":/icons/icons/settings.svg", iconColor));
}
