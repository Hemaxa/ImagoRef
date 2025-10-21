#pragma once
#include <QToolButton>
#include <QPropertyAnimation>
#include <QPixmap>

class AnimationManager : public QToolButton {
    Q_OBJECT
    Q_PROPERTY(qreal iconScale READ getIconScale WRITE setIconScale)

public:
    explicit AnimationManager(const QString& iconPath, const QString& toolTip,
                            const QKeySequence& shortcut, QWidget* parent = nullptr);

public slots:
    void updateIconColor(const QColor& color);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void setIconScale(qreal scale);
    qreal getIconScale() const;

    QString m_iconPath;
    QPixmap m_pixmap;
    QPropertyAnimation* m_animation;
    qreal m_currentIconScale;
    const qreal m_dfIconScale = 1.0;
    const qreal m_lgIconScale = 1.15;
};
