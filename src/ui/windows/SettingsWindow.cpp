#include "SettingsWindow.h" // ✅

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
#include <QScrollArea>
#include <QString>

SettingsDialog::SettingsDialog(int currentGridSize, const QString &currentTheme, QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Настройки");
    setFixedWidth(750);
    setFixedHeight(450);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *contentLayout = new QHBoxLayout();

    QListWidget *navigationList = new QListWidget(this);
    navigationList->setObjectName("settingsNavigationList");
    navigationList->setFixedWidth(180);
    navigationList->addItem("Общие");
    navigationList->addItem("Горячие клавиши");

    QStackedWidget *pagesWidget = new QStackedWidget(this);
    pagesWidget->setObjectName("settingsPages");

    //--- вкладка "Общие" ---
    QWidget *generalPage = new QWidget;
    QHBoxLayout* pageLayout = new QHBoxLayout(generalPage);
    QFormLayout *formLayout = new QFormLayout();

    formLayout->setHorizontalSpacing(30);
    formLayout->setLabelAlignment(Qt::AlignLeft);

    m_gridSizeSpinBox = new QSpinBox;
    m_gridSizeSpinBox->setRange(20, 200);
    m_gridSizeSpinBox->setValue(currentGridSize);
    m_gridSizeSpinBox->setSuffix(" px");
    formLayout->addRow("Шаг сетки:", m_gridSizeSpinBox);

    m_themeComboBox = new QComboBox;
    m_themeComboBox->addItem("Imago", "imago");
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
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setObjectName("hotkeysScrollArea");
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidget(hotkeysPage);

    pagesWidget->addWidget(generalPage);
    pagesWidget->addWidget(scrollArea);

    connect(navigationList, &QListWidget::currentRowChanged, pagesWidget, &QStackedWidget::setCurrentIndex);
    navigationList->setCurrentRow(0);

    contentLayout->addWidget(navigationList);
    contentLayout->addWidget(pagesWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Применить");
    buttonBox->button(QDialogButtonBox::Cancel)->setText("Отмена");

    mainLayout->addLayout(contentLayout);
    mainLayout->addWidget(buttonBox);
}

SettingsDialog::~SettingsDialog()
{
}

int SettingsDialog::gridSize() const {
    return m_gridSizeSpinBox->value();
}

QString SettingsDialog::theme() const {
    return m_themeComboBox->currentData().toString();
}

QWidget* SettingsDialog::createHotkeysPage() {
    QWidget *hotkeysPage = new QWidget;
    QFormLayout *unifiedLayout = new QFormLayout(hotkeysPage);
    unifiedLayout->setVerticalSpacing(15);
    unifiedLayout->setHorizontalSpacing(30);

    auto addHotkeyRow = [](QFormLayout* layout, const QString& description, const QString& keys) {
        QLabel* keyLabel = new QLabel(keys);
        keyLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        keyLabel->setStyleSheet("font-weight: bold;");
        layout->addRow(description, keyLabel);
    };

    auto createSectionHeader = [](const QString& title) {
        QLabel* header = new QLabel(title);
        header->setObjectName("hotkeySectionHeader");
        return header;
    };

    unifiedLayout->addRow(createSectionHeader("Навигация и холст"));
    addHotkeyRow(unifiedLayout, "Перемещение холста:", "Зажать среднюю кнопку мыши");
    addHotkeyRow(unifiedLayout, "Масштабирование:", "Ctrl/Cmd + Колесико мыши");
    addHotkeyRow(unifiedLayout, "Приблизить:", "Ctrl/Cmd + Плюс (+)");
    addHotkeyRow(unifiedLayout, "Отдалить:", "Ctrl/Cmd + Минус (-)");
    addHotkeyRow(unifiedLayout, "Привязать все к сетке:", "Ctrl + G");

    unifiedLayout->addRow(createSectionHeader("Управление элементами"));
    addHotkeyRow(unifiedLayout, "Вставить из буфера:", "Ctrl/Cmd + V");
    addHotkeyRow(unifiedLayout, "Удалить выделенное:", "Delete/Backspace");
    addHotkeyRow(unifiedLayout, "Отменить действие:", "Ctrl/Cmd + Z");
    addHotkeyRow(unifiedLayout, "Повторить действие:", "Ctrl/Cmd + Shift + Z");

    unifiedLayout->addRow(createSectionHeader("Трансформации"));
    addHotkeyRow(unifiedLayout, "Режим изменения размера:", "Ctrl + E");
    addHotkeyRow(unifiedLayout, "Вращать по часовой:", "Ctrl + R");
    addHotkeyRow(unifiedLayout, "Вращать против часовой:", "Ctrl + Shift + R");
    addHotkeyRow(unifiedLayout, "Сохранять пропорции:", "Зажать Shift");

    unifiedLayout->addRow(createSectionHeader("Интерфейс"));
    addHotkeyRow(unifiedLayout, "Скрыть/показать панель:", "Tab");
    addHotkeyRow(unifiedLayout, "Открыть настройки:", "Ctrl/Cmd + ,");

    return hotkeysPage;
}
