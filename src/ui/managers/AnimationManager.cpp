#include "AnimationManager.h"
#include "ThemeManager.h"

#include <QPainter>
#include <QPaintEvent>

AnimationManager::AnimationManager(const QString& iconPath, const QString& toolTip,
                               const QKeySequence& shortcut, QWidget* parent)
    : QToolButton(parent), m_iconPath(iconPath)
{
    setToolTip(toolTip);
    setShortcut(shortcut);
    setFixedSize(38, 38); // Немного уменьшил для ImagoRef
    setObjectName("AnimationManager");

    m_currentIconScale = m_dfIconScale;
    QColor iconColor = ThemeManager::instance().getIconColor();
    QSize iconSize(22, 22);

    m_pixmap = ThemeManager::instance().colorizeSvg(iconPath, iconColor, iconSize);

    m_animation = new QPropertyAnimation(this, "iconScale", this);
    m_animation->setDuration(100);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
}

void AnimationManager::updateIconColor(const QColor& color) {
    QSize iconSize(22, 22);
    m_pixmap = ThemeManager::instance().colorizeSvg(m_iconPath, color, iconSize);
    update();
}

void AnimationManager::enterEvent(QEnterEvent* event) {
    m_animation->stop();
    m_animation->setEndValue(m_lgIconScale);
    m_animation->start();
    QToolButton::enterEvent(event);
}

void AnimationManager::leaveEvent(QEvent* event) {
    m_animation->stop();
    m_animation->setEndValue(m_dfIconScale);
    m_animation->start();
    QToolButton::leaveEvent(event);
}

void AnimationManager::paintEvent(QPaintEvent* event) {
    QToolButton::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    int scaledSize = static_cast<int>(m_pixmap.width() * m_currentIconScale);
    int x = (width() - scaledSize) / 2;
    int y = (height() - scaledSize) / 2;

    painter.drawPixmap(x, y, scaledSize, scaledSize, m_pixmap);
}

void AnimationManager::setIconScale(qreal scale) {
    if (m_currentIconScale != scale) {
        m_currentIconScale = scale;
        update();
    }
}

qreal AnimationManager::getIconScale() const {
    return m_currentIconScale;
}
