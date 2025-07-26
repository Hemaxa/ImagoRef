#pragma once //защита от двойного включения, вместо кучи #define
#include <QMainWindow> //класс основного окна приложения в Qt

//наследуемые классы
class CanvasView;
class QToolBar;
class QAction;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    //метод перхватывания нажатия клавиш для глобальных шорткатов
    void keyPressEvent(QKeyEvent *event) override;

private:
    void createActions(); //создание действий (кнопки, меню)
    void createToolBar(); //создание панели инструментов

    //указатели на основные компоненты из унаследованных классов
    CanvasView *m_canvasView;
    QToolBar *m_mainToolBar;

    //действия, которые можно будет повесить на кнопки и горячие клавиши
    QAction *m_deleteAction;
    QAction *m_snapToGridAction;
};
