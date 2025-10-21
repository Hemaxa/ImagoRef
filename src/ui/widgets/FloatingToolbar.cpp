#include "FloatingToolbar.h"
#include "AnimationManager.h"

#include <QVBoxLayout>
#include <QToolButton>
#include <QAction>

FloatingToolBar::FloatingToolBar(QWidget *parent) : QFrame(parent) {
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(8, 8, 8, 8);
    m_layout->setSpacing(5);
    m_layout->addStretch();
}

FloatingToolBar::~FloatingToolBar() {}

void FloatingToolBar::addAction(QAction *action, const QString& iconPath) {
    if (!action) return;
    // Создаем нашу анимированную кнопку
    AnimationManager *button = new AnimationManager(iconPath, action->toolTip(), action->shortcut(), this);

    // Связываем клик кнопки с QAction
    connect(button, &AnimationManager::clicked, action, &QAction::triggered);

    // Связываем состояние enabled QAction с кнопкой
    connect(action, &QAction::enabledChanged, button, &AnimationManager::setEnabled);
    button->setEnabled(action->isEnabled());

    m_buttons.append(button); // Добавляем в список для обновления
    m_layout->insertWidget(m_layout->count() - 1, button);
}

void FloatingToolBar::addSeparator() {
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setObjectName("toolBarSeparator");
    m_layout->insertWidget(m_layout->count() - 1, line);
}

void FloatingToolBar::updateIconColors(const QColor& color) {
    for (AnimationManager* button : m_buttons) {
        button->updateIconColor(color);
    }
}
