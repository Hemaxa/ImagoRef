#pragma once

#include <QObject>
#include <QSettings>
#include <QString>

class SettingsManager : public QObject {
    Q_OBJECT

private:
    //конструктор
    explicit SettingsManager(QObject* parent = nullptr);
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

public:
    static SettingsManager& instance();

    void loadSettings();
    void saveSettings();

    void setThemeName(const QString& themeName);
    QString getThemeName() const;

    void setGridSize(int size);
    int getGridSize() const;

private:
    QSettings m_settings;
    QString m_currentThemeName;
    int m_gridSize;
};
