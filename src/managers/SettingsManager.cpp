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
    m_canvasPattern = m_settings.value("canvas/pattern", "dots").toString();
    m_labelFontSize = m_settings.value("label/fontSize", 14).toInt();
    m_arrangeSpacing = m_settings.value("arrange/spacing", 20).toInt();
}

void SettingsManager::saveSettings()
{
    m_settings.setValue("theme/name", m_themeName);
    m_settings.setValue("canvas/gridSize", m_gridSize);
    m_settings.setValue("canvas/pattern", m_canvasPattern);
    m_settings.setValue("label/fontSize", m_labelFontSize);
    m_settings.setValue("arrange/spacing", m_arrangeSpacing);
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

QString SettingsManager::canvasPattern() const
{
    return m_canvasPattern;
}

void SettingsManager::setCanvasPattern(const QString& pattern)
{
    if (m_canvasPattern != pattern) {
        m_canvasPattern = pattern;
        saveSettings();
        emit canvasPatternChanged();
    }
}

int SettingsManager::labelFontSize() const
{
    return m_labelFontSize;
}

void SettingsManager::setLabelFontSize(int size)
{
    if (m_labelFontSize != size) {
        m_labelFontSize = size;
        saveSettings();
        emit labelFontSizeChanged();
    }
}

int SettingsManager::arrangeSpacing() const
{
    return m_arrangeSpacing;
}

void SettingsManager::setArrangeSpacing(int spacing)
{
    if (m_arrangeSpacing != spacing) {
        m_arrangeSpacing = spacing;
        saveSettings();
        emit arrangeSpacingChanged();
    }
}
