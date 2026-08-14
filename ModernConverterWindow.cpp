#include "ModernConverterWindow.h"
#include <signal.h>
#include <sys/types.h>
#include "Constants.h"
#include "MochaMsgBox.h"
#include "RippleButton.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QRegularExpression>
#include <QApplication>
#include <QTime>

ModernConverterWindow::ModernConverterWindow(const QString &profile, const QString &inputFile, const QString &hwDevice, QWidget *parent)
    : QWidget(parent), m_profile(profile), m_inputFile(inputFile), m_hwDevice(hwDevice)
{
    setWindowTitle(QString("Media Mesh - %1").arg(m_profile.toUpper()));
    setMinimumSize(600, 400);
    
    m_currentGlowPos = QPointF(width() / 2.0, height() / 2.0);
    m_targetGlowPos  = m_currentGlowPos;

    m_extension = getOutputExtension(m_profile);
    QFileInfo fi(m_inputFile);
    m_outputFile = fi.absolutePath() + QDir::separator() + fi.completeBaseName() + "." + m_extension;

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
    
    // جلب مدة الفيديو لعمل نسبة مئوية دقيقة
    fetchMediaDuration();
}

ModernConverterWindow::~ModernConverterWindow() {
    if (m_ffmpegProcess && m_ffmpegProcess->state() == QProcess::Running) {
        m_ffmpegProcess->kill();
        m_ffmpegProcess->waitForFinished();
    }
}

// حل مشكلة Davinci وغيرها بالامتدادات
QString ModernConverterWindow::getOutputExtension(const QString &profile) {
    if (profile.contains("mp4")) return "mp4";
    if (profile.contains("mkv")) return "mkv";
    if (profile.contains("webm")) return "webm";
    if (profile.contains("davinci") || profile.contains("prores")) return "mov";
    if (profile.contains("av1")) return "mkv";
    if (profile.contains("jpg") || profile.contains("jpeg")) return "jpg";
    if (profile.contains("gif")) return "gif";
    if (profile.contains("webp")) return "webp";
    if (profile.contains("mp3") || profile.contains("aac") || profile.contains("wav")) return profile;
    return "mp4"; 
}

void ModernConverterWindow::applyStyle() {
    QString qss = QStringLiteral(R"(
        QWidget { font-family: 'Inter', 'Segoe UI', sans-serif; color: %1; }
        QProgressBar {
            background-color: %2; border-radius: 10px; text-align: center;
            color: %1; font-weight: bold; font-size: 14px; height: 20px; border: 1px solid %3;
        }
        QProgressBar::chunk { background-color: %4; border-radius: 9px; }
        QWidget#cardWidget {
            background-color: rgba(49, 50, 68, 0.6); border-radius: 16px;
            border: 1px solid rgba(205, 214, 244, 0.1);
        }
        QLabel#titleLabel { font-size: 24px; font-weight: bold; color: %4; }
        QLabel#fileLabel { font-size: 14px; color: %5; }
        QLabel#statVal { font-size: 20px; font-weight: bold; color: %6; }
        QLabel#statKey { font-size: 13px; font-weight: bold; color: %5; text-transform: uppercase; }
        QPushButton#btnCancel { 
            background-color: transparent; color: %7; border: 2px solid %7; 
            border-radius: 8px; padding: 10px 24px; font-size: 15px; font-weight: bold; 
        }
        QPushButton#btnCancel:hover { background-color: %7; color: %8; }
    )").arg(Mocha::Text.name(), Mocha::Surface.name(), Mocha::Hover.name(), 
            Mocha::Accent.name(), Mocha::Subtext.name(), Mocha::Green.name(), 
            Mocha::Red.name(), Mocha::Base.name());
    this->setStyleSheet(qss);
}

