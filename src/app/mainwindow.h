#pragma once //защита от двойного включения, вместо кучи #define
#include <QMainWindow> //класс основного окна приложения в Qt

//наследуемые классы
//используется такая структура, потому что необходим только указатель на обект этих классов
class CanvasView; //класс доски с изображениями
class FloatingToolBar; //класс всплывающего окна инструментов
class QAction; //класс Qt для обработки действий
class QKeyEvent; //класс Qt для обработки событий с клавиатуры

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr); //explicit используется для предотвращения неявного преобразования типов при вызове конструктора

protected:
    //метод перхватывания нажатия клавиш для глобальных шорткатов
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onAnimationFinished(); //метод завершения анимации ухода боковой панели
    void openSettingsDialog(); //метод открытия окна настроек приложение

private:
    void createActions(); //создание действий (кнопки, меню)
    void createToolBar(); //создание панели инструментов
    void applyTheme(const QString &themeName); //изменение темы приложения

    //указатели на основные компоненты из унаследованных классов
    CanvasView *m_canvasView;
    FloatingToolBar *m_toolBar;
    QString m_currentThemeName;

    //действия, которые можно установить на кнопки и горячие клавиши
    QAction *m_deleteAction;
    QAction *m_pasteAction;
    QAction *m_snapToGridAction;
    QAction *m_resizeAction;
    QAction *m_zoomInAction;
    QAction *m_zoomOutAction;
    QAction *m_settingsAction;
};
