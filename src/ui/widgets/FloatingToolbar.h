#pragma once

#include <QFrame> //для создания собственного виджета
#include <QList>

class QVBoxLayout;
class QAction;
class AnimationManager;

class FloatingToolBar : public QFrame {
    Q_OBJECT

public:
    explicit FloatingToolBar(QWidget *parent = nullptr);
    ~FloatingToolBar();

    void addAction(QAction *action, const QString& iconPath);
    void addSeparator();

public slots:
    void updateIconColors(const QColor& color);

private:
    QVBoxLayout *m_layout;
    QList<AnimationManager*> m_buttons;
};
