#pragma once
#include <QFrame> //для создания собственного виджета

class QVBoxLayout;
class QAction;

class FloatingToolBar : public QFrame {
    Q_OBJECT

public:
    explicit FloatingToolBar(QWidget *parent = nullptr);
    void addAction(QAction *action);
    void addSeparator();

private:
    QVBoxLayout *m_layout;
};
