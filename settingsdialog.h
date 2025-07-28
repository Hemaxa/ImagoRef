#pragma once
#include <QDialog> //класс создания диалоговых окон Qt

class QSpinBox;
class QComboBox;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(int currentGridSize, const QString &currentTheme, QWidget *parent = nullptr);

    int gridSize() const; //выбранный шаг сетки
    QString theme() const; //выбранная тема приложения

private:
    QWidget* createHotkeysPage();

    QSpinBox *m_gridSizeSpinBox;
    QComboBox *m_themeComboBox;
};
