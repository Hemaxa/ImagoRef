#include "MainWindow.h"
#include "CanvasView.h"
#include "FloatingToolbar.h"
#include "SettingsWindow.h"
#include "ImageItem.h"

#include <QAction>
#include <QKeyEvent>
#include <QIcon>
#include <QPropertyAnimation>
#include <QFile>
#include <QSvgRenderer>
#include <QPainter>
#include <QUndoStack>
#include <QTextStream>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QBuffer>

// ✅ ПОДКЛЮЧАЕМ РЕАЛИЗАЦИИ ИНСТРУМЕНТОВ
#include "BaseTool.h"
#include "DeleteTool.h"
#include "PasteTool.h"
#include "GridSnapTool.h"
#include "ResizeTool.h"
#include "RotateTool.h"
#include "ZoomTool.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    m_undoStack = new QUndoStack(this);
    m_canvasView = new CanvasView(m_undoStack, this);
    setCentralWidget(m_canvasView);

    createTools();
    createActions();
    createToolBar();

    updateWindowTitle();
    resize(1280, 800);
    applyTheme("imago");
}

MainWindow::~MainWindow() {
}

QIcon createRecolorableIcon(const QString& path, const QColor& color, const QSize& size = QSize(24, 24)) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QIcon();
    }
    QTextStream in(&file);
    QString svgData = in.readAll();
    svgData.replace("currentColor", color.name());
    QSvgRenderer renderer(svgData.toUtf8());
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    return QIcon(pixmap);
}

