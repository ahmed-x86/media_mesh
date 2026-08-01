#include "HomeWindow.h"
#include "Constants.h"
#include "MochaMsgBox.h"
#include "FileInfoBox.h"
#include "ModernConverterWindow.h"
#include "RippleButton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFileDialog>
#include <QStyledItemDelegate>
#include <QAbstractItemView>
#include <QGraphicsDropShadowEffect>
#include <QDesktopServices>
#include <QUrl>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QPropertyAnimation>
#include <QDir>
#include <QProcess>
#include <QMessageBox>

HomeWindow::HomeWindow(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Media Mesh - Menu Extensions");
    setMinimumSize(700, 500);
    
    m_currentGlowPos = QPointF(width() / 2.0, height() / 2.0);
    m_targetGlowPos  = m_currentGlowPos;

    setupUI();
    buildSidebar();
    applyStyle();

    m_glowTimer = new QTimer(this);
    m_glowTimer->setInterval(16);
    connect(m_glowTimer, &QTimer::timeout, this, [this]() {
        m_currentGlowPos += (m_targetGlowPos - m_currentGlowPos) * 0.08;
        update();
    });
    m_glowTimer->start();

    qApp->installEventFilter(this);
}

void HomeWindow::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), Mocha::Base);

    QRadialGradient glow(m_currentGlowPos, 350.0);
    QColor glowColor(Mocha::Accent);
    glowColor.setAlpha(30);
    glow.setColorAt(0.0, glowColor);
    glowColor.setAlpha(0);
    glow.setColorAt(1.0, glowColor);

    painter.setBrush(QBrush(glow));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());
}

bool HomeWindow::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::MouseMove) {
        m_targetGlowPos = mapFromGlobal(static_cast<QMouseEvent*>(event)->globalPosition().toPoint());
    }
    // Auto-close sidebar if clicked outside of it
    if (event->type() == QEvent::MouseButtonPress && m_isSidebarOpen) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->pos().x() > 250) { 
            toggleSidebar();
        }
    }
    return QWidget::eventFilter(watched, event);
}

void HomeWindow::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (m_sidebar) {
        if (m_isSidebarOpen) {
            m_sidebar->setGeometry(0, 0, 250, height());
        } else {
            m_sidebar->setGeometry(-250, 0, 250, height());
        }
    }
}

void HomeWindow::applyStyle() {
    QString qss = QStringLiteral(R"(
        QWidget { font-family: 'Inter', 'Segoe UI', sans-serif; color: %1; }
        
        /* Sidebar Styles */
        QWidget#sidebar { background-color: rgba(49, 50, 68, 0.95); border-right: 1px solid %5; }
        QPushButton#btnSidebarItem { background: transparent; color: %1; text-align: left; padding: 12px 20px; font-weight: bold; font-size: 15px; }
        QPushButton#btnSidebarItem:hover { background: %5; border-radius: 8px; }
        QPushButton#hamburgerBtn { background: transparent; color: %1; padding: 5px; border-radius: 8px; border: none; font-size: 22px; font-weight: bold; }
        QPushButton#hamburgerBtn:hover { background: %5; }
        
        /* Cards & Main UI */
        QWidget#cardWidget {
            background-color: rgba(49, 50, 68, 0.7); border-radius: 16px;
            border: 1px solid rgba(205, 214, 244, 0.1);
        }
        QLabel#titleLabel { font-size: 28px; font-weight: bold; color: %2; margin-bottom: 10px; }
        QLabel#subTitle { font-size: 15px; color: %3; margin-bottom: 20px; }
        QLabel#homeDesc { font-size: 18px; color: %1; font-weight: bold; }
        
        QLineEdit {
            background-color: %4; color: %1; border: 1px solid %5;
            border-radius: 8px; padding: 8px 12px; font-size: 14px;
        }
        QComboBox {
            background-color: %4; color: %1; border: 1px solid %5;
            border-radius: 8px; padding: 8px 12px; font-size: 14px; font-weight: bold;
        }
        QComboBox::drop-down { border: none; width: 30px; }
        QComboBox QAbstractItemView {
            background-color: %4; color: %1; border: 1px solid %5;
            border-radius: 8px; selection-background-color: %2;
            selection-color: %4; outline: none; 
        }
        QComboBox QAbstractItemView::item { min-height: 35px; padding-left: 10px; }
        QComboBox QAbstractItemView::item:selected { background-color: %2; color: %4; border-radius: 4px; }
        
        QScrollBar:vertical { background: transparent; width: 8px; }
        QScrollBar::handle:vertical { background: %5; border-radius: 4px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { background: none; border: none; }
        
        QPushButton#btnBrowse, QPushButton#btnInfo, QPushButton#btnInstallFM {
            background-color: %5; color: %1; border-radius: 8px; padding: 10px 16px; font-weight: bold; font-size: 14px;
        }
        QPushButton#btnBrowse:hover, QPushButton#btnInfo:hover, QPushButton#btnInstallFM:hover { 
            background-color: %2; color: %4; 
        }
        
        QPushButton#btnStart {
            background-color: %2; color: %4; border-radius: 10px; padding: 12px;
            font-size: 16px; font-weight: bold; margin-top: 10px;
        }
        QPushButton#btnStart:hover { background-color: %6; }
        
        QPushButton#btnLink { background-color: transparent; color: %3; font-weight: bold; border: none; padding: 5px; }
        QPushButton#btnLink:hover { color: %2; }
    )").arg(Mocha::Text.name(), Mocha::Accent.name(), Mocha::Subtext.name(),
            Mocha::Base.name(), Mocha::Surface.name(), Mocha::Green.name());
    
    this->setStyleSheet(qss);
}

