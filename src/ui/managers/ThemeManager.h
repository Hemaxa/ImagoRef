#pragma once

#include <QObject>
#include <QColor>
#include <QIcon>
#include <QMap>
#include <QPixmap>
#include <QSize>

class ThemeManager : public QObject {
    Q_OBJECT

private:
    explicit ThemeManager(QObject *parent = nullptr);
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

public:
    static ThemeManager& instance();

    void applyTheme(const QString& themeName);
    QPixmap colorizeSvg(const QString& path, const QColor& color, const QSize& size);
    QColor getColor(const QString& key) const;
    QColor getIconColor() const;

signals:
    void themeChanged(const QColor& iconColor, const QColor& gridColor);

private:
    void loadThemeFromFile(const QString& themeName);
    void parseThemeColors(const QString& styleSheet);

    QMap<QString, QColor> m_themeColors;
    QColor m_iconColor;
};