void ModernConverterWindow::setupUI() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setAlignment(Qt::AlignCenter);

    auto *cardWidget = new QWidget(this);
    cardWidget->setObjectName("cardWidget");
    auto *cardLayout = new QVBoxLayout(cardWidget);
    cardLayout->setContentsMargins(30, 30, 30, 30);
    cardLayout->setSpacing(25);

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 5);
    cardWidget->setGraphicsEffect(shadow);

    auto *titleLbl = new QLabel(QString("Encoding: %1 [%2]").arg(m_profile.toUpper(), m_hwDevice.toUpper()));
    titleLbl->setObjectName("titleLabel");
    titleLbl->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(titleLbl);

    QFileInfo fi(m_inputFile);
    auto *fileLbl = new QLabel(fi.fileName());
    fileLbl->setObjectName("fileLabel");
    fileLbl->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(fileLbl);

    auto *progressLayout = new QHBoxLayout();
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setFormat("0%");
    
    m_btnPauseResume = new RippleButton("Pause");
    m_btnPauseResume->setObjectName("btnCancel"); 
    m_btnPauseResume->setFixedSize(100, 35);
    connect(m_btnPauseResume, &QPushButton::clicked, this, &ModernConverterWindow::togglePauseResume);
    
    progressLayout->addWidget(m_progressBar);
    progressLayout->addWidget(m_btnPauseResume);
    cardLayout->addLayout(progressLayout);

    auto *gridLayout = new QGridLayout();
    gridLayout->setSpacing(20);
    auto createStatBox = [&](const QString &title, QLabel *&valLabel) {
        auto *vbox = new QVBoxLayout();
        vbox->setSpacing(5);
        auto *t = new QLabel(title); t->setObjectName("statKey"); t->setAlignment(Qt::AlignCenter);
        valLabel = new QLabel("--"); valLabel->setObjectName("statVal"); valLabel->setAlignment(Qt::AlignCenter);
        vbox->addWidget(t); vbox->addWidget(valLabel);
        return vbox;
    };
    gridLayout->addLayout(createStatBox("Speed", m_valSpeed), 0, 0);
    gridLayout->addLayout(createStatBox("Elapsed", m_valElapsed), 0, 1);
    gridLayout->addLayout(createStatBox("ETA", m_valETA), 1, 0);
    gridLayout->addLayout(createStatBox("Frame", m_valFrame), 1, 1);
    cardLayout->addLayout(gridLayout);

    auto *cancelBtn = new RippleButton("Cancel Conversion");
    cancelBtn->setObjectName("btnCancel");
    cancelBtn->setFixedSize(200, 45);
    connect(cancelBtn, &QPushButton::clicked, this, &ModernConverterWindow::cancelConversion);
    
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(cancelBtn, 0, Qt::AlignCenter);
    cardLayout->addLayout(btnLayout);

    mainLayout->addWidget(cardWidget);
}

void ModernConverterWindow::fetchMediaDuration() {
    QProcess ffprobe;
    QStringList args = {"-v", "error", "-show_entries", "format=duration", 
                        "-of", "default=noprint_wrappers=1:nokey=1", m_inputFile};
    ffprobe.start("ffprobe", args);
    ffprobe.waitForFinished();
    m_totalDuration = ffprobe.readAllStandardOutput().trimmed().toDouble();
    startFFmpeg();
}