void HomeWindow::setupUI() {
    // Main Container
    m_mainContent = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(m_mainContent);
    rootLayout->setContentsMargins(20, 15, 20, 15);
    
    auto *windowLayout = new QVBoxLayout(this);
    windowLayout->setContentsMargins(0, 0, 0, 0);
    windowLayout->addWidget(m_mainContent);

    // Top Bar (Hamburger Menu)
    m_topBar = new QWidget(this);
    auto *topRow = new QHBoxLayout(m_topBar);
    topRow->setContentsMargins(0, 0, 0, 0);

    m_hamburgerBtn = new QPushButton("☰");
    m_hamburgerBtn->setObjectName("hamburgerBtn");
    m_hamburgerBtn->setCursor(Qt::PointingHandCursor);
    connect(m_hamburgerBtn, &QPushButton::clicked, this, &HomeWindow::toggleSidebar);

    topRow->addWidget(m_hamburgerBtn);
    topRow->addStretch();
    rootLayout->addWidget(m_topBar);

    // Stacked Widget for Pages
    m_stackedWidget = new QStackedWidget(this);

    // ==========================================
    // Page 1: Home
    // ==========================================
    m_homePage = new QWidget(this);
    auto *homeLayout = new QVBoxLayout(m_homePage);
    homeLayout->setAlignment(Qt::AlignCenter);
    
    auto *homeDesc = new QLabel("This is just a program that adds right-click options\nto Linux file managers for fast media conversion.");
    homeDesc->setObjectName("homeDesc");
    homeDesc->setAlignment(Qt::AlignCenter);
    homeLayout->addWidget(homeDesc);
    m_stackedWidget->addWidget(m_homePage);

    // ==========================================
    // Page 2: Converter (Old Main UI)
    // ==========================================
    m_converterPage = new QWidget(this);
    auto *converterMainLayout = new QVBoxLayout(m_converterPage);
    converterMainLayout->setContentsMargins(20, 20, 20, 0);

    auto *cardWidget = new QWidget(m_converterPage);
    cardWidget->setObjectName("cardWidget");
    auto *cardLayout = new QVBoxLayout(cardWidget);
    cardLayout->setContentsMargins(30, 40, 30, 40);

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 5);
    cardWidget->setGraphicsEffect(shadow);

    auto *titleLbl = new QLabel("Media Mesh Converter");
    titleLbl->setObjectName("titleLabel");
    titleLbl->setAlignment(Qt::AlignCenter);
    
    auto *subLbl = new QLabel("Select a file and format to begin conversion");
    subLbl->setObjectName("subTitle");
    subLbl->setAlignment(Qt::AlignCenter);

    cardLayout->addWidget(titleLbl);
    cardLayout->addWidget(subLbl);

    auto *fileLayout = new QHBoxLayout();
    m_pathInput = new QLineEdit();
    m_pathInput->setPlaceholderText("Path to media file...");
    m_pathInput->setReadOnly(true);
    
    auto *browseBtn = new RippleButton("Browse");
    browseBtn->setObjectName("btnBrowse");
    connect(browseBtn, &QPushButton::clicked, this, [this]() {
        QFileDialog dialog(nullptr, "Select Media File");
        dialog.setNameFilter("Media Files (*.mp4 *.mkv *.webm *.mov *.avi *.mp3 *.aac *.wav *.gif *.jpg *.png *.webp);;All Files (*)");
        if (dialog.exec() == QDialog::Accepted) {
            m_pathInput->setText(dialog.selectedFiles().first());
        }
    });

    auto *infoBtn = new RippleButton("Info");
    infoBtn->setObjectName("btnInfo");
    connect(infoBtn, &QPushButton::clicked, this, [this]() {
        if (m_pathInput->text().isEmpty() || !QFileInfo::exists(m_pathInput->text())) {
            MochaMsgBox::showMsg(this, "Error", "Please select a valid input file first.", true);
            return;
        }
        FileInfoBox::showInfo(this, m_pathInput->text());
    });
    
    fileLayout->addWidget(m_pathInput);
    fileLayout->addWidget(browseBtn);
    fileLayout->addWidget(infoBtn);
    cardLayout->addLayout(fileLayout);

    m_profileCombo = new QComboBox();
    QStringList profiles = {
        "mp4", "mp4_nvenc", "mp4_amd_vaapi", "mkv", "webm", "av1", 
        "davinci_cuda_full", "davinci_amd_full", "prores_cuda_full", 
        "mp3", "aac", "wav", "gif", "jpg", "webp"
    };
    m_profileCombo->addItems(profiles);
    m_profileCombo->setItemDelegate(new QStyledItemDelegate(this));
    
    if (m_profileCombo->view()->parentWidget()) {
        m_profileCombo->view()->parentWidget()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
        m_profileCombo->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground);
    }
    cardLayout->addWidget(m_profileCombo);

    auto *startBtn = new RippleButton("Start Conversion");
    startBtn->setObjectName("btnStart");
    connect(startBtn, &QPushButton::clicked, this, [this]() {
        if (m_pathInput->text().isEmpty() || !QFileInfo::exists(m_pathInput->text())) {
            MochaMsgBox::showMsg(this, "Error", "Please select a valid input file first.", true);
            return;
        }

        QString inPath = m_pathInput->text();
        QString profile = m_profileCombo->currentText();
        QFileInfo fi(inPath);
        QString ext = fi.suffix().toLower();

        bool isInputVideo = MediaCategories::VideoExtensions.contains(ext);
        bool isInputAudio = MediaCategories::AudioExtensions.contains(ext);
        bool isInputImage = MediaCategories::ImageExtensions.contains(ext);

        bool isOutputVideo = MediaCategories::VideoProfiles.contains(profile);
        bool isOutputAudio = MediaCategories::AudioProfiles.contains(profile);
        bool isOutputImage = MediaCategories::ImageProfiles.contains(profile);

        bool valid = false;
        QString errorMsg;

        if (isInputVideo) {
            if (isOutputVideo || isOutputAudio) valid = true;
            else errorMsg = "Videos can only be converted to videos or audios.";
        } else if (isInputAudio) {
            if (isOutputAudio) valid = true;
            else errorMsg = "Audios can only be converted to audios.";
        } else if (isInputImage) {
            if (isOutputImage) valid = true;
            else errorMsg = "Images can only be converted to images.";
        } else {
            valid = true; 
        }

        if (!valid) {
            MochaMsgBox::showMsg(this, "Conversion Not Allowed", errorMsg, true);
            return;
        }

        auto *converter = new ModernConverterWindow(m_profileCombo->currentText(), m_pathInput->text());
        converter->show();
        this->close();
    });
    
    cardLayout->addWidget(startBtn);
    converterMainLayout->addWidget(cardWidget);
    converterMainLayout->addStretch();
    m_stackedWidget->addWidget(m_converterPage);


    // ==========================================
    // Page 3: Install File Manager Extensions
    // ==========================================
    m_installPage = new QWidget(this);
    auto *installLayout = new QVBoxLayout(m_installPage);
    installLayout->setContentsMargins(20, 20, 20, 0);

    auto *installCard = new QWidget(m_installPage);
    installCard->setObjectName("cardWidget");
    auto *installCardLayout = new QVBoxLayout(installCard);
    
    auto *installShadow = new QGraphicsDropShadowEffect(this);
    installShadow->setBlurRadius(20);
    installShadow->setColor(QColor(0, 0, 0, 80));
    installShadow->setOffset(0, 5);
    installCard->setGraphicsEffect(installShadow);

    auto *installTitle = new QLabel("Supported File Managers");
    installTitle->setObjectName("titleLabel");
    installTitle->setAlignment(Qt::AlignCenter);
    installCardLayout->addWidget(installTitle);

    auto *installGrid = new QGridLayout();
    installGrid->setSpacing(15);
    
    // Struct for defining file manager configs
    struct FMConfig { QString name; QString outputDir; QString installPath; QString restartCmd; };
    QList<FMConfig> fms = {
        {"Nautilus", "nautilus", QDir::homePath() + "/.local/share/nautilus-python/extensions", "nautilus -q"},
        {"Nemo",     "nemo",     QDir::homePath() + "/.local/share/nemo-python/extensions",     "nemo -q"},
        {"Dolphin",  "dolphin",  QDir::homePath() + "/.local/share/kio/servicemenus",           "killall dolphin"},
        {"Thunar",   "thunar",   QDir::homePath() + "/.local/share/thunarx-python/extensions",  "thunar -q"},
        {"PCManFM",  "pcmanfm",  QDir::homePath() + "/.local/share/file-manager/actions",       "pcmanfm --quit"},
        {"Caja",     "caja",     QDir::homePath() + "/.local/share/caja-python/extensions",     "caja -q"}
    };

    int row = 0; int col = 0;
    for (const auto &fm : fms) {
        auto *btn = new RippleButton(fm.name);
        btn->setObjectName("btnInstallFM");
        btn->setFixedHeight(50);
        connect(btn, &QPushButton::clicked, this, [this, fm]() {
            installExtension(fm.outputDir, fm.installPath, fm.restartCmd);
        });
        
        installGrid->addWidget(btn, row, col);
        col++;
        if (col > 1) { col = 0; row++; }
    }
    
    installCardLayout->addLayout(installGrid);
    installLayout->addWidget(installCard);
    installLayout->addStretch();
    m_stackedWidget->addWidget(m_installPage);

    // ==========================================
    // Wrap up Main Layout
    // ==========================================
    rootLayout->addWidget(m_stackedWidget);

    auto *footerLayout = new QHBoxLayout();
    footerLayout->setAlignment(Qt::AlignCenter);
    footerLayout->setSpacing(20);

    auto *githubBtn = new QPushButton("GitHub");
    githubBtn->setObjectName("btnLink");
    githubBtn->setCursor(Qt::PointingHandCursor);
    connect(githubBtn, &QPushButton::clicked, [](){ QDesktopServices::openUrl(QUrl("https://github.com/ahmed-x86/media_mesh")); });

    auto *issueBtn = new QPushButton("Report Issue");
    issueBtn->setObjectName("btnLink");
    issueBtn->setCursor(Qt::PointingHandCursor);
    connect(issueBtn, &QPushButton::clicked, [](){ QDesktopServices::openUrl(QUrl("https://github.com/ahmed-x86/media_mesh/issues")); });

    footerLayout->addWidget(githubBtn);
    footerLayout->addWidget(issueBtn);
    rootLayout->addLayout(footerLayout);
}

