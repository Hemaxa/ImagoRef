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
    Q_PROPERTY(QVariantMap colors READ getColors NOTIFY themeChanged)
    Q_PROPERTY(QVariantMap icons READ getIcons NOTIFY themeChanged)


public:
    static ThemeManager* create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    static ThemeManager& instance();

    Q_INVOKABLE void applyTheme(const QString& themeName);
    Q_INVOKABLE QPixmap colorizeSvg(const QString& path, const QColor& color, const QSize& size);

    QString getCurrentTheme() const;
    QStringList getAvailableThemes() const;

    QVariantMap getColors() const;
    QVariantMap getIcons() const;

signals:
    void themeChanged();
    void availableThemesChanged();

private:
    explicit ThemeManager(QObject *parent = nullptr);
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    void resetToDefaults();
    void scanAvailableThemes();
    void loadThemeFromFile(const QString& themeName);
    void parseThemeJson(const QByteArray& jsonData, const QString& themeDir);

    QString m_currentTheme;
    QStringList m_availableThemes;
    QVariantMap m_themeColors;
    QVariantMap m_themeIcons;
};
