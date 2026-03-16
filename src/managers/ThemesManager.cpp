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
    QString defaultThemeDir = themesPath + "/dark"; // fallback path for icons
    
    // Default Icons fallback (using the "dark" theme paths for relative resolution safety)
    m_themeIcons["logo"] = "file://" + defaultThemeDir + "/WelcomeWindow/logo/logo.svg";
    m_themeIcons["mascot"] = "file://" + defaultThemeDir + "/WelcomeWindow/mascot/mascot.svg";
    m_themeIcons["welcomeDecoDots"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/deco-dots.svg";
    m_themeIcons["welcomeDecoTriangle"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/deco-triangle.svg";
    m_themeIcons["welcomeDecoZigzag"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/deco-zigzag.svg";
    m_themeIcons["welcomeDecoStar"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/deco-star.svg";
    m_themeIcons["projectFrame"] = "file://" + defaultThemeDir + "/WelcomeWindow/background/project-frame.svg";
    
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
    m_themeIcons["settingsIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/settings.svg";
    m_themeIcons["opacityIcon"] = "file://" + defaultThemeDir + "/MainWindow/tools/settings.svg";
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
