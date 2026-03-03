#pragma once

#include <QObject>
#include <QSettings>
#include <QString>
#include <QtQml/qqml.h>

/**
 * @brief SettingsManager - синглтон для управления настройками приложения.
 * Адаптирован для доступа из QML через Q_PROPERTY.
 */
class SettingsManager : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString themeName READ themeName WRITE setThemeName NOTIFY themeNameChanged)
    Q_PROPERTY(int gridSize READ gridSize WRITE setGridSize NOTIFY gridSizeChanged)
    Q_PROPERTY(QString canvasPattern READ canvasPattern WRITE setCanvasPattern NOTIFY canvasPatternChanged)
    Q_PROPERTY(int labelFontSize READ labelFontSize WRITE setLabelFontSize NOTIFY labelFontSizeChanged)
    Q_PROPERTY(int arrangeSpacing READ arrangeSpacing WRITE setArrangeSpacing NOTIFY arrangeSpacingChanged)

public:
    static SettingsManager* create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    static SettingsManager& instance();

    void loadSettings();
    void saveSettings();

    QString themeName() const;
    void setThemeName(const QString& name);

    int gridSize() const;
    void setGridSize(int size);

    QString canvasPattern() const;
    void setCanvasPattern(const QString& pattern);

    int labelFontSize() const;
    void setLabelFontSize(int size);

    int arrangeSpacing() const;
    void setArrangeSpacing(int spacing);

signals:
    void themeNameChanged();
    void gridSizeChanged();
    void canvasPatternChanged();
    void labelFontSizeChanged();
    void arrangeSpacingChanged();

private:
    explicit SettingsManager(QObject* parent = nullptr);
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    QSettings m_settings;
    QString m_themeName;
    int m_gridSize;
    QString m_canvasPattern;
    int m_labelFontSize;
    int m_arrangeSpacing;
};
