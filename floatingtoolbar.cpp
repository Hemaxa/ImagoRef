#include "floatingtoolbar.h"
#include <QVBoxLayout>
#include <QToolButton>
#include <QAction>
#include <QStyle>

FloatingToolBar::FloatingToolBar(QWidget *parent) : QFrame(parent) {
    //внешний вид панели через таблицы стилей CSS
    setStyleSheet(R"(
        QFrame {
            background-color: rgba(35, 35, 35, 230); /* Полупрозрачный темный фон */
            border: 1px solid rgb(70, 70, 70);
            border-radius: 12px; /* Скругление углов */
        }
        QToolButton {
            background-color: transparent;
            border: none;
            padding: 8px;
            margin: 2px;
            border-radius: 6px;
        }
        QToolButton:hover {
            background-color: rgb(80, 80, 80);
        }
        QToolButton:pressed {
            background-color: rgb(90, 90, 90);
        }
    )");

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
    button->setIconSize(QSize(24, 24)); //размер иконок

    //кнопка перед "распоркой"
    m_layout->insertWidget(m_layout->count() - 1, button);
}
