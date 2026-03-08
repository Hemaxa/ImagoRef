#include "ThemesManager.h"
#include <QJSEngine>

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <QSvgRenderer>
#include <QPainter>
#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

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
    // Значения по умолчанию
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

    m_themeColors["welcomeBgColor"] = QColor("#FF6B35");
    m_themeColors["welcomeBtnNewGradientStart"] = QColor("#FF69B4");
    m_themeColors["welcomeBtnNewGradientEnd"] = QColor("#00CED1");
    m_themeColors["welcomeBtnOpenColor"] = QColor("#ADFF2F");
    m_themeColors["welcomeTextDark"] = QColor("#141414");
    m_themeColors["welcomeAccentYellow"] = QColor("#FFE135");

    scanAvailableThemes();
}

void ThemeManager::scanAvailableThemes()
{
    m_availableThemes.clear();
    QString themesPath = QCoreApplication::applicationDirPath() + "/themes";
    QDir dir(themesPath);
    
    if (dir.exists()) {
        QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString& subdir : subdirs) {
            QString jsonPath = dir.absoluteFilePath(subdir + "/theme.json");
            if (QFile::exists(jsonPath)) {
                m_availableThemes.append(subdir);
            }
        }
    }
    emit availableThemesChanged();
}

void ThemeManager::applyTheme(const QString& themeName)
{
    loadThemeFromFile(themeName);
    m_currentTheme = themeName;
    emit themeChanged();
}

void ThemeManager::loadThemeFromFile(const QString& themeName)
{
    QString themesPath = QCoreApplication::applicationDirPath() + "/themes";
    QString themeDir = themesPath + "/" + themeName;
    QString jsonPath = themeDir + "/theme.json";
    
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть файл темы:" << jsonPath;
        return;
    }
    
    QByteArray jsonData = file.readAll();
    file.close();
    
    parseThemeJson(jsonData, themeDir);
}

void ThemeManager::parseThemeJson(const QByteArray& jsonData, const QString& themeDir)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Ошибка парсинга theme.json:" << error.errorString();
        return;
    }
    
    QJsonObject root = doc.object();
    
    if (root.contains("colors")) {
        QJsonObject colors = root["colors"].toObject();
        for (auto it = colors.begin(); it != colors.end(); ++it) {
            m_themeColors[it.key()] = QColor(it.value().toString());
        }
    }
    
    if (root.contains("icons")) {
        QJsonObject icons = root["icons"].toObject();
        for (auto it = icons.begin(); it != icons.end(); ++it) {
            QString relativePath = it.value().toString();
            QString absolutePath = themeDir + "/" + relativePath;
            m_themeIcons[it.key()] = "file://" + absolutePath;
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

QString ThemeManager::getCurrentTheme() const { return m_currentTheme; }
QStringList ThemeManager::getAvailableThemes() const { return m_availableThemes; }

QColor ThemeManager::getBackgroundColor() const { return m_themeColors.value("backgroundColor"); }
QColor ThemeManager::getTextColor() const { return m_themeColors.value("textColor"); }
QColor ThemeManager::getAccentColor() const { return m_themeColors.value("accentColor"); }
QColor ThemeManager::getAccentHoverColor() const { return m_themeColors.value("accentHoverColor"); }
QColor ThemeManager::getAccentPressedColor() const { return m_themeColors.value("accentPressedColor"); }
QColor ThemeManager::getIconColor() const { return m_themeColors.value("iconColor"); }
QColor ThemeManager::getGridColor() const { return m_themeColors.value("gridColor"); }
QColor ThemeManager::getBorderColor() const { return m_themeColors.value("borderColor"); }
QColor ThemeManager::getPanelColor() const { return m_themeColors.value("panelColor"); }
QColor ThemeManager::getControlBackground() const { return m_themeColors.value("controlBackground"); }

QColor ThemeManager::getWelcomeBgColor() const { return m_themeColors.value("welcomeBgColor"); }
QColor ThemeManager::getWelcomeBtnNewGradientStart() const { return m_themeColors.value("welcomeBtnNewGradientStart"); }
QColor ThemeManager::getWelcomeBtnNewGradientEnd() const { return m_themeColors.value("welcomeBtnNewGradientEnd"); }
QColor ThemeManager::getWelcomeBtnOpenColor() const { return m_themeColors.value("welcomeBtnOpenColor"); }
QColor ThemeManager::getWelcomeTextDark() const { return m_themeColors.value("welcomeTextDark"); }
QColor ThemeManager::getWelcomeAccentYellow() const { return m_themeColors.value("welcomeAccentYellow"); }

QString ThemeManager::getLogoPath() const { return m_themeIcons.value("logo"); }
QString ThemeManager::getMascotPath() const { return m_themeIcons.value("mascot"); }
QString ThemeManager::getWelcomeDecoDotsPath() const { return m_themeIcons.value("welcomeDecoDots"); }
QString ThemeManager::getWelcomeDecoTrianglePath() const { return m_themeIcons.value("welcomeDecoTriangle"); }
QString ThemeManager::getWelcomeDecoZigzagPath() const { return m_themeIcons.value("welcomeDecoZigzag"); }
QString ThemeManager::getWelcomeDecoStarPath() const { return m_themeIcons.value("welcomeDecoStar"); }
QString ThemeManager::getProjectFramePath() const { return m_themeIcons.value("projectFrame"); }

QString ThemeManager::getPasteIconPath() const { return m_themeIcons.value("pasteIcon"); }
QString ThemeManager::getDeleteIconPath() const { return m_themeIcons.value("deleteIcon"); }
QString ThemeManager::getGridSnapIconPath() const { return m_themeIcons.value("gridSnapIcon"); }
QString ThemeManager::getScaleIconPath() const { return m_themeIcons.value("scaleIcon"); }
QString ThemeManager::getUpscaleIconPath() const { return m_themeIcons.value("upscaleIcon", m_themeIcons.value("scaleIcon")); }
QString ThemeManager::getCropIconPath() const { return m_themeIcons.value("cropIcon"); }
QString ThemeManager::getLabelIconPath() const { return m_themeIcons.value("labelIcon"); }
QString ThemeManager::getArrangeIconPath() const { return m_themeIcons.value("arrangeIcon"); }
QString ThemeManager::getRotateLeftIconPath() const { return m_themeIcons.value("rotateLeftIcon"); }
QString ThemeManager::getRotateRightIconPath() const { return m_themeIcons.value("rotateRightIcon"); }
QString ThemeManager::getZoomInIconPath() const { return m_themeIcons.value("zoomInIcon"); }
QString ThemeManager::getZoomOutIconPath() const { return m_themeIcons.value("zoomOutIcon"); }
QString ThemeManager::getUndoIconPath() const { return m_themeIcons.value("undoIcon"); }
QString ThemeManager::getRedoIconPath() const { return m_themeIcons.value("redoIcon"); }
QString ThemeManager::getPinIconPath() const { return m_themeIcons.value("pinIcon"); }
QString ThemeManager::getSettingsIconPath() const { return m_themeIcons.value("settingsIcon"); }
