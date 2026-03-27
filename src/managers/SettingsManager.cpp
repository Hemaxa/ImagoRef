#include "SettingsManager.h"
#include <QJSEngine>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

SettingsManager* SettingsManager::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    auto *inst = &instance();
    QJSEngine::setObjectOwnership(inst, QJSEngine::CppOwnership);
    return inst;
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
    m_hasPromptedUpscale = m_settings.value("models/hasPromptedUpscale", false).toBool();
    m_toolbarColumns = m_settings.value("toolbar/columns", 1).toInt();
    m_colorCopyMode = m_settings.value("colorCopyMode", 0).toInt();
    m_colorHistory = m_settings.value("colorHistory", QStringList()).toStringList();
    m_jwtToken = m_settings.value("auth/jwtToken", "").toString();
    m_userEmail = m_settings.value("auth/userEmail", "").toString();
    
    // Загрузка recentBoards из JSON строки
    m_recentBoards.clear();
    QString recentJson = m_settings.value("history/recentBoardsJson", "").toString();
    if (!recentJson.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(recentJson.toUtf8());
        if (doc.isArray()) {
            QJsonArray arr = doc.array();
            for (const QJsonValue &val : arr) {
                QJsonObject obj = val.toObject();
                QVariantMap board;
                board["id"] = obj["id"].toString();
                board["path"] = obj["path"].toString();
                board["name"] = obj["name"].toString();
                board["type"] = obj["type"].toString();
                m_recentBoards.append(board);
            }
        }
    }
    
    // Tools enablement
    m_toolsEnablement.clear();
    m_settings.beginGroup("tools");
    QStringList toolKeys = m_settings.childKeys();
    for (const QString& key : toolKeys) {
        m_toolsEnablement[key] = m_settings.value(key).toBool();
    }
    m_settings.endGroup();
}

void SettingsManager::saveSettings()
{
    m_settings.setValue("theme/name", m_themeName);
    m_settings.setValue("canvas/gridSize", m_gridSize);
    m_settings.setValue("canvas/pattern", m_canvasPattern);
    m_settings.setValue("label/fontSize", m_labelFontSize);
    m_settings.setValue("arrange/spacing", m_arrangeSpacing);
    m_settings.setValue("models/hasPromptedUpscale", m_hasPromptedUpscale);
    m_settings.setValue("toolbar/columns", m_toolbarColumns);
    m_settings.setValue("colorCopyMode", m_colorCopyMode);
    m_settings.setValue("colorHistory", m_colorHistory);
    m_settings.setValue("auth/jwtToken", m_jwtToken);
    m_settings.setValue("auth/userEmail", m_userEmail);
    
    // Сохранение recentBoards как JSON строка
    QJsonArray arr;
    for (const QVariant &v : m_recentBoards) {
        QVariantMap map = v.toMap();
        QJsonObject obj;
        obj["id"] = map.value("id").toString();
        obj["path"] = map.value("path").toString();
        obj["name"] = map.value("name").toString();
        obj["type"] = map.value("type").toString();
        arr.append(obj);
    }
    m_settings.setValue("history/recentBoardsJson", QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact)));
    
    m_settings.beginGroup("tools");
    for (auto it = m_toolsEnablement.constBegin(); it != m_toolsEnablement.constEnd(); ++it) {
        m_settings.setValue(it.key(), it.value());
    }
    m_settings.endGroup();
}

QString SettingsManager::getThemeName() const
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

int SettingsManager::getGridSize() const
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

QString SettingsManager::getCanvasPattern() const
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

int SettingsManager::getLabelFontSize() const
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

int SettingsManager::getArrangeSpacing() const
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

bool SettingsManager::getHasPromptedUpscale() const { return m_hasPromptedUpscale; }

void SettingsManager::setHasPromptedUpscale(bool prompted)
{
    if (m_hasPromptedUpscale != prompted) {
        m_hasPromptedUpscale = prompted;
        saveSettings();
        emit hasPromptedUpscaleChanged();
    }
}

int SettingsManager::getToolbarColumns() const { return m_toolbarColumns; }

void SettingsManager::setToolbarColumns(int columns)
{
    if (m_toolbarColumns != columns) {
        m_toolbarColumns = columns;
        saveSettings();
        emit toolbarColumnsChanged();
    }
}

int SettingsManager::getColorCopyMode() const
{
    return m_colorCopyMode;
}

void SettingsManager::setColorCopyMode(int mode)
{
    if (m_colorCopyMode != mode) {
        m_colorCopyMode = mode;
        saveSettings();
        emit colorCopyModeChanged();
    }
}

QStringList SettingsManager::getColorHistory() const
{
    return m_colorHistory;
}

void SettingsManager::setColorHistory(const QStringList& history)
{
    if (m_colorHistory != history) {
        m_colorHistory = history;
        saveSettings();
        emit colorHistoryChanged();
    }
}

void SettingsManager::addColorToHistory(const QString& hexColor)
{
    // Remove if exists to move to front
    m_colorHistory.removeAll(hexColor);
    
    // Add to front
    m_colorHistory.prepend(hexColor);
    
    // Keep only 6 elements
    while (m_colorHistory.size() > 6) {
        m_colorHistory.removeLast();
    }
    
    saveSettings();
    emit colorHistoryChanged();
}

QString SettingsManager::getJwtToken() const
{
    return m_jwtToken;
}

void SettingsManager::setJwtToken(const QString& token)
{
    if (m_jwtToken != token) {
        m_jwtToken = token;
        saveSettings();
        emit jwtTokenChanged();
    }
}

QString SettingsManager::getUserEmail() const
{
    return m_userEmail;
}

void SettingsManager::setUserEmail(const QString& email)
{
    if (m_userEmail != email) {
        m_userEmail = email;
        saveSettings();
        emit userEmailChanged();
    }
}

QVariantList SettingsManager::getRecentBoards() const
{
    return m_recentBoards;
}

void SettingsManager::setRecentBoards(const QVariantList& boards)
{
    if (m_recentBoards != boards) {
        m_recentBoards = boards;
        saveSettings();
        emit recentBoardsChanged();
    }
}

void SettingsManager::addRecentBoard(const QVariantMap& board)
{
    // Check if board with same ID or path already exists, and remove it to push to top
    QString boardIdOrPath = board.value("id").toString();
    if (boardIdOrPath.isEmpty()) {
        boardIdOrPath = board.value("path").toString();
    }

    for (int i = 0; i < m_recentBoards.size(); ++i) {
        QVariantMap existing = m_recentBoards[i].toMap();
        QString existingIdOrPath = existing.value("id").toString();
        if (existingIdOrPath.isEmpty()) {
            existingIdOrPath = existing.value("path").toString();
        }
        
        if (existingIdOrPath == boardIdOrPath && !boardIdOrPath.isEmpty()) {
            m_recentBoards.removeAt(i);
            break;
        }
    }

    m_recentBoards.prepend(board);

    while (m_recentBoards.size() > 6) {
        m_recentBoards.removeLast();
    }

    saveSettings();
    emit recentBoardsChanged();
}

bool SettingsManager::isToolEnabled(const QString &toolName) const
{
    // По умолчанию все инструменты включены
    return m_toolsEnablement.value(toolName, true);
}

void SettingsManager::setToolEnabled(const QString &toolName, bool enabled)
{
    if (m_toolsEnablement.value(toolName, true) != enabled) {
        m_toolsEnablement[toolName] = enabled;
        saveSettings();
        emit toolEnablementChanged(toolName, enabled);
    }
}
