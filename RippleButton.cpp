#include "RippleButton.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

RippleButton::RippleButton(const QString &text, QWidget* parent) : QPushButton(text, parent) {
    setCursor(Qt::PointingHandCursor);
}

qreal RippleButton::rippleRadius() const { return m_rippleRadius; }
void RippleButton::setRippleRadius(qreal r) { m_rippleRadius = r; update(); }

qreal RippleButton::rippleOpacity() const { return m_rippleOpacity; }
void RippleButton::setRippleOpacity(qreal o) { m_rippleOpacity = o; update(); }

void RippleButton::mousePressEvent(QMouseEvent* event) {
    QPushButton::mousePressEvent(event);
    m_ripplePos = event->position();
    m_rippleRadius = 0;
    m_rippleOpacity = 0.2;

    auto* radiusAnim = new QPropertyAnimation(this, "rippleRadius");
    radiusAnim->setDuration(350);
    radiusAnim->setStartValue(0);
    radiusAnim->setEndValue(width() * 1.5);
    radiusAnim->setEasingCurve(QEasingCurve::OutQuad);

    auto* opacityAnim = new QPropertyAnimation(this, "rippleOpacity");
    opacityAnim->setDuration(350);
    opacityAnim->setStartValue(0.2);
    opacityAnim->setEndValue(0.0);

    auto* group = new QParallelAnimationGroup(this);
    group->addAnimation(radiusAnim);
    group->addAnimation(opacityAnim);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void RippleButton::paintEvent(QPaintEvent* event) {
    QPushButton::paintEvent(event);
    if (m_rippleRadius > 0 && m_rippleOpacity > 0) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addRoundedRect(rect(), 8, 8);
        painter.setClipPath(path);
        painter.setBrush(QColor(0, 0, 0, static_cast<int>(m_rippleOpacity * 255)));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(m_ripplePos, m_rippleRadius, m_rippleRadius);
    }
}