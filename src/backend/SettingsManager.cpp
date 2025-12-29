#include "SettingsManager.h"

SettingsManager* SettingsManager::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return &instance();
}

SettingsManager& SettingsManager::instance()
{
    static SettingsManager manager;
    return manager;
}

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent)
    , m_settings("ImagoRef", "ImagoRef")
{
    loadSettings();
}

void SettingsManager::loadSettings()
{
    m_themeName = m_settings.value("theme/name", "imago").toString();
    m_gridSize = m_settings.value("canvas/gridSize", 25).toInt();
}

void SettingsManager::saveSettings()
{
    m_settings.setValue("theme/name", m_themeName);
    m_settings.setValue("canvas/gridSize", m_gridSize);
}

QString SettingsManager::themeName() const
{
    return m_themeName;
}

void SettingsManager::setThemeName(const QString& name)
{
    if (m_themeName != name) {
        m_themeName = name;
        saveSettings();
        emit themeNameChanged();
    }
}

int SettingsManager::gridSize() const
{
    return m_gridSize;
}

void SettingsManager::setGridSize(int size)
{
    if (m_gridSize != size) {
        m_gridSize = size;
        saveSettings();
        emit gridSizeChanged();
    }
}
