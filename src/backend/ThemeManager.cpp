#include "ThemeManager.h"

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
    return &instance();
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
    m_themeColors["borderColor"] = QColor("#333333");
    m_themeColors["panelColor"] = QColor(31, 31, 31, 230);
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
    QRegularExpression regex("@(\\w+):\\s*([^;]+);");
    auto it = regex.globalMatch(styleSheet);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString key = match.captured(1);
        QString value = match.captured(2).trimmed();
        value = value.section("/*", 0, 0).trimmed();
        
        if (!value.isEmpty()) {
            // Маппинг переменных из QSS в наши свойства
            if (key == "iconColor") {
                m_themeColors["iconColor"] = QColor(value);
                m_themeColors["accentColor"] = QColor(value);
            } else if (key == "gridColor") {
                m_themeColors["gridColor"] = QColor(value);
            }
        }
    }

    // Парсинг дополнительных цветов из CSS правил
    QRegularExpression bgRegex("background-color:\\s*([^;]+);");
    auto bgIt = bgRegex.globalMatch(styleSheet);
    if (bgIt.hasNext()) {
        QRegularExpressionMatch match = bgIt.next();
        QString value = match.captured(1).trimmed();
        if (value.startsWith("#")) {
            m_themeColors["backgroundColor"] = QColor(value);
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
