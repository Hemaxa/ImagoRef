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

    Q_PROPERTY(QString currentTheme READ currentTheme NOTIFY themeChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY themeChanged)
    Q_PROPERTY(QColor textColor READ textColor NOTIFY themeChanged)
    Q_PROPERTY(QColor accentColor READ accentColor NOTIFY themeChanged)
    Q_PROPERTY(QColor accentHoverColor READ accentHoverColor NOTIFY themeChanged)
    Q_PROPERTY(QColor accentPressedColor READ accentPressedColor NOTIFY themeChanged)
    Q_PROPERTY(QColor iconColor READ iconColor NOTIFY themeChanged)
    Q_PROPERTY(QColor gridColor READ gridColor NOTIFY themeChanged)
    Q_PROPERTY(QColor borderColor READ borderColor NOTIFY themeChanged)
    Q_PROPERTY(QColor panelColor READ panelColor NOTIFY themeChanged)
    Q_PROPERTY(QColor controlBackground READ controlBackground NOTIFY themeChanged)

public:
    static ThemeManager* create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    static ThemeManager& instance();

    Q_INVOKABLE void applyTheme(const QString& themeName);
    Q_INVOKABLE QPixmap colorizeSvg(const QString& path, const QColor& color, const QSize& size);

    QString currentTheme() const;
    QColor backgroundColor() const;
    QColor textColor() const;
    QColor accentColor() const;
    QColor accentHoverColor() const;
    QColor accentPressedColor() const;
    QColor iconColor() const;
    QColor gridColor() const;
    QColor borderColor() const;
    QColor panelColor() const;
    QColor controlBackground() const;

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
