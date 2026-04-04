#pragma once

#include <QObject>
#include <QSettings>
#include <QString>
#include <QtQml/qqml.h>

/**
 * @brief SettingsManager - синглтон для управления настройками приложения.
 * Адаптирован для доступа из QML через Q_PROPERTY.
 */
class SettingsManager : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString themeName READ getThemeName WRITE setThemeName NOTIFY themeNameChanged)
    Q_PROPERTY(int gridSize READ getGridSize WRITE setGridSize NOTIFY gridSizeChanged)
    Q_PROPERTY(QString canvasPattern READ getCanvasPattern WRITE setCanvasPattern NOTIFY canvasPatternChanged)
    Q_PROPERTY(int labelFontSize READ getLabelFontSize WRITE setLabelFontSize NOTIFY labelFontSizeChanged)
    Q_PROPERTY(int arrangeSpacing READ getArrangeSpacing WRITE setArrangeSpacing NOTIFY arrangeSpacingChanged)
    Q_PROPERTY(bool hasPromptedUpscale READ getHasPromptedUpscale WRITE setHasPromptedUpscale NOTIFY hasPromptedUpscaleChanged)
    Q_PROPERTY(int toolbarColumns READ getToolbarColumns WRITE setToolbarColumns NOTIFY toolbarColumnsChanged)
    Q_PROPERTY(int colorCopyMode READ getColorCopyMode WRITE setColorCopyMode NOTIFY colorCopyModeChanged)
    Q_PROPERTY(QStringList colorHistory READ getColorHistory WRITE setColorHistory NOTIFY colorHistoryChanged)
    Q_PROPERTY(QString jwtToken READ getJwtToken WRITE setJwtToken NOTIFY jwtTokenChanged)
    Q_PROPERTY(QString userEmail READ getUserEmail WRITE setUserEmail NOTIFY userEmailChanged)
    Q_PROPERTY(QString userNickname READ getUserNickname WRITE setUserNickname NOTIFY userNicknameChanged)
    Q_PROPERTY(QString userAvatarHash READ getUserAvatarHash WRITE setUserAvatarHash NOTIFY userAvatarHashChanged)
    Q_PROPERTY(QVariantList recentBoards READ getRecentBoards WRITE setRecentBoards NOTIFY recentBoardsChanged)

public:
    static SettingsManager* create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    static SettingsManager& instance();

    void loadSettings();
    void saveSettings();

    QString getThemeName() const;
    void setThemeName(const QString& name);

    int getGridSize() const;
    void setGridSize(int size);

    QString getCanvasPattern() const;
    void setCanvasPattern(const QString& pattern);

    int getLabelFontSize() const;
    void setLabelFontSize(int size);

    int getArrangeSpacing() const;
    void setArrangeSpacing(int spacing);

    bool getHasPromptedUpscale() const;
    void setHasPromptedUpscale(bool prompted);

    int getToolbarColumns() const;
    void setToolbarColumns(int columns);

    int getColorCopyMode() const;
    void setColorCopyMode(int mode);

    QStringList getColorHistory() const;
    void setColorHistory(const QStringList& history);
    Q_INVOKABLE void addColorToHistory(const QString& hexColor);

    QString getJwtToken() const;
    void setJwtToken(const QString& token);

    QString getUserEmail() const;
    void setUserEmail(const QString& email);

    QString getUserNickname() const;
    void setUserNickname(const QString& nickname);

    QString getUserAvatarHash() const;
    void setUserAvatarHash(const QString& hash);

    QVariantList getRecentBoards() const;
    void setRecentBoards(const QVariantList& boards);
    Q_INVOKABLE void addRecentBoard(const QVariantMap& board);
    Q_INVOKABLE void updateBoardPreview(const QString& idOrPath, const QString& previewPath);
    
    Q_INVOKABLE bool isToolEnabled(const QString &toolName) const;
    Q_INVOKABLE void setToolEnabled(const QString &toolName, bool enabled);

signals:
    void themeNameChanged();
    void gridSizeChanged();
    void canvasPatternChanged();
    void labelFontSizeChanged();
    void arrangeSpacingChanged();
    void hasPromptedUpscaleChanged();
    void toolbarColumnsChanged();
    void colorCopyModeChanged();
    void colorHistoryChanged();
    void jwtTokenChanged();
    void userEmailChanged();
    void userNicknameChanged();
    void userAvatarHashChanged();
    void recentBoardsChanged();
    
    void toolEnablementChanged(QString toolName, bool enabled);

private:
    explicit SettingsManager(QObject* parent = nullptr);
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    QSettings m_settings;
    QString m_themeName;
    int m_gridSize;
    QString m_canvasPattern;
    int m_labelFontSize;
    int m_arrangeSpacing;
    bool m_hasPromptedUpscale;
    int m_toolbarColumns;
    int m_colorCopyMode;
    QStringList m_colorHistory;
    QString m_jwtToken;
    QString m_userEmail;
    QString m_userNickname;
    QString m_userAvatarHash;
    QVariantList m_recentBoards;
    QHash<QString, bool> m_toolsEnablement;
};
