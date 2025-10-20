//WelcomeWindow - класс, который отвечает за стартовое окно приложение

#pragma once

#include <QDialog>

class WelcomeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit WelcomeWindow(QWidget *parent = nullptr);
    ~WelcomeWindow();

signals:
    void newBoardRequested();
    void openBoardRequested();

private:
    void setupUi();
};