void ModernConverterWindow::startFFmpeg() {
    m_ffmpegProcess = new QProcess(this);
    connect(m_ffmpegProcess, &QProcess::readyReadStandardError, this, &ModernConverterWindow::onFfmpegReadyRead);
    connect(m_ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ModernConverterWindow::onFfmpegFinished);

    QStringList args;
    args << "-y" << "-hide_banner"; 

    // 1. إعدادات تسريع العتاد للقراءة (Decoding)
    if (m_hwDevice == "cuda") {
        args << "-hwaccel" << "cuda";
    } else if (m_hwDevice == "vaapi") {
        args << "-hwaccel" << "vaapi" << "-hwaccel_device" << "/dev/dri/renderD128" << "-hwaccel_output_format" << "vaapi";
    }

    args << "-i" << m_inputFile;

    // 2. إعدادات المخرج (Encoding) مبنية على اختيار المستخدم ديناميكياً
    if (m_profile.contains("mp4")) {
        if (m_hwDevice == "cuda") args << "-c:v" << "h264_nvenc" << "-preset" << "p4" << "-c:a" << "aac" << "-movflags" << "+faststart";
        else if (m_hwDevice == "vaapi") args << "-vf" << "format=nv12,hwupload" << "-c:v" << "h264_vaapi" << "-c:a" << "aac" << "-movflags" << "+faststart";
        else args << "-c:v" << "libx264" << "-c:a" << "aac" << "-movflags" << "+faststart";
    } 
    else if (m_profile.contains("mkv")) {
        if (m_hwDevice == "cuda") args << "-c:v" << "hevc_nvenc" << "-preset" << "p4" << "-c:a" << "aac";
        else if (m_hwDevice == "vaapi") args << "-vf" << "format=nv12,hwupload" << "-c:v" << "hevc_vaapi" << "-c:a" << "aac";
        else args << "-c:v" << "libx264" << "-c:a" << "aac";
    }
    else if (m_profile.contains("webm")) {
        args << "-c:v" << "libvpx-vp9" << "-b:v" << "0" << "-crf" << "30" << "-c:a" << "libopus";
    }
    else if (m_profile.contains("davinci")) {
        args << "-pix_fmt" << "yuv422p" << "-c:v" << "dnxhd" << "-profile:v" << "dnxhr_sq" << "-c:a" << "pcm_s16le";
    }
    else if (m_profile.contains("prores")) {
        args << "-profile:v" << "2" << "-vendor" << "apl0" << "-bits_per_mb" << "8000" << "-pix_fmt" << "yuv422p10le" << "-c:v" << "prores_ks" << "-c:a" << "pcm_s16le";
    }
    else if (m_profile.contains("av1")) {
        args << "-c:v" << "libsvtav1" << "-preset" << "6" << "-crf" << "30" << "-c:a" << "libopus";
    }
    else if (m_profile.contains("mp3")) { args << "-vn" << "-c:a" << "libmp3lame" << "-b:a" << "192k"; }
    else if (m_profile.contains("aac")) { args << "-vn" << "-c:a" << "aac" << "-b:a" << "192k"; }
    else if (m_profile.contains("wav")) { args << "-vn" << "-c:a" << "pcm_s16le"; }
    else if (m_profile.contains("gif")) { args << "-vf" << "fps=15,scale=720:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse"; }
    else if (m_profile.contains("webp")) { args << "-c:v" << "libwebp" << "-q:v" << "75"; }
    else if (m_profile.contains("jpg") || m_profile.contains("bmp")) { args << "-q:v" << "2"; }

    args << m_outputFile;

    m_elapsedTimer.start();
    m_ffmpegProcess->start("ffmpeg", args);
}

