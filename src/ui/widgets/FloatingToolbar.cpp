#include "FloatingToolbar.h" // ✅
#include <QVBoxLayout>
#include <QToolButton>
#include <QAction>

FloatingToolBar::FloatingToolBar(QWidget *parent) : QFrame(parent) {
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(8, 8, 8, 8);
    m_layout->setSpacing(5);
    m_layout->addStretch();
}

FloatingToolBar::~FloatingToolBar()
{
}

void FloatingToolBar::addAction(QAction *action) {
    if (!action) return;
    QToolButton *button = new QToolButton(this);
    button->setDefaultAction(action);
    button->setIconSize(QSize(22, 22));
    m_layout->insertWidget(m_layout->count() - 1, button);
}

void FloatingToolBar::addSeparator() {
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setObjectName("toolBarSeparator");
    m_layout->insertWidget(m_layout->count() - 1, line);
}
