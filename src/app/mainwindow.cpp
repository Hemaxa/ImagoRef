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
#include <QUndoStack> //класс стека для Undo & Redo

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
    //создание стека
    m_undoStack = new QUndoStack(this); //объект QUndoStack создается как дочерний для MainWindow

    //создание холста
    m_canvasView = new CanvasView(m_undoStack, this); //объект CanvasView создается как дочерний для MainWindow
    //это заничт, что при уничтожении MainWindow, уничтожится и CanvasView

    //холст устанавливается как центральный виджет окна, он займет все доступное пространство
    setCentralWidget(m_canvasView);

    //создание действий и панели инструментов
    createActions();
    createToolBar();

    //настройка окна
    setWindowTitle("ImagoRef");
    resize(1280, 720);
    applyTheme("imago");
}

MainWindow::~MainWindow() {
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
        animation->setEasingCurve(QEasingCurve::InOutCubic); //создание кривой сглаживания

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
    m_resizeAction->setShortcut(tr("Ctrl+E"));
    connect(m_resizeAction, &QAction::triggered, m_canvasView, &CanvasView::enterResizeMode);

    //действие "Приблизить"
    m_zoomInAction = new QAction(createRecolorableIcon(":/icons/icons/zoom-in.svg", defaultIconColor), "Приблизить", this);
    m_zoomInAction->setShortcuts({QKeySequence::ZoomIn, QKeySequence(tr("Ctrl+=")), QKeySequence(tr("Ctrl++"))});
    connect(m_zoomInAction, &QAction::triggered, m_canvasView, &CanvasView::zoomIn);

    //действие "Отдалить"
    m_zoomOutAction = new QAction(createRecolorableIcon(":/icons/icons/zoom-out.svg", defaultIconColor), "Отдалить", this);
    m_zoomOutAction->setShortcuts({QKeySequence::ZoomOut, QKeySequence(tr("Ctrl--"))});
    connect(m_zoomOutAction, &QAction::triggered, m_canvasView, &CanvasView::zoomOut);

    //действие "Вращать против часовой"
    m_rotateLeftAction = new QAction(createRecolorableIcon(":/icons/icons/rotate-left.svg", defaultIconColor), "Вращать против часовой", this);
    m_rotateLeftAction->setShortcut(tr("Ctrl+Shift+R"));
    connect(m_rotateLeftAction, &QAction::triggered, m_canvasView, &CanvasView::rotateSelectedLeft);

    //действие "Вращать по часовой"
    m_rotateRightAction = new QAction(createRecolorableIcon(":/icons/icons/rotate-right.svg", defaultIconColor), "Вращать по часовой", this);
    m_rotateRightAction->setShortcut(tr("Ctrl+R"));
    connect(m_rotateRightAction, &QAction::triggered, m_canvasView, &CanvasView::rotateSelectedRight);

    //дейсвтие "Отменить"
    m_undoAction = new QAction(createRecolorableIcon(":/icons/icons/undo.svg", defaultIconColor), "Отменить", this);
    m_undoAction->setShortcut(QKeySequence::Undo);
    connect(m_undoAction, &QAction::triggered, m_undoStack, &QUndoStack::undo);
    connect(m_undoStack, &QUndoStack::canUndoChanged, m_undoAction, &QAction::setEnabled);

    //дейсвтие "Повторить"
    m_redoAction = new QAction(createRecolorableIcon(":/icons/icons/redo.svg", defaultIconColor), "Повторить", this);
    m_redoAction->setShortcut(QKeySequence::Redo);
    connect(m_redoAction, &QAction::triggered, m_undoStack, &QUndoStack::redo);
    connect(m_undoStack, &QUndoStack::canRedoChanged, m_redoAction, &QAction::setEnabled);

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
    //группа "Буфер обмена и удаление"
    m_toolBar->addAction(m_pasteAction);
    m_toolBar->addAction(m_deleteAction);
    m_toolBar->addSeparator();

    //группа "Трансформации"
    m_toolBar->addAction(m_snapToGridAction);
    m_toolBar->addAction(m_resizeAction);
    m_toolBar->addAction(m_rotateLeftAction);
    m_toolBar->addAction(m_rotateRightAction);
    m_toolBar->addSeparator();

    //группа "Масштаб"
    m_toolBar->addAction(m_zoomInAction);
    m_toolBar->addAction(m_zoomOutAction);
    m_toolBar->addSeparator();

    //группа "История"
    m_toolBar->addAction(m_undoAction);
    m_toolBar->addAction(m_redoAction);
    m_toolBar->addSeparator();

    //группа "Настройки"
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

//метод применения темы приложения
void MainWindow::applyTheme(const QString &themeName) {
    m_currentThemeName = themeName; //сохранения имени новой темы
    QString path = QString(":/themes/themes/%1.qss").arg(themeName);

    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file); //объект QTextStream для чтения содержимого файла
        QString styleSheet = stream.readAll(); //чтение всего содержимого в строку styleSheet
        setStyleSheet(styleSheet);
        file.close();
    } else {
        setStyleSheet("");
    }

    QColor iconColor;
    QColor gridColor;

    if (themeName == "imago") {
        iconColor = QColor("#e67e22");
        gridColor = QColor("#1f1f1f");
    } else if (themeName == "dark") {
        iconColor = QColor("#e0e0e0");
        gridColor = QColor("#1f1f1f");
    } else if (themeName == "light") {
        iconColor = QColor("#2a2a2a");
        gridColor = QColor("#d0d0d0");
    } else if (themeName == "purple") {
        iconColor = QColor("#6b5b95");
        gridColor = QColor("#d6cadd");
    } else if (themeName == "orange") {
        iconColor = QColor("#8c5a2d");
        gridColor = QColor("#ffe8cc");
    } else if (themeName == "blue") {
        iconColor = QColor("#3c6c8e");
        gridColor = QColor("#d4eaf7");
    } else if (themeName == "pink") {
        iconColor = QColor("#96536c");
        gridColor = QColor("#ffe6eb");
    } else if (themeName == "aquamarine") {
        iconColor = QColor("#3d8a7e");
        gridColor = QColor("#d0f7ee");
    } else if (themeName == "green") {
        iconColor = QColor("#556b2f");
        gridColor = QColor("#e0eee0");
    } else {
        //цвет по умолчанию, если тема не найдена
        iconColor = QColor("#2a2a2a");
        gridColor = QColor("#d0d0d0");
    }

    m_canvasView->setGridColor(gridColor);

    m_deleteAction->setIcon(createRecolorableIcon(":/icons/icons/delete.svg", iconColor));
    m_pasteAction->setIcon(createRecolorableIcon(":/icons/icons/paste.svg", iconColor));
    m_snapToGridAction->setIcon(createRecolorableIcon(":/icons/icons/grid.svg", iconColor));
    m_resizeAction->setIcon(createRecolorableIcon(":/icons/icons/resize.svg", iconColor));
    m_zoomInAction->setIcon(createRecolorableIcon(":/icons/icons/zoom-in.svg", iconColor));
    m_zoomOutAction->setIcon(createRecolorableIcon(":/icons/icons/zoom-out.svg", iconColor));
    m_rotateLeftAction->setIcon(createRecolorableIcon(":/icons/icons/rotate-left.svg", iconColor));
    m_rotateRightAction->setIcon(createRecolorableIcon(":/icons/icons/rotate-right.svg", iconColor));
    m_undoAction->setIcon(createRecolorableIcon(":/icons/icons/undo.svg", iconColor));
    m_redoAction->setIcon(createRecolorableIcon(":/icons/icons/redo.svg", iconColor));
    m_settingsAction->setIcon(createRecolorableIcon(":/icons/icons/settings.svg", iconColor));
}
