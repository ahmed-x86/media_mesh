#pragma once
#include <QPushButton>

class RippleButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(qreal rippleRadius READ rippleRadius WRITE setRippleRadius)
    Q_PROPERTY(qreal rippleOpacity READ rippleOpacity WRITE setRippleOpacity)

public:
    explicit RippleButton(const QString &text, QWidget* parent = nullptr);
    
    qreal rippleRadius() const;
    void setRippleRadius(qreal r);
    
    qreal rippleOpacity() const;
    void setRippleOpacity(qreal o);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QPointF m_ripplePos;
    qreal m_rippleRadius = 0;
    qreal m_rippleOpacity = 0;
};