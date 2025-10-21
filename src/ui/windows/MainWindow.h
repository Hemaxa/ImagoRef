#pragma once

#include <QMainWindow>

class CanvasView;
class FloatingToolBar;
class QAction;
class QKeyEvent;
class QUndoStack;
class SettingsManager;
class ThemeManager;

class BaseTool;
class DeleteTool;
class PasteTool;
class GridSnapTool;
class ResizeTool;
class RotateTool;
class ZoomTool;


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool openBoard();
    void applyInitialSettings();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onAnimationFinished();
    void openSettingsDialog();
    void saveBoard();
    void saveBoardAs();

private:
    void createActions();
    void createToolBar();
    void createTools();

    void loadBoardFromFile(const QString &filePath);
    void saveBoardToFile(const QString &filePath);
    void updateWindowTitle();

    CanvasView *m_canvasView;
    FloatingToolBar *m_toolBar;
    QUndoStack *m_undoStack;
    QString m_currentFilePath;

    DeleteTool* m_deleteTool;
    PasteTool* m_pasteTool;
    GridSnapTool* m_snapToGridTool;
    ResizeTool* m_resizeTool;
    RotateTool* m_rotateTool;
    ZoomTool* m_zoomTool;

    QAction *m_openAction;
    QAction *m_saveAction;
    QAction *m_saveAsAction;
    QAction *m_deleteAction;
    QAction *m_pasteAction;
    QAction *m_snapToGridAction;
    QAction *m_resizeAction;
    QAction *m_zoomInAction;
    QAction *m_zoomOutAction;
    QAction *m_rotateLeftAction;
    QAction *m_rotateRightAction;
    QAction *m_undoAction;
    QAction *m_redoAction;
    QAction *m_settingsAction;
};
