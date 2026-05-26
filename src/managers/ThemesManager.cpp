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
#include <QStringList>
#include <QUrl>

namespace {

QString themesRootPath()
{
    const QString appDirPath = QCoreApplication::applicationDirPath();
    QStringList candidates;

    candidates << QDir(appDirPath).filePath("themes");
    candidates << QDir(appDirPath).filePath("../themes");
    candidates << QDir(appDirPath).filePath("../Resources/themes");

    for (const QString& candidate : candidates) {
        QDir dir(candidate);
        if (dir.exists()) {
            return dir.absolutePath();
        }
    }

    return QDir::cleanPath(QDir(appDirPath).filePath("themes"));
}

QString themeAssetUrl(const QString& themeDir, const QString& relativePath)
{
    return QUrl::fromLocalFile(QDir(themeDir).filePath(relativePath)).toString();
}

} // namespace

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

    QString themesPath = themesRootPath();
    QString defaultThemeDir = QDir(themesPath).filePath("imago"); // fallback path for icons
    const auto iconUrl = [&defaultThemeDir](const QString& relativePath) {
        return themeAssetUrl(defaultThemeDir, relativePath);
    };
    
    // Default Icons fallback (using the "imago" theme paths for relative resolution safety)
    m_themeIcons["logo"] = iconUrl("WelcomeWindow/background/logo.svg");
    m_themeIcons["character"] = iconUrl("WelcomeWindow/background/character.svg");
    m_themeIcons["button_1"] = iconUrl("WelcomeWindow/background/button_1.svg");
    m_themeIcons["button_2"] = iconUrl("WelcomeWindow/background/button_2.svg");
    m_themeIcons["circles_1"] = iconUrl("WelcomeWindow/background/circles_1.svg");
    m_themeIcons["circles_2"] = iconUrl("WelcomeWindow/background/circles_2.svg");
    m_themeIcons["circles_3"] = iconUrl("WelcomeWindow/background/circles_3.svg");
    m_themeIcons["circles_4"] = iconUrl("WelcomeWindow/background/circles_4.svg");
    m_themeIcons["dots"] = iconUrl("WelcomeWindow/background/dots.svg");
    m_themeIcons["form_1"] = iconUrl("WelcomeWindow/background/form_1.svg");
    m_themeIcons["form_2"] = iconUrl("WelcomeWindow/background/form_2.svg");
    m_themeIcons["form_3"] = iconUrl("WelcomeWindow/background/form_3.svg");
    m_themeIcons["form_4"] = iconUrl("WelcomeWindow/background/form_4.svg");
    m_themeIcons["frame"] = iconUrl("WelcomeWindow/background/frame.svg");
    m_themeIcons["line_1"] = iconUrl("WelcomeWindow/background/line_1.svg");
    m_themeIcons["line_2"] = iconUrl("WelcomeWindow/background/line_2.svg");
    m_themeIcons["line_3"] = iconUrl("WelcomeWindow/background/line_3.svg");
    m_themeIcons["lines_star"] = iconUrl("WelcomeWindow/background/lines_star.svg");
    m_themeIcons["path_1"] = iconUrl("WelcomeWindow/background/path_1.svg");
    m_themeIcons["path_2"] = iconUrl("WelcomeWindow/background/path_2.svg");
    m_themeIcons["path_3"] = iconUrl("WelcomeWindow/background/path_3.svg");
    m_themeIcons["recent_projects"] = iconUrl("WelcomeWindow/background/recent_projects.svg");
    m_themeIcons["rect_1"] = iconUrl("WelcomeWindow/background/rect_1.svg");
    m_themeIcons["rect_2"] = iconUrl("WelcomeWindow/background/rect_2.svg");
    m_themeIcons["rectangles_1"] = iconUrl("WelcomeWindow/background/rectangles_1.svg");
    m_themeIcons["rectangles_2"] = iconUrl("WelcomeWindow/background/rectangles_2.svg");
    m_themeIcons["star_1"] = iconUrl("WelcomeWindow/background/star_1.svg");
    m_themeIcons["star_2"] = iconUrl("WelcomeWindow/background/star_2.svg");
    m_themeIcons["star_3"] = iconUrl("WelcomeWindow/background/star_3.svg");
    m_themeIcons["triangles_1"] = iconUrl("WelcomeWindow/background/triangles_1.svg");
    m_themeIcons["triangles_2"] = iconUrl("WelcomeWindow/background/triangles_2.svg");
    
    m_themeIcons["emptyBoardPlaceholder"] = iconUrl("WelcomeWindow/background/empty-board.svg");
    m_themeIcons["openExistingBgPlaceholder"] = iconUrl("WelcomeWindow/background/open-existing-bg.svg");
    m_themeIcons["sidebarTabCloud"] = iconUrl("WelcomeWindow/background/sidebar-cloud.svg");
    m_themeIcons["sidebarTabLocal"] = iconUrl("WelcomeWindow/background/sidebar-local.svg");
    
    m_themeIcons["pasteIcon"] = iconUrl("MainWindow/tools/paste.svg");
    m_themeIcons["deleteIcon"] = iconUrl("MainWindow/tools/delete.svg");
    m_themeIcons["gridSnapIcon"] = iconUrl("MainWindow/tools/grid_snap.svg");
    m_themeIcons["scaleIcon"] = iconUrl("MainWindow/tools/scale.svg");
    m_themeIcons["upscaleIcon"] = iconUrl("MainWindow/tools/upscale.svg");
    m_themeIcons["cropIcon"] = iconUrl("MainWindow/tools/crop.svg");
    m_themeIcons["labelIcon"] = iconUrl("MainWindow/tools/label.svg");
    m_themeIcons["arrangeIcon"] = iconUrl("MainWindow/tools/arrange.svg");
    m_themeIcons["rotateLeftIcon"] = iconUrl("MainWindow/tools/rotate_left.svg");
    m_themeIcons["rotateRightIcon"] = iconUrl("MainWindow/tools/rotate_right.svg");
    m_themeIcons["zoomInIcon"] = iconUrl("MainWindow/tools/zoom_in.svg");
    m_themeIcons["zoomOutIcon"] = iconUrl("MainWindow/tools/zoom_out.svg");
    m_themeIcons["undoIcon"] = iconUrl("MainWindow/tools/undo.svg");
    m_themeIcons["redoIcon"] = iconUrl("MainWindow/tools/redo.svg");
    m_themeIcons["pinIcon"] = iconUrl("MainWindow/tools/pin.svg");
    m_themeIcons["settingsIcon"] = iconUrl("MainWindow/tools/settings.svg");
    m_themeIcons["opacityIcon"] = iconUrl("MainWindow/tools/opacity.svg");
    m_themeIcons["eyedropperIcon"] = iconUrl("MainWindow/tools/eyedropper.svg");
}

void ThemeManager::scanAvailableThemes()
{
    m_availableThemes.clear();
    QString themesPath = themesRootPath();
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
    
    QString themesPath = themesRootPath();
    QString themeDir = QDir(themesPath).filePath(themeName);
    QString jsonPath = QDir(themeDir).filePath("theme.json");
    
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
            QString localPath = QDir(themeDir).filePath(relativePath);
            if (QFile::exists(localPath)) {
                m_themeIcons[it.key()] = QUrl::fromLocalFile(localPath).toString();
            } else {
                qWarning() << "Файл иконки темы не найден:" << localPath;
            }
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
