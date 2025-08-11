#include "floatingtoolbar.h"
#include <QVBoxLayout>
#include <QToolButton>
#include <QAction>

FloatingToolBar::FloatingToolBar(QWidget *parent) : QFrame(parent) {
    //вертикальный layout для расположения кнопок
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(8, 8, 8, 8); //отступы внутри панели
    m_layout->setSpacing(5); //расстояние между кнопками
    m_layout->addStretch(); //добавляется "распорка", чтобы кнопки прижимались к верху
}

void FloatingToolBar::addAction(QAction *action) {
    if (!action) return;

    //создается кнопка, связанная с действием (QAction)
    QToolButton *button = new QToolButton(this);
    button->setDefaultAction(action);
    button->setIconSize(QSize(22, 22)); //размер иконок

    //кнопка перед "распоркой"
    m_layout->insertWidget(m_layout->count() - 1, button);
}

void FloatingToolBar::addSeparator() {
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);   //горизонтальная линия для разделителя
    line->setObjectName("toolBarSeparator");
    m_layout->insertWidget(m_layout->count() - 1, line);
}
