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
    resetToDefaults();
    scanAvailableThemes();
}

void ThemeManager::resetToDefaults()
{
    m_themeColors.clear();
    m_themeIcons.clear();
    
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

    QString themesPath = QCoreApplication::applicationDirPath() + "/themes";
    QString defaultThemeDir = themesPath + "/imago"; // fallback path for icons
    
    // Default Icons fallback (using the "imago" theme paths for relative resolution safety)
    m_themeIcons["logo"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/logo.svg";
    m_themeIcons["character"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/character.svg";
    m_themeIcons["button_1"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/button_1.svg";
    m_themeIcons["button_2"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/button_2.svg";
    m_themeIcons["circles_1"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/circles_1.svg";
    m_themeIcons["circles_2"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/circles_2.svg";
    m_themeIcons["circles_3"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/circles_3.svg";
    m_themeIcons["circles_4"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/circles_4.svg";
    m_themeIcons["dots"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/dots.svg";
    m_themeIcons["form_1"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/form_1.svg";
    m_themeIcons["form_2"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/form_2.svg";
    m_themeIcons["form_3"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/form_3.svg";
    m_themeIcons["form_4"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/form_4.svg";
    m_themeIcons["frame"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/frame.svg";
    m_themeIcons["line_1"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/line_1.svg";
    m_themeIcons["line_2"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/line_2.svg";
    m_themeIcons["line_3"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/line_3.svg";
    m_themeIcons["lines_star"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/lines_star.svg";
    m_themeIcons["path_1"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/path_1.svg";
    m_themeIcons["path_2"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/path_2.svg";
    m_themeIcons["path_3"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/path_3.svg";
    m_themeIcons["recent_projects"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/recent_projects.svg";
    m_themeIcons["rect_1"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/rect_1.svg";
    m_themeIcons["rect_2"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/rect_2.svg";
    m_themeIcons["rectangles_1"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/rectangles_1.svg";
    m_themeIcons["rectangles_2"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/rectangles_2.svg";
    m_themeIcons["star_1"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/star_1.svg";
    m_themeIcons["star_2"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/star_2.svg";
    m_themeIcons["star_3"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/star_3.svg";
    m_themeIcons["triangles_1"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/triangles_1.svg";
    m_themeIcons["triangles_2"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/triangles_2.svg";
    
    m_themeIcons["emptyBoardPlaceholder"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/empty-board.svg";
    m_themeIcons["openExistingBgPlaceholder"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/open-existing-bg.svg";
    m_themeIcons["sidebarTabCloud"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/sidebar-cloud.svg";
    m_themeIcons["sidebarTabLocal"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/sidebar-local.svg";
    
    m_themeIcons["pasteIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/paste.svg";
    m_themeIcons["deleteIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/delete.svg";
    m_themeIcons["gridSnapIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/grid_snap.svg";
    m_themeIcons["scaleIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/scale.svg";
    m_themeIcons["upscaleIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/upscale.svg";
    m_themeIcons["cropIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/crop.svg";
    m_themeIcons["labelIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/label.svg";
    m_themeIcons["arrangeIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/arrange.svg";
    m_themeIcons["rotateLeftIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/rotate_left.svg";
    m_themeIcons["rotateRightIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/rotate_right.svg";
    m_themeIcons["zoomInIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/zoom_in.svg";
    m_themeIcons["zoomOutIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/zoom_out.svg";
    m_themeIcons["undoIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/undo.svg";
    m_themeIcons["redoIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/redo.svg";
    m_themeIcons["pinIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/pin.svg";
    m_themeIcons["settingsIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/label.svg";
    m_themeIcons["opacityIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/label.svg";
    m_themeIcons["eyedropperIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/label.svg";
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
    resetToDefaults();
    
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

QVariantMap ThemeManager::getColors() const { return m_themeColors; }
QVariantMap ThemeManager::getIcons() const { return m_themeIcons; }