void MainWindow::onAnimationFinished() {
    m_toolBar->setProperty("animation", QVariant());
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Tab) {
        event->accept();
        auto* currentAnimation = m_toolBar->property("animation").value<QPropertyAnimation*>();
        if (currentAnimation) {
            currentAnimation->stop();
        }
        const int duration = 300;
        const int endXOnScreen = 15;
        auto* animation = new QPropertyAnimation(m_toolBar, "pos", this);
        animation->setDuration(duration);
        animation->setEasingCurve(QEasingCurve::InOutCubic);
        connect(animation, &QPropertyAnimation::finished, this, &MainWindow::onAnimationFinished);

        if (m_toolBar->isVisible()) {
            animation->setEndValue(QPoint(-m_toolBar->width(), m_toolBar->y()));
            connect(animation, &QPropertyAnimation::finished, m_toolBar, &QWidget::hide);
        }
        else {
            m_toolBar->move(-m_toolBar->width(), m_toolBar->y());
            m_toolBar->show();
            animation->setEndValue(QPoint(endXOnScreen, m_toolBar->y()));
        }
        m_toolBar->setProperty("animation", QVariant::fromValue(animation));
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::createTools()
{
    m_deleteTool = new DeleteTool(this);
    m_pasteTool = new PasteTool(this);
    m_snapToGridTool = new GridSnapTool(this);
    m_resizeTool = new ResizeTool(this);
    m_rotateTool = new RotateTool(this);
    m_zoomTool = new ZoomTool(this);

    QList<BaseTool*> tools = {
        m_deleteTool, m_pasteTool, m_snapToGridTool,
        m_resizeTool, m_rotateTool, m_zoomTool
    };

    for (BaseTool* tool : tools) {
        tool->setContext(m_canvasView, m_undoStack);
    }
}

void MainWindow::createActions() {
    QColor defaultIconColor("#e0e0e0");

    m_openAction = new QAction(createRecolorableIcon(":/icons/icons/paste.svg", defaultIconColor), "Открыть...", this);
    m_openAction->setShortcut(QKeySequence::Open);
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openBoard);

    m_saveAction = new QAction(createRecolorableIcon(":/icons/icons/paste.svg", defaultIconColor), "Сохранить", this);
    m_saveAction->setShortcut(QKeySequence::Save);
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveBoard);

    m_saveAsAction = new QAction(createRecolorableIcon(":/icons/icons/paste.svg", defaultIconColor), "Сохранить как...", this);
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::saveBoardAs);

    // --- ПОДКЛЮЧЕНИЕ К ИНСТРУМЕНТАМ ---
    m_deleteAction = new QAction(createRecolorableIcon(":/icons/icons/delete.svg", defaultIconColor), "Удалить", this);
    m_deleteAction->setShortcut(QKeySequence::Delete);
    connect(m_deleteAction, &QAction::triggered, m_deleteTool, &DeleteTool::execute);

    m_pasteAction = new QAction(createRecolorableIcon(":/icons/icons/paste.svg", defaultIconColor), "Вставить", this);
    m_pasteAction->setShortcut(QKeySequence::Paste);
    connect(m_pasteAction, &QAction::triggered, m_pasteTool, &PasteTool::execute);

    m_snapToGridAction = new QAction(createRecolorableIcon(":/icons/icons/grid.svg", defaultIconColor), "Привязать к сетке", this);
    m_snapToGridAction->setShortcut(tr("Ctrl+G"));
    connect(m_snapToGridAction, &QAction::triggered, m_snapToGridTool, &GridSnapTool::execute);

    m_resizeAction = new QAction(createRecolorableIcon(":/icons/icons/resize.svg", defaultIconColor), "Изменить размер", this);
    m_resizeAction->setShortcut(tr("Ctrl+E"));
    connect(m_resizeAction, &QAction::triggered, m_resizeTool, &ResizeTool::execute);

    m_zoomInAction = new QAction(createRecolorableIcon(":/icons/icons/zoom-in.svg", defaultIconColor), "Приблизить", this);
    m_zoomInAction->setShortcuts({QKeySequence::ZoomIn, QKeySequence(tr("Ctrl+=")), QKeySequence(tr("Ctrl++"))});
    connect(m_zoomInAction, &QAction::triggered, m_zoomTool, &ZoomTool::zoomIn);

    m_zoomOutAction = new QAction(createRecolorableIcon(":/icons/icons/zoom-out.svg", defaultIconColor), "Отдалить", this);
    m_zoomOutAction->setShortcuts({QKeySequence::ZoomOut, QKeySequence(tr("Ctrl--"))});
    connect(m_zoomOutAction, &QAction::triggered, m_zoomTool, &ZoomTool::zoomOut);

    m_rotateLeftAction = new QAction(createRecolorableIcon(":/icons/icons/rotate-left.svg", defaultIconColor), "Вращать против часовой", this);
    m_rotateLeftAction->setShortcut(tr("Ctrl+Shift+R"));
    connect(m_rotateLeftAction, &QAction::triggered, m_rotateTool, &RotateTool::rotateLeft);

    m_rotateRightAction = new QAction(createRecolorableIcon(":/icons/icons/rotate-right.svg", defaultIconColor), "Вращать по часовой", this);
    m_rotateRightAction->setShortcut(tr("Ctrl+R"));
    connect(m_rotateRightAction, &QAction::triggered, m_rotateTool, &RotateTool::rotateRight);

    // --- ПОДКЛЮЧЕНИЕ К QUndoStack ---
    m_undoAction = new QAction(createRecolorableIcon(":/icons/icons/undo.svg", defaultIconColor), "Отменить", this);
    m_undoAction->setShortcut(QKeySequence::Undo);
    connect(m_undoAction, &QAction::triggered, m_undoStack, &QUndoStack::undo);
    connect(m_undoStack, &QUndoStack::canUndoChanged, m_undoAction, &QAction::setEnabled);

    m_redoAction = new QAction(createRecolorableIcon(":/icons/icons/redo.svg", defaultIconColor), "Повторить", this);
    m_redoAction->setShortcut(QKeySequence::Redo);
    connect(m_redoAction, &QAction::triggered, m_undoStack, &QUndoStack::redo);
    connect(m_undoStack, &QUndoStack::canRedoChanged, m_redoAction, &QAction::setEnabled);

    m_settingsAction = new QAction(createRecolorableIcon(":/icons/icons/settings.svg", defaultIconColor), "Настройки", this);
    m_settingsAction->setShortcut(tr("Ctrl+,"));
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::openSettingsDialog);
}

void MainWindow::createToolBar() {
    m_toolBar = new FloatingToolBar(m_canvasView);

    m_toolBar->addAction(m_openAction);
    m_toolBar->addAction(m_saveAction);
    m_toolBar->addAction(m_saveAsAction);
    m_toolBar->addSeparator();

    m_toolBar->addAction(m_pasteAction);
    m_toolBar->addAction(m_deleteAction);
    m_toolBar->addSeparator();

    m_toolBar->addAction(m_snapToGridAction);
    m_toolBar->addAction(m_resizeAction);
    m_toolBar->addAction(m_rotateLeftAction);
    m_toolBar->addAction(m_rotateRightAction);
    m_toolBar->addSeparator();

    m_toolBar->addAction(m_zoomInAction);
    m_toolBar->addAction(m_zoomOutAction);
    m_toolBar->addSeparator();

    m_toolBar->addAction(m_undoAction);
    m_toolBar->addAction(m_redoAction);
    m_toolBar->addSeparator();

    m_toolBar->addAction(m_settingsAction);
    m_toolBar->move(15, 15);
}