void HomeWindow::buildSidebar() {
    m_sidebar = new QWidget(this);
    m_sidebar->setObjectName("sidebar");
    m_sidebar->setGeometry(-250, 0, 250, height()); 

    auto* layout = new QVBoxLayout(m_sidebar);
    layout->setContentsMargins(10, 60, 10, 20);
    layout->setSpacing(10);

    m_btnNavHome = new RippleButton("🏠  Home");
    m_btnNavHome->setObjectName("btnSidebarItem");
    
    m_btnNavConvert = new RippleButton("🔄  Convert Media");
    m_btnNavConvert->setObjectName("btnSidebarItem");

    m_btnNavInstall = new RippleButton("🔌  Install Extensions");
    m_btnNavInstall->setObjectName("btnSidebarItem");
    
    layout->addWidget(m_btnNavHome);
    layout->addWidget(m_btnNavConvert);
    layout->addWidget(m_btnNavInstall);
    layout->addStretch();

    connect(m_btnNavHome, &QPushButton::clicked, this, [this]{
        m_stackedWidget->setCurrentIndex(0);
        toggleSidebar();
    });
    connect(m_btnNavConvert, &QPushButton::clicked, this, [this]{
        m_stackedWidget->setCurrentIndex(1);
        toggleSidebar();
    });
    connect(m_btnNavInstall, &QPushButton::clicked, this, [this]{
        m_stackedWidget->setCurrentIndex(2);
        toggleSidebar();
    });
}

