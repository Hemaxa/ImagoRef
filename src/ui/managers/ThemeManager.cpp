#include "ThemeManager.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <QSvgRenderer>
#include <QPainter>

ThemeManager& ThemeManager::instance() {
    static ThemeManager manager;
    return manager;
}

ThemeManager::ThemeManager(QObject *parent) : QObject(parent) {}

void ThemeManager::applyTheme(const QString& themeName) {
    loadThemeFromFile(themeName);
}

void ThemeManager::loadThemeFromFile(const QString& themeName) {
    QString filePath = QString(":/themes/themes/%1.qss").arg(themeName);
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл темы:" << filePath;
        return;
    }

    QString styleSheet = file.readAll();
    file.close();

    // 1. Парсим цвета (как и раньше)
    parseThemeColors(styleSheet);
    m_iconColor = getColor("iconColor"); // Получаем цвет для иконок

    // 2. ✅ НОВЫЙ ШАГ: Создаем "чистую" версию QSS
    //    Мы удаляем все строки вида "@variable: ...;"
    QString cleanStyleSheet = styleSheet;
    QRegularExpression regex("@(\\w+):\\s*([^;]+);");
    cleanStyleSheet.remove(regex); // Удаляем все определения переменных

    // 3. Применяем "чистую" QSS, которую Qt сможет понять
    qApp->setStyleSheet(cleanStyleSheet);

    // 4. Сообщаем всем подписчикам, что тема изменилась
    emit themeChanged(m_iconColor, getColor("gridColor")); // Получаем цвет для сетки
}

void ThemeManager::parseThemeColors(const QString& styleSheet) {
    m_themeColors.clear();
    QRegularExpression regex("@(\\w+):\\s*([^;]+);");
    auto it = regex.globalMatch(styleSheet);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString key = match.captured(1);
        QString value = match.captured(2).trimmed();
        value = value.section("/*", 0, 0).trimmed();
        if (!value.isEmpty()) {
            m_themeColors.insert(key, QColor(value));
        }
    }
}

QPixmap ThemeManager::colorizeSvg(const QString& path, const QColor& color, const QSize& size)
{
    // 1. Читаем файл
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть SVG-файл:" << path;
        return QPixmap(); // Возвращаем пустой Pixmap
    }
    QString svgData = QTextStream(&file).readAll();
    file.close();

    // 2. Заменяем "currentColor" на нужный нам цвет
    svgData.replace("currentColor", color.name(QColor::HexRgb));

    // 3. Используем QSvgRenderer для отрисовки
    QByteArray svgBytes = svgData.toUtf8();
    QSvgRenderer renderer(svgBytes);

    if (!renderer.isValid()) {
        qWarning() << "Не удалось отрендерить SVG-файл:" << path;
        return QPixmap(); // Возвращаем пустой Pixmap
    }

    // 4. Создаем QPixmap для отрисовки
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent); // Заполняем прозрачным фоном

    // 5. Рисуем SVG на QPixmap
    QPainter painter(&pixmap);
    renderer.render(&painter);

    return pixmap;
}

QColor ThemeManager::getIconColor() const { return m_iconColor; }
QColor ThemeManager::getColor(const QString& key) const {
    return m_themeColors.value(key, Qt::white);
}
