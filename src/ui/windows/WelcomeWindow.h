//WelcomeWindow - класс, который отвечает за стартовое окно приложения

#pragma once

#include <QDialog> //класс Qt диалогового окна приложения
#include <QPixmap> //класс Qt для работы с изображениями

class WelcomeWindow : public QDialog
{
    Q_OBJECT

public:
    //конструктор и деструктор
    explicit WelcomeWindow(QWidget *parent = nullptr);
    ~WelcomeWindow();

signals:
    //сигнал создания новой доски
    void newBoardRequested();

    //сигнал открытия существующей доски
    void openBoardRequested();

protected:
    //метод для отрисовки фона
    void paintEvent(QPaintEvent *event) override;

private:
    //метод создания интерфейса стартового окна
    void createInterface();

    //картинка для паттерна фона
    QPixmap m_patternPixmap;
};
