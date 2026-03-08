#pragma once

#include <QObject>
#include <QColor>
#include <QMap>
#include <QPixmap>
#include <QSize>
#include <QtQml/qqml.h>

/**
 * @brief ThemeManager - синглтон для управления темами приложения.
 * Предоставляет цветовые свойства для QML через Q_PROPERTY.
 */
class ThemeManager : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString currentTheme READ getCurrentTheme NOTIFY themeChanged)
    Q_PROPERTY(QStringList availableThemes READ getAvailableThemes NOTIFY availableThemesChanged)
    Q_PROPERTY(QColor backgroundColor READ getBackgroundColor NOTIFY themeChanged)
    Q_PROPERTY(QColor textColor READ getTextColor NOTIFY themeChanged)
    Q_PROPERTY(QColor accentColor READ getAccentColor NOTIFY themeChanged)
    Q_PROPERTY(QColor accentHoverColor READ getAccentHoverColor NOTIFY themeChanged)
    Q_PROPERTY(QColor accentPressedColor READ getAccentPressedColor NOTIFY themeChanged)
    Q_PROPERTY(QColor iconColor READ getIconColor NOTIFY themeChanged)
    Q_PROPERTY(QColor gridColor READ getGridColor NOTIFY themeChanged)
    Q_PROPERTY(QColor borderColor READ getBorderColor NOTIFY themeChanged)
    Q_PROPERTY(QColor panelColor READ getPanelColor NOTIFY themeChanged)
    Q_PROPERTY(QColor controlBackground READ getControlBackground NOTIFY themeChanged)

    Q_PROPERTY(QColor welcomeBgColor READ getWelcomeBgColor NOTIFY themeChanged)
    Q_PROPERTY(QColor welcomeBtnNewGradientStart READ getWelcomeBtnNewGradientStart NOTIFY themeChanged)
    Q_PROPERTY(QColor welcomeBtnNewGradientEnd READ getWelcomeBtnNewGradientEnd NOTIFY themeChanged)
    Q_PROPERTY(QColor welcomeBtnOpenColor READ getWelcomeBtnOpenColor NOTIFY themeChanged)
    Q_PROPERTY(QColor welcomeTextDark READ getWelcomeTextDark NOTIFY themeChanged)
    Q_PROPERTY(QColor welcomeAccentYellow READ getWelcomeAccentYellow NOTIFY themeChanged)

    Q_PROPERTY(QString logoPath READ getLogoPath NOTIFY themeChanged)
    Q_PROPERTY(QString mascotPath READ getMascotPath NOTIFY themeChanged)
    Q_PROPERTY(QString welcomeDecoDotsPath READ getWelcomeDecoDotsPath NOTIFY themeChanged)
    Q_PROPERTY(QString welcomeDecoTrianglePath READ getWelcomeDecoTrianglePath NOTIFY themeChanged)
    Q_PROPERTY(QString welcomeDecoZigzagPath READ getWelcomeDecoZigzagPath NOTIFY themeChanged)
    Q_PROPERTY(QString welcomeDecoStarPath READ getWelcomeDecoStarPath NOTIFY themeChanged)
    Q_PROPERTY(QString projectFramePath READ getProjectFramePath NOTIFY themeChanged)

    Q_PROPERTY(QString pasteIconPath READ getPasteIconPath NOTIFY themeChanged)
    Q_PROPERTY(QString deleteIconPath READ getDeleteIconPath NOTIFY themeChanged)
    Q_PROPERTY(QString gridSnapIconPath READ getGridSnapIconPath NOTIFY themeChanged)
    Q_PROPERTY(QString scaleIconPath READ getScaleIconPath NOTIFY themeChanged)
    Q_PROPERTY(QString upscaleIconPath READ getUpscaleIconPath NOTIFY themeChanged)
    Q_PROPERTY(QString cropIconPath READ getCropIconPath NOTIFY themeChanged)
    Q_PROPERTY(QString labelIconPath READ getLabelIconPath NOTIFY themeChanged)
    Q_PROPERTY(QString arrangeIconPath READ getArrangeIconPath NOTIFY themeChanged)
    Q_PROPERTY(QString rotateLeftIconPath READ getRotateLeftIconPath NOTIFY themeChanged)
    Q_PROPERTY(QString rotateRightIconPath READ getRotateRightIconPath NOTIFY themeChanged)
    Q_PROPERTY(QString zoomInIconPath READ getZoomInIconPath NOTIFY themeChanged)
    Q_PROPERTY(QString zoomOutIconPath READ getZoomOutIconPath NOTIFY themeChanged)
    Q_PROPERTY(QString undoIconPath READ getUndoIconPath NOTIFY themeChanged)
    Q_PROPERTY(QString redoIconPath READ getRedoIconPath NOTIFY themeChanged)
    Q_PROPERTY(QString pinIconPath READ getPinIconPath NOTIFY themeChanged)
    Q_PROPERTY(QString settingsIconPath READ getSettingsIconPath NOTIFY themeChanged)


public:
    static ThemeManager* create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    static ThemeManager& instance();

    Q_INVOKABLE void applyTheme(const QString& themeName);
    Q_INVOKABLE QPixmap colorizeSvg(const QString& path, const QColor& color, const QSize& size);

    QString getCurrentTheme() const;
    QStringList getAvailableThemes() const;

    QColor getBackgroundColor() const;
    QColor getTextColor() const;
    QColor getAccentColor() const;
    QColor getAccentHoverColor() const;
    QColor getAccentPressedColor() const;
    QColor getIconColor() const;
    QColor getGridColor() const;
    QColor getBorderColor() const;
    QColor getPanelColor() const;
    QColor getControlBackground() const;

    QColor getWelcomeBgColor() const;
    QColor getWelcomeBtnNewGradientStart() const;
    QColor getWelcomeBtnNewGradientEnd() const;
    QColor getWelcomeBtnOpenColor() const;
    QColor getWelcomeTextDark() const;
    QColor getWelcomeAccentYellow() const;

    QString getLogoPath() const;
    QString getMascotPath() const;
    QString getWelcomeDecoDotsPath() const;
    QString getWelcomeDecoTrianglePath() const;
    QString getWelcomeDecoZigzagPath() const;
    QString getWelcomeDecoStarPath() const;
    QString getProjectFramePath() const;

    QString getPasteIconPath() const;
    QString getDeleteIconPath() const;
    QString getGridSnapIconPath() const;
    QString getScaleIconPath() const;
    QString getUpscaleIconPath() const;
    QString getCropIconPath() const;
    QString getLabelIconPath() const;
    QString getArrangeIconPath() const;
    QString getRotateLeftIconPath() const;
    QString getRotateRightIconPath() const;
    QString getZoomInIconPath() const;
    QString getZoomOutIconPath() const;
    QString getUndoIconPath() const;
    QString getRedoIconPath() const;
    QString getPinIconPath() const;
    QString getSettingsIconPath() const;

signals:
    void themeChanged();
    void availableThemesChanged();

private:
    explicit ThemeManager(QObject *parent = nullptr);
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    void scanAvailableThemes();
    void loadThemeFromFile(const QString& themeName);
    void parseThemeJson(const QByteArray& jsonData, const QString& themeDir);

    QString m_currentTheme;
    QStringList m_availableThemes;
    QMap<QString, QColor> m_themeColors;
    QMap<QString, QString> m_themeIcons;
};