void HomeWindow::toggleSidebar() {
    m_isSidebarOpen = !m_isSidebarOpen;
    
    if (m_isSidebarOpen) {
        m_sidebar->raise(); 
    }

    auto* anim = new QPropertyAnimation(m_sidebar, "geometry");
    anim->setDuration(300);
    anim->setEasingCurve(QEasingCurve::OutQuint);

    if (m_isSidebarOpen) {
        anim->setStartValue(QRect(-250, 0, 250, height()));
        anim->setEndValue(QRect(0, 0, 250, height()));
    } else {
        anim->setStartValue(QRect(0, 0, 250, height()));
        anim->setEndValue(QRect(-250, 0, 250, height()));
    }
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void HomeWindow::installExtension(const QString &fmName, const QString &destPath, const QString &restartCmd) {
    QString sourceDir = QDir::currentPath() + "/output/" + fmName;
    if (!QDir(sourceDir).exists()) {
        MochaMsgBox::showMsg(this, "Error", "Output directory for this file manager not found.\nPlease run the mesh script first.", true);
        return;
    }

    QDir().mkpath(destPath);

    QString cpCommand = QString("cp -r %1/* %2/").arg(sourceDir, destPath);
    QProcess::execute("bash", {"-c", cpCommand});

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Restart Required");
    msgBox.setText("Installation successful!\nYou must restart the file manager to see the changes.\nDo you want to restart it now?");
    
    QString msgQss = QStringLiteral(R"(
        QMessageBox { background-color: %1; color: %2; }
        QLabel { color: %2; font-size: 14px; }
        QPushButton { background-color: %3; color: %2; border-radius: 6px; padding: 6px 15px; font-weight: bold; }
        QPushButton:hover { background-color: %4; }
    )").arg(Mocha::Base.name(), Mocha::Text.name(), Mocha::Surface.name(), Mocha::Hover.name());
    msgBox.setStyleSheet(msgQss);

    QPushButton *okBtn = msgBox.addButton("OK", QMessageBox::AcceptRole);
    msgBox.addButton("Later", QMessageBox::RejectRole);

    msgBox.exec();

    if (msgBox.clickedButton() == okBtn) {
        QProcess::startDetached("bash", {"-c", restartCmd});
    }
}