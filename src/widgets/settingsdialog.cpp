#include "settingsdialog.h"

//части интерфейса
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

#include <QString>

SettingsDialog::SettingsDialog(int currentGridSize, const QString &currentTheme, QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Настройки");
    setFixedWidth(550);
    setMinimumHeight(300);

    //основной вертикальный макет для всего диалога
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    //горизонтальный макет для списка и содержимого
    QHBoxLayout *contentLayout = new QHBoxLayout();

    //список слева
    QListWidget *navigationList = new QListWidget(this);
    navigationList->setObjectName("settingsNavigationList");
    navigationList->setFixedWidth(160); //фиксированная ширина списка
    navigationList->addItem("Общие");
    navigationList->addItem("Горячие клавиши");

    //создание "стопки" виджетов для содержимого вкладок
    QStackedWidget *pagesWidget = new QStackedWidget(this);
    pagesWidget->setObjectName("settingsPages");

    //--- вкладка "Общие" ---
    //параметр шага сетки
    QWidget *generalPage = new QWidget;
    QHBoxLayout* pageLayout = new QHBoxLayout(generalPage);
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignLeft);
    m_gridSizeSpinBox = new QSpinBox;
    m_gridSizeSpinBox->setRange(20, 200);
    m_gridSizeSpinBox->setValue(currentGridSize);
    m_gridSizeSpinBox->setSuffix(" px");
    formLayout->addRow("Шаг сетки:", m_gridSizeSpinBox);

    //параметр темы
    m_themeComboBox = new QComboBox;
    m_themeComboBox->addItem("Темная", "dark");
    m_themeComboBox->addItem("Светлая", "light");
    m_themeComboBox->addItem("Голубая", "blue");
    m_themeComboBox->addItem("Аквамариновая", "aquamarine");
    m_themeComboBox->addItem("Зеленая", "green");
    m_themeComboBox->addItem("Фиолетовая", "purple");
    m_themeComboBox->addItem("Розовая", "pink");
    m_themeComboBox->addItem("Оранжевая", "orange");
    int index = m_themeComboBox->findData(currentTheme);
    if (index != -1) { m_themeComboBox->setCurrentIndex(index); }
    formLayout->addRow("Тема интерфейса:", m_themeComboBox);

    pageLayout->addLayout(formLayout);
    pageLayout->addStretch();

    //--- вкладка "Горячие клавиши" ---
    QWidget *hotkeysPage = createHotkeysPage();

    //добавление страниц в стопку
    pagesWidget->addWidget(generalPage);
    pagesWidget->addWidget(hotkeysPage);

    //соединение выбора в списке с переключением страниц
    connect(navigationList, &QListWidget::currentRowChanged, pagesWidget, &QStackedWidget::setCurrentIndex);
    navigationList->setCurrentRow(0); //первый элемент выбран по умолчанию

    //сборка макета
    contentLayout->addWidget(navigationList);
    contentLayout->addWidget(pagesWidget);

    //кнопки "Отмена" и "Применить"
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Применить");
    buttonBox->button(QDialogButtonBox::Cancel)->setText("Отмена");

    mainLayout->addLayout(contentLayout);
    mainLayout->addWidget(buttonBox);
}

int SettingsDialog::gridSize() const {
    return m_gridSizeSpinBox->value();
}

QString SettingsDialog::theme() const {
    return m_themeComboBox->currentData().toString();
}

QWidget* SettingsDialog::createHotkeysPage() {
    QWidget *hotkeysPage = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(hotkeysPage);
    QTextEdit *hotkeysText = new QTextEdit;
    hotkeysText->setObjectName("hotkeysTextEdit");
    hotkeysText->setReadOnly(true);
    hotkeysText->setHtml(R"(
        <style>
            body {
                color: #e0e0e0;
                line-height: 1.6;
            }
            h3 {
                color: #ffffff;
                border-bottom: 1px solid #5a5a5a;
                padding-bottom: 5px;
                margin-bottom: 10px;
            }
            ul {
                list-style: none;
                padding-left: 0;
            }
            li {
                padding-bottom: 8px;
            }
            b {
                color: #ffffff;
                font-weight: bold;
            }
        </style>
        <h3>Основные действия</h3>
        <ul>
            <li><b>Перемещение холста:</b> Зажать среднюю кнопку мыши</li>
            <li><b>Масштабирование:</b> Ctrl/Cmd + колесико мыши ИЛИ Ctrl/Cmd + / -</li>
            <li><b>Удалить элемент:</b> Delete</li>
            <li><b>Привязать к сетке:</b> Ctrl+G</li>
            <li><b>Скрыть/показать панель:</b> Tab</li>
        </ul>
    )");
    layout->addWidget(hotkeysText);
    return hotkeysPage;
}
