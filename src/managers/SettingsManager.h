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

    Q_PROPERTY(QString themeName READ getThemeName WRITE setThemeName NOTIFY themeNameChanged)
    Q_PROPERTY(int gridSize READ getGridSize WRITE setGridSize NOTIFY gridSizeChanged)
    Q_PROPERTY(QString canvasPattern READ getCanvasPattern WRITE setCanvasPattern NOTIFY canvasPatternChanged)
    Q_PROPERTY(int labelFontSize READ getLabelFontSize WRITE setLabelFontSize NOTIFY labelFontSizeChanged)
    Q_PROPERTY(int arrangeSpacing READ getArrangeSpacing WRITE setArrangeSpacing NOTIFY arrangeSpacingChanged)

public:
    static SettingsManager* create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    static SettingsManager& instance();

    void loadSettings();
    void saveSettings();

    QString getThemeName() const;
    void setThemeName(const QString& name);

    int getGridSize() const;
    void setGridSize(int size);

    QString getCanvasPattern() const;
    void setCanvasPattern(const QString& pattern);

    int getLabelFontSize() const;
    void setLabelFontSize(int size);

    int getArrangeSpacing() const;
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
