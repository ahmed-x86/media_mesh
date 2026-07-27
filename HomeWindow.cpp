#include "HomeWindow.h"
#include "Constants.h"
#include "MochaMsgBox.h"
#include "FileInfoBox.h"
#include "ModernConverterWindow.h"
#include "RippleButton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
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

HomeWindow::HomeWindow(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Media Mesh - File Converter");
    setMinimumSize(600, 450);
    
    m_currentGlowPos = QPointF(width() / 2.0, height() / 2.0);
    m_targetGlowPos  = m_currentGlowPos;

    setupUI();
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
    return QWidget::eventFilter(watched, event);
}

void HomeWindow::applyStyle() {
    QString qss = QStringLiteral(R"(
        QWidget { font-family: 'Inter', 'Segoe UI', sans-serif; color: %1; }
        QWidget#cardWidget {
            background-color: rgba(49, 50, 68, 0.7); border-radius: 16px;
            border: 1px solid rgba(205, 214, 244, 0.1);
        }
        QLabel#titleLabel { font-size: 28px; font-weight: bold; color: %2; margin-bottom: 10px; }
        QLabel#subTitle { font-size: 14px; color: %3; margin-bottom: 20px; }
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
            background-color: %4;
            color: %1;
            border: 1px solid %5;
            border-radius: 8px;
            selection-background-color: %2;
            selection-color: %4;
            outline: none; 
        }
        QComboBox QAbstractItemView::item {
            min-height: 35px;
            padding-left: 10px;
        }
        QComboBox QAbstractItemView::item:selected {
            background-color: %2;
            color: %4;
            border-radius: 4px;
        }
        QScrollBar:vertical { background: transparent; width: 8px; }
        QScrollBar::handle:vertical { background: %5; border-radius: 4px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { background: none; border: none; }
        QPushButton#btnBrowse {
            background-color: %5; color: %1; border-radius: 8px; padding: 8px 16px; font-weight: bold;
        }
        QPushButton#btnBrowse:hover { background-color: %2; color: %4; }
        QPushButton#btnInfo {
            background-color: %5; color: %1; border-radius: 8px; padding: 8px 16px; font-weight: bold;
        }
        QPushButton#btnInfo:hover { background-color: %2; color: %4; }
        QPushButton#btnStart {
            background-color: %2; color: %4; border-radius: 10px; padding: 12px;
            font-size: 16px; font-weight: bold; margin-top: 10px;
        }
        QPushButton#btnStart:hover { background-color: %6; }
        QPushButton#btnLink {
            background-color: transparent; color: %3; font-weight: bold; border: none; padding: 5px;
        }
        QPushButton#btnLink:hover { color: %2; }
    )").arg(Mocha::Text.name(), Mocha::Accent.name(), Mocha::Subtext.name(),
            Mocha::Base.name(), Mocha::Surface.name(), Mocha::Green.name());
    
    this->setStyleSheet(qss);
}

void HomeWindow::setupUI() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 20);

    auto *cardWidget = new QWidget(this);
    cardWidget->setObjectName("cardWidget");
    auto *cardLayout = new QVBoxLayout(cardWidget);
    cardLayout->setContentsMargins(30, 40, 30, 40);

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 5);
    cardWidget->setGraphicsEffect(shadow);

    auto *titleLbl = new QLabel("Media Mesh");
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
        dialog.setNameFilter("Media Files (*.mp4 *.mkv *.webm *.mov *.avi *.mp3 *.aac *.gif *.jpg *.png *.webp);;All Files (*)");
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
        "mp3", "aac", "gif", "jpg", "webp"
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
            if (isOutputVideo || isOutputAudio) {
                valid = true;
            } else {
                errorMsg = "Videos can only be converted to videos or audios.";
            }
        } else if (isInputAudio) {
            if (isOutputAudio) {
                valid = true;
            } else {
                errorMsg = "Audios can only be converted to audios.";
            }
        } else if (isInputImage) {
            if (isOutputImage) {
                valid = true;
            } else {
                errorMsg = "Images can only be converted to images.";
            }
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

    mainLayout->addWidget(cardWidget);
    mainLayout->addStretch();

    auto *footerLayout = new QHBoxLayout();
    footerLayout->setAlignment(Qt::AlignCenter);
    footerLayout->setSpacing(20);

    auto *githubBtn = new QPushButton("GitHub");
    githubBtn->setObjectName("btnLink");
    githubBtn->setCursor(Qt::PointingHandCursor);
    connect(githubBtn, &QPushButton::clicked, [](){
        QDesktopServices::openUrl(QUrl("https://github.com/ahmed-x86/media_mesh"));
    });

    auto *issueBtn = new QPushButton("Report Issue");
    issueBtn->setObjectName("btnLink");
    issueBtn->setCursor(Qt::PointingHandCursor);
    connect(issueBtn, &QPushButton::clicked, [](){
        QDesktopServices::openUrl(QUrl("https://github.com/ahmed-x86/media_mesh/issues"));
    });

    footerLayout->addWidget(githubBtn);
    footerLayout->addWidget(issueBtn);
    mainLayout->addLayout(footerLayout);
}