void ModernConverterWindow::onFfmpegReadyRead() {
    QString output = m_ffmpegProcess->readAllStandardError();
    
    QRegularExpression timeRegex(R"(time=(\d+):(\d+):(\d+\.\d+))");
    QRegularExpressionMatch timeMatch = timeRegex.match(output);
    
    QRegularExpression speedRegex(R"(speed=\s*([\d\.]+)x)");
    QRegularExpressionMatch speedMatch = speedRegex.match(output);

    QRegularExpression frameRegex(R"(frame=\s*(\d+))");
    QRegularExpressionMatch frameMatch = frameRegex.match(output);

    if (timeMatch.hasMatch()) {
        double hours = timeMatch.captured(1).toDouble();
        double minutes = timeMatch.captured(2).toDouble();
        double seconds = timeMatch.captured(3).toDouble();
        
        double currentTime = (hours * 3600) + (minutes * 60) + seconds;
        
        if (m_totalDuration > 0) {
            int percent = qMin(100, static_cast<int>((currentTime / m_totalDuration) * 100));
            m_progressBar->setValue(percent);
            m_progressBar->setFormat(QString("%1%").arg(percent));
            
            qint64 elapsedSecs = m_elapsedTimer.elapsed() / 1000;
            if (percent > 0) {
                qint64 totalEstimated = (elapsedSecs * 100) / percent;
                qint64 remainingSecs = totalEstimated - elapsedSecs;
                QTime etaTime(0, 0, 0);
                etaTime = etaTime.addSecs(remainingSecs);
                m_valETA->setText(etaTime.toString("HH:mm:ss"));
            }
        }
    }

    if (speedMatch.hasMatch()) { m_valSpeed->setText(speedMatch.captured(1) + "x"); }
    if (frameMatch.hasMatch()) { m_valFrame->setText(frameMatch.captured(1)); }

    QTime elapsed(0, 0, 0);
    elapsed = elapsed.addMSecs(m_elapsedTimer.elapsed());
    m_valElapsed->setText(elapsed.toString("HH:mm:ss"));
}

void ModernConverterWindow::onFfmpegFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        m_progressBar->setValue(100);
        m_progressBar->setFormat("100%");
        m_valETA->setText("00:00:00");
        sendNotification("Conversion Complete", "Successfully converted:\n" + m_outputFile, false);
        MochaMsgBox::showMsg(this, "Success", "Conversion completed successfully!\n\n" + m_outputFile, false);
    } else {
        sendNotification("Conversion Failed", "The conversion process was canceled or failed.", true);
        MochaMsgBox::showMsg(this, "Error", "Conversion failed or was canceled.", true);
    }
    close();
}

void ModernConverterWindow::cancelConversion() {
    if (m_ffmpegProcess && m_ffmpegProcess->state() == QProcess::Running) {
        m_ffmpegProcess->kill();
    }
    close();
}

void ModernConverterWindow::sendNotification(const QString &title, const QString &message, bool isError) {
    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.freedesktop.Notifications", 
        "/org/freedesktop/Notifications", 
        "org.freedesktop.Notifications", 
        "Notify"
    );

    QString iconName = isError ? "dialog-error" : "video-x-generic";

    QList<QVariant> args;
    args.append("Media Mesh");
    args.append(0U);
    args.append(iconName);
    args.append(title);
    args.append(message);
    args.append(QStringList());
    args.append(QVariantMap());
    args.append(5000);

    msg.setArguments(args);
    QDBusConnection::sessionBus().asyncCall(msg);
}

void ModernConverterWindow::togglePauseResume() {
    if (!m_ffmpegProcess || m_ffmpegProcess->state() != QProcess::Running) return;

    qint64 pid = m_ffmpegProcess->processId();
    
    if (pid > 0) {
        if (!m_isPaused) {
            kill(pid, SIGSTOP); // تجميد العملية بالكامل على مستوى النظام
            m_btnPauseResume->setText("Resume");
            m_isPaused = true;
        } else {
            kill(pid, SIGCONT); // استكمال العملية
            m_btnPauseResume->setText("Pause");
            m_isPaused = false;
        }
    }
}

void ModernConverterWindow::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), Mocha::Base);

    QRadialGradient glow(m_currentGlowPos, 300.0);
    QColor glowColor(Mocha::Accent);
    glowColor.setAlpha(25);
    glow.setColorAt(0.0, glowColor);
    glowColor.setAlpha(0);
    glow.setColorAt(1.0, glowColor);

    painter.setBrush(QBrush(glow));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());
}

bool ModernConverterWindow::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::MouseMove) {
        m_targetGlowPos = mapFromGlobal(static_cast<QMouseEvent*>(event)->globalPosition().toPoint());
    }
    return QWidget::eventFilter(watched, event);
}