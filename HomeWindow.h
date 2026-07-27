#pragma once
#include <QWidget>
#include <QPointF>
#include <QTimer>
#include <QLineEdit>
#include <QComboBox>

class HomeWindow : public QWidget {
    Q_OBJECT
public:
    HomeWindow(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QPointF m_currentGlowPos, m_targetGlowPos;
    QTimer* m_glowTimer;
    QLineEdit *m_pathInput;
    QComboBox *m_profileCombo;

    void applyStyle();
    void setupUI();
};