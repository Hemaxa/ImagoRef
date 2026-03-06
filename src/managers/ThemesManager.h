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

public:
    static ThemeManager* create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    static ThemeManager& instance();

    Q_INVOKABLE void applyTheme(const QString& themeName);
    Q_INVOKABLE QPixmap colorizeSvg(const QString& path, const QColor& color, const QSize& size);

    QString getCurrentTheme() const;
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

signals:
    void themeChanged();

private:
    explicit ThemeManager(QObject *parent = nullptr);
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    void loadThemeFromFile(const QString& themeName);
    void parseThemeColors(const QString& styleSheet);

    QString m_currentTheme;
    QMap<QString, QColor> m_themeColors;
};
