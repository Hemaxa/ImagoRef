#include "SettingsManager.h"

SettingsManager& SettingsManager::instance() {
    static SettingsManager manager;
    return manager;
}

SettingsManager::SettingsManager(QObject* parent) : QObject(parent), m_settings("ImagoRef", "ImagoRef") {}

void SettingsManager::loadSettings() {
    m_currentThemeName = m_settings.value("theme/name", "imago").toString();
    m_gridSize = m_settings.value("canvas/gridSize", 25).toInt();
}

void SettingsManager::saveSettings() {
    m_settings.setValue("theme/name", m_currentThemeName);
    m_settings.setValue("canvas/gridSize", m_gridSize);
}

void SettingsManager::setThemeName(const QString& themeName) { m_currentThemeName = themeName; }
QString SettingsManager::getThemeName() const { return m_currentThemeName; }

void SettingsManager::setGridSize(int size) { m_gridSize = size; }
int SettingsManager::getGridSize() const { return m_gridSize; }
