#include "ThemesManager.h"
#include <QJSEngine>

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <QSvgRenderer>
#include <QPainter>

ThemeManager* ThemeManager::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    auto *inst = &instance();
    QJSEngine::setObjectOwnership(inst, QJSEngine::CppOwnership);
    return inst;
}

ThemeManager& ThemeManager::instance()
{
    static ThemeManager manager;
    return manager;
}

ThemeManager::ThemeManager(QObject *parent) : QObject(parent)
{
    // Значения по умолчанию (тема imago)
    m_themeColors["backgroundColor"] = QColor("#141414");
    m_themeColors["textColor"] = QColor("#e0e0e0");
    m_themeColors["accentColor"] = QColor("#E67E22");
    m_themeColors["accentHoverColor"] = QColor("#F39C12");
    m_themeColors["accentPressedColor"] = QColor("#D35400");
    m_themeColors["iconColor"] = QColor("#E67E22");
    m_themeColors["gridColor"] = QColor("#3c3c3c");
    m_themeColors["borderColor"] = QColor("#3c3c3c");
    m_themeColors["panelColor"] = QColor("#3c3c3c");
    m_themeColors["controlBackground"] = QColor("#1f1f1f");
}

void ThemeManager::applyTheme(const QString& themeName)
{
    loadThemeFromFile(themeName);
    m_currentTheme = themeName;
    emit themeChanged();
}

void ThemeManager::loadThemeFromFile(const QString& themeName)
{
    QString filePath = QString(":/themes/themes/%1.qss").arg(themeName);
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл темы:" << filePath;
        return;
    }

    QString styleSheet = file.readAll();
    file.close();

    parseThemeColors(styleSheet);
}

void ThemeManager::parseThemeColors(const QString& styleSheet)
{
    // Парсим все переменные формата @key: value;
    QRegularExpression regex("@(\\w+):\\s*([^;]+);");
    auto it = regex.globalMatch(styleSheet);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString key = match.captured(1);
        QString value = match.captured(2).trimmed();

        // Удаляем комментарии после значения
        value = value.section("/*", 0, 0).trimmed();
        if (value.isEmpty()) continue;

        // Парсим цвет — поддержка hex (#rrggbb) и rgba(r,g,b,a)
        QColor color;
        if (value.startsWith("rgba(")) {
            // rgba(r, g, b, a) — a может быть целым (0-255) или дробным (0.0-1.0)
            QRegularExpression rgbaRegex("rgba\\((\\d+)\\s*,\\s*(\\d+)\\s*,\\s*(\\d+)\\s*,\\s*([\\d.]+)\\)");
            auto rgbaMatch = rgbaRegex.match(value);
            if (rgbaMatch.hasMatch()) {
                int r = rgbaMatch.captured(1).toInt();
                int g = rgbaMatch.captured(2).toInt();
                int b = rgbaMatch.captured(3).toInt();
                double a = rgbaMatch.captured(4).toDouble();
                // Если a > 1, трактуем как 0-255; иначе как 0.0-1.0
                int alpha = (a > 1.0) ? static_cast<int>(a) : static_cast<int>(a * 255);
                color = QColor(r, g, b, alpha);
            }
        } else {
            color = QColor(value);
        }

        if (color.isValid()) {
            m_themeColors[key] = color;
        }
    }
}

QPixmap ThemeManager::colorizeSvg(const QString& path, const QColor& color, const QSize& size)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть SVG-файл:" << path;
        return QPixmap();
    }
    QString svgData = QTextStream(&file).readAll();
    file.close();

    svgData.replace("currentColor", color.name(QColor::HexRgb));

    QByteArray svgBytes = svgData.toUtf8();
    QSvgRenderer renderer(svgBytes);

    if (!renderer.isValid()) {
        qWarning() << "Не удалось отрендерить SVG-файл:" << path;
        return QPixmap();
    }

    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    renderer.render(&painter);

    return pixmap;
}

QString ThemeManager::currentTheme() const { return m_currentTheme; }
QColor ThemeManager::backgroundColor() const { return m_themeColors.value("backgroundColor"); }
QColor ThemeManager::textColor() const { return m_themeColors.value("textColor"); }
QColor ThemeManager::accentColor() const { return m_themeColors.value("accentColor"); }
QColor ThemeManager::accentHoverColor() const { return m_themeColors.value("accentHoverColor"); }
QColor ThemeManager::accentPressedColor() const { return m_themeColors.value("accentPressedColor"); }
QColor ThemeManager::iconColor() const { return m_themeColors.value("iconColor"); }
QColor ThemeManager::gridColor() const { return m_themeColors.value("gridColor"); }
QColor ThemeManager::borderColor() const { return m_themeColors.value("borderColor"); }
QColor ThemeManager::panelColor() const { return m_themeColors.value("panelColor"); }
QColor ThemeManager::controlBackground() const { return m_themeColors.value("controlBackground"); }
