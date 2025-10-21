#include "WelcomeWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QFrame>
#include <QSpacerItem>

WelcomeWindow::WelcomeWindow(QWidget *parent) : QDialog(parent)
{
    //загрузка картинки паттерна фона в originalPattern
    QPixmap originalPattern(":/graphics/graphics/pattern.png");

    if (!originalPattern.isNull()) {

        //размер плитки паттерна
        QSize newTileSize(400, 400);

        //мастабирование плитки паттерна
        m_patternPixmap = originalPattern.scaled(
            newTileSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );

    } else {
        //загрузка не удалась
        m_patternPixmap = QPixmap();
    }

    //создание интерфейса
    createInterface();
}

WelcomeWindow::~WelcomeWindow() {}

void WelcomeWindow::paintEvent(QPaintEvent *event)
{
    //отрисовка фона с паттерном
    if (!m_patternPixmap.isNull()) {
        QPainter painter(this);
        //окно заполняется полностью повторением QPixmap
        painter.drawTiledPixmap(rect(), m_patternPixmap);
    }

    //вызывается базовый paintEvent, чтобы виджеты-потомки отрисовались поверх фона
    QDialog::paintEvent(event);
}

void WelcomeWindow::createInterface()
{
    //заголовок и размер окна
    setWindowTitle("ImagoRef - Welcome!");
    setMinimumSize(900, 600);

    //название для стартового окна
    this->setObjectName("welcomeDialog");

    //главный макет окна
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 50, 50, 50);
    mainLayout->setSpacing(30);

    //1) логотип
    QLabel *logoLabel = new QLabel(this);
    logoLabel->setObjectName("welcomeLogoLabel");
    QPixmap logoPixmap(":/graphics/graphics/text-logo.png");

    //изменение размера логотипа
    if (!logoPixmap.isNull())
    {
        logoLabel->setPixmap(logoPixmap.scaledToHeight(160, Qt::SmoothTransformation));
    }
    //запасной вариант, если логотип не загрузился
    else
    {
        logoLabel->setText("ImagoRef");
        logoLabel->setObjectName("welcomeTitleLabel");
    }
    logoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(logoLabel);

    mainLayout->addSpacing(30); //отступ

    //2) область "Recent Projects"
    QLabel *recentLabel = new QLabel("Recent projects", this);
    recentLabel->setObjectName("recentProjectsLabel");
    mainLayout->addWidget(recentLabel);

    //горизонтальный макет для заглушек
    QHBoxLayout *recentProjectsLayout = new QHBoxLayout;
    recentProjectsLayout->setSpacing(15);

    for (int i = 0; i < 5; ++i) {
        QFrame *placeholder = new QFrame(this);
        placeholder->setFixedSize(150, 100);
        placeholder->setObjectName("projectPlaceholder");
        placeholder->setStyleSheet(
            "QFrame#projectPlaceholder {"
            "  background-color: rgba(0, 0, 0, 0.2);"
            "  border: 1px solid rgba(0, 0, 0, 0.3);"
            "  border-radius: 6px;"
            "}"
            );
        recentProjectsLayout->addWidget(placeholder);
    }
    recentProjectsLayout->addStretch(); //прижимает заглушки влево
    mainLayout->addLayout(recentProjectsLayout);

    //распорка для заполнения всего оставшегося места
    mainLayout->addStretch();

    //3) кнопки
    //горизонтальный макет для кнопок
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    //создание кнопок
    QPushButton *newButton = new QPushButton("Создать", this);
    newButton->setObjectName("welcomeButton");
    newButton->setMinimumHeight(50);

    QPushButton *openButton = new QPushButton("Открыть", this);
    openButton->setObjectName("welcomeButton");
    openButton->setMinimumHeight(50);

    //связывание сигналов от кнопок
    connect(newButton, &QPushButton::clicked, this, &WelcomeWindow::newBoardRequested);
    connect(openButton, &QPushButton::clicked, this, &WelcomeWindow::openBoardRequested);

    buttonLayout->addStretch(); //распорка прижимает кнопки вправо
    buttonLayout->addWidget(newButton);
    buttonLayout->addWidget(openButton);

    mainLayout->addLayout(buttonLayout);
}
