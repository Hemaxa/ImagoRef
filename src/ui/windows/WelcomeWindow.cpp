#include "WelcomeWindow.h" // ✅

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

WelcomeWindow::WelcomeWindow(QWidget *parent) : QDialog(parent)
{
    setupUi();
}

WelcomeWindow::~WelcomeWindow() {}

void WelcomeWindow::setupUi()
{
    setWindowTitle("ImagoRef - Welcome!");
    setMinimumSize(800, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 50, 50, 50);
    mainLayout->setSpacing(30);

    QLabel *titleLabel = new QLabel("ImagoRef", this);
    titleLabel->setObjectName("welcomeTitle");
    titleLabel->setAlignment(Qt::AlignCenter);

    QPushButton *newButton = new QPushButton("Создать новую доску", this);
    newButton->setObjectName("welcomeButton");
    newButton->setMinimumHeight(50);

    QPushButton *openButton = new QPushButton("Открыть из файла...", this);
    openButton->setObjectName("welcomeButton");
    openButton->setMinimumHeight(50);

    connect(newButton, &QPushButton::clicked, this, &WelcomeWindow::newBoardRequested);
    connect(openButton, &QPushButton::clicked, this, &WelcomeWindow::openBoardRequested);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(newButton);
    buttonLayout->addWidget(openButton);
    buttonLayout->addStretch();

    mainLayout->addWidget(titleLabel);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();
}
