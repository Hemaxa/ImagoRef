#include "MainWindow.h"
#include "CanvasView.h"
#include "FloatingToolbar.h"
#include "SettingsWindow.h"
#include "ImageItem.h"
#include "SettingsManager.h"
#include "ThemeManager.h"

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

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, m_toolBar, &FloatingToolBar::updateIconColors);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](const QColor& iconColor, const QColor& gridColor) { m_canvasView->setGridColor(gridColor); });

    updateWindowTitle();
    resize(1280, 800);
}

MainWindow::~MainWindow() {
    SettingsManager::instance().saveSettings();
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

void MainWindow::applyInitialSettings() {
    // Применяем настройки, загруженные в Main.cpp
    int gridSize = SettingsManager::instance().getGridSize();
    m_canvasView->setGridSize(gridSize);

    // Тема уже была применена в Main.cpp, но мы вызываем
    // applyTheme() еще раз, чтобы *отправить сигналы*
    // и обновить цвета иконок/сетки.
    ThemeManager::instance().applyTheme(SettingsManager::instance().getThemeName());
}

void MainWindow::createActions() {
    QColor defaultIconColor("#e0e0e0");

    m_openAction = new QAction("Открыть...", this);
    m_openAction->setShortcut(QKeySequence::Open);
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openBoard);

    m_saveAction = new QAction("Сохранить", this);
    m_saveAction->setShortcut(QKeySequence::Save);
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveBoard);

    m_saveAsAction = new QAction("Сохранить как...", this);
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::saveBoardAs);

    m_deleteAction = new QAction("Удалить", this);
    m_deleteAction->setShortcut(QKeySequence::Delete);
    connect(m_deleteAction, &QAction::triggered, m_deleteTool, &DeleteTool::execute);

    m_pasteAction = new QAction("Вставить", this);
    m_pasteAction->setShortcut(QKeySequence::Paste);
    connect(m_pasteAction, &QAction::triggered, m_pasteTool, &PasteTool::execute);

    m_snapToGridAction = new QAction("Привязать к сетке", this);
    m_snapToGridAction->setShortcut(tr("Ctrl+G"));
    connect(m_snapToGridAction, &QAction::triggered, m_snapToGridTool, &GridSnapTool::execute);

    m_resizeAction = new QAction("Изменить размер", this);
    m_resizeAction->setShortcut(tr("Ctrl+E"));
    connect(m_resizeAction, &QAction::triggered, m_resizeTool, &ResizeTool::execute);

    m_zoomInAction = new QAction("Приблизить", this);
    m_zoomInAction->setShortcuts({QKeySequence::ZoomIn, QKeySequence(tr("=")), QKeySequence(tr("Ctrl+=")), QKeySequence(tr("Ctrl++"))});
    connect(m_zoomInAction, &QAction::triggered, m_zoomTool, &ZoomTool::zoomIn);

    m_zoomOutAction = new QAction("Отдалить", this);
    m_zoomOutAction->setShortcuts({QKeySequence::ZoomOut, QKeySequence(tr("Ctrl--"))});
    connect(m_zoomOutAction, &QAction::triggered, m_zoomTool, &ZoomTool::zoomOut);

    m_rotateLeftAction = new QAction("Вращать против часовой", this);
    m_rotateLeftAction->setShortcut(tr("Ctrl+Shift+R"));
    connect(m_rotateLeftAction, &QAction::triggered, m_rotateTool, &RotateTool::rotateLeft);

    m_rotateRightAction = new QAction("Вращать по часовой", this);
    m_rotateRightAction->setShortcut(tr("Ctrl+R"));
    connect(m_rotateRightAction, &QAction::triggered, m_rotateTool, &RotateTool::rotateRight);

    m_undoAction = new QAction("Отменить", this);
    m_undoAction->setShortcut(QKeySequence::Undo);
    connect(m_undoAction, &QAction::triggered, m_undoStack, &QUndoStack::undo);
    connect(m_undoStack, &QUndoStack::canUndoChanged, m_undoAction, &QAction::setEnabled);

    m_redoAction = new QAction("Повторить", this);
    m_redoAction->setShortcut(QKeySequence::Redo);
    connect(m_redoAction, &QAction::triggered, m_undoStack, &QUndoStack::redo);
    connect(m_undoStack, &QUndoStack::canRedoChanged, m_redoAction, &QAction::setEnabled);

    m_settingsAction = new QAction("Настройки", this);
    m_settingsAction->setShortcut(tr("Ctrl+,"));
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::openSettingsDialog);
}

void MainWindow::createToolBar() {
    m_toolBar = new FloatingToolBar(m_canvasView);

    m_toolBar->addAction(m_openAction, ":/icons/icons/paste.svg");
    m_toolBar->addAction(m_saveAction, ":/icons/icons/paste.svg");
    m_toolBar->addAction(m_saveAsAction, ":/icons/icons/paste.svg");
    m_toolBar->addSeparator();

    m_toolBar->addAction(m_pasteAction, ":/icons/icons/paste.svg");
    m_toolBar->addAction(m_deleteAction, ":/icons/icons/delete.svg");
    m_toolBar->addSeparator();

    m_toolBar->addAction(m_snapToGridAction, ":/icons/icons/grid.svg");
    m_toolBar->addAction(m_resizeAction, ":/icons/icons/resize.svg");
    m_toolBar->addAction(m_rotateLeftAction, ":/icons/icons/rotate-left.svg");
    m_toolBar->addAction(m_rotateRightAction, ":/icons/icons/rotate-right.svg");
    m_toolBar->addSeparator();

    m_toolBar->addAction(m_zoomInAction, ":/icons/icons/zoom-in.svg");
    m_toolBar->addAction(m_zoomOutAction, ":/icons/icons/zoom-out.svg");
    m_toolBar->addSeparator();

    m_toolBar->addAction(m_undoAction, ":/icons/icons/undo.svg");
    m_toolBar->addAction(m_redoAction, ":/icons/icons/redo.svg");
    m_toolBar->addSeparator();

    m_toolBar->addAction(m_settingsAction, ":/icons/icons/settings.svg");
    m_toolBar->move(15, 15);
}

void MainWindow::openSettingsDialog() {
    SettingsDialog dialog(
        SettingsManager::instance().getGridSize(),
        SettingsManager::instance().getThemeName(),
        this
    );
    if (dialog.exec() == QDialog::Accepted) {m_canvasView->setGridSize(dialog.gridSize());
        SettingsManager::instance().setGridSize(dialog.gridSize());
        SettingsManager::instance().setThemeName(dialog.theme());

        m_canvasView->setGridSize(dialog.gridSize());
        ThemeManager::instance().applyTheme(dialog.theme()); // Это обновит цвета
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