void MainWindow::openSettingsDialog() {
    SettingsDialog dialog(m_canvasView->getGridSize(), m_currentThemeName, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_canvasView->setGridSize(dialog.gridSize());
        applyTheme(dialog.theme());
    }
}

bool MainWindow::openBoard()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Открыть доску", "", "ImagoRef доска (*.iref);;Все файлы (*)");
    if (filePath.isEmpty()) {
        return false;
    }
    loadBoardFromFile(filePath);
    return true;
}

void MainWindow::saveBoard()
{
    if (m_currentFilePath.isEmpty()) {
        saveBoardAs();
    } else {
        saveBoardToFile(m_currentFilePath);
    }
}

void MainWindow::saveBoardAs()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Сохранить доску", "", "ImagoRef доска (*.iref);;Все файлы (*)");
    if (!filePath.isEmpty()) {
        saveBoardToFile(filePath);
    }
}

void MainWindow::updateWindowTitle()
{
    QString title = "ImagoRef - ";
    if (m_currentFilePath.isEmpty()) {
        title += "Новая доска";
    } else {
        title += QFileInfo(m_currentFilePath).fileName();
    }
    setWindowTitle(title);
}

void MainWindow::saveBoardToFile(const QString &filePath)
{
    QJsonObject rootObj;
    rootObj["version"] = "1.0";
    QJsonObject canvasObj;
    canvasObj["gridSize"] = m_canvasView->getGridSize();
    canvasObj["gridColor"] = m_canvasView->palette().window().color().name();
    rootObj["canvas"] = canvasObj;

    QJsonArray itemsArray;
    for (QGraphicsItem *item : m_canvasView->scene()->items()) {
        if (ImageItem *imageItem = dynamic_cast<ImageItem*>(item)) {
            QJsonObject itemObj;
            itemObj["pos_x"] = imageItem->pos().x();
            itemObj["pos_y"] = imageItem->pos().y();
            itemObj["width"] = imageItem->boundingRect().width();
            itemObj["height"] = imageItem->boundingRect().height();
            itemObj["rotation"] = imageItem->rotation();
            itemObj["zValue"] = imageItem->zValue();

            QByteArray byteArray;
            QBuffer buffer(&byteArray);
            buffer.open(QIODevice::WriteOnly);
            imageItem->m_originalPixmap.save(&buffer, "PNG");
            itemObj["imageData"] = QString::fromLatin1(byteArray.toBase64());

            itemsArray.append(itemObj);
        }
    }
    rootObj["items"] = itemsArray;

    QJsonDocument doc(rootObj);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        m_currentFilePath = filePath;
        updateWindowTitle();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл.");
    }
}

void MainWindow::loadBoardFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл.");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    m_canvasView->scene()->clear();
    m_undoStack->clear();

    QJsonObject rootObj = doc.object();

    if (rootObj.contains("canvas")) {
        QJsonObject canvasObj = rootObj["canvas"].toObject();
        m_canvasView->setGridSize(canvasObj["gridSize"].toInt(25));
    }

    if (rootObj.contains("items")) {
        QJsonArray itemsArray = rootObj["items"].toArray();
        for (const QJsonValue &value : itemsArray) {
            QJsonObject itemObj = value.toObject();

            QByteArray byteArray = QByteArray::fromBase64(itemObj["imageData"].toString().toLatin1());
            QPixmap pixmap;
            if (pixmap.loadFromData(byteArray, "PNG")) {
                ImageItem *imageItem = new ImageItem(pixmap, m_undoStack);
                qreal width = itemObj["width"].toDouble();
                qreal height = itemObj["height"].toDouble();
                QPointF pos(itemObj["pos_x"].toDouble(), itemObj["pos_y"].toDouble());
                imageItem->setGeometry(QRectF(0, 0, width, height), pos);
                imageItem->setRotation(itemObj["rotation"].toDouble());
                imageItem->setZValue(itemObj["zValue"].toDouble());
                m_canvasView->scene()->addItem(imageItem);
            }
        }
    }

    m_currentFilePath = filePath;
    updateWindowTitle();
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
