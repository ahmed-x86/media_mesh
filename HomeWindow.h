#pragma once
#include <QWidget>
#include <QPointF>
#include <QTimer>
#include <QLineEdit>
#include <QComboBox>
#include <QStackedWidget>
#include <QPushButton>

class RippleButton;

class HomeWindow : public QWidget {
    Q_OBJECT
public:
    HomeWindow(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QPointF m_currentGlowPos, m_targetGlowPos;
    QTimer* m_glowTimer;

    QLineEdit *m_pathInput;
    QComboBox *m_profileCombo;
    QComboBox *m_hwAccelCombo;

    bool m_isSidebarOpen = false;
    QWidget *m_sidebar;
    QWidget *m_mainContent;
    QWidget *m_topBar;
    QStackedWidget *m_stackedWidget;
    QWidget *m_homePage, *m_converterPage, *m_installPage;
    QPushButton *m_hamburgerBtn;
    
    RippleButton *m_btnNavHome, *m_btnNavConvert, *m_btnNavInstall;

    void applyStyle();
    void setupUI();
    void buildSidebar();
    void toggleSidebar();
    void installExtension(const QString &fmName, const QString &destPath, const QString &restartCmd);
};