#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QProcess>
#include <QElapsedTimer>
#include <QRegularExpression>
#include <QFileInfo>
#include <QTime>
#include <QDir>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QTimer>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsDropShadowEffect>
#include <QDesktopServices>
#include <QUrl>
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QFileDialog>
#include <QStyledItemDelegate>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QTextEdit>
#include <QStringList>

namespace MediaCategories {
    const QStringList ImageExtensions = {"jpg", "jpeg", "png", "bmp", "webp", "gif", "ico"};
    const QStringList VideoExtensions = {"mp4", "mkv", "webm", "mov", "avi", "flv"};
    const QStringList AudioExtensions = {"mp3", "aac", "wav", "flac", "ogg", "m4a"};

    const QStringList ImageProfiles = {"gif", "jpg", "jpeg", "webp", "ico", "bmp"};
    const QStringList VideoProfiles = {"mp4", "mp4_nvenc", "mp4_amd_vaapi", "mkv", "webm", "av1", "davinci_cuda_full", "davinci_amd_full", "prores_cuda_full"};
    const QStringList AudioProfiles = {"mp3", "aac"};
}

namespace Mocha {
    static const QColor Base      (0x1e, 0x1e, 0x2e);
    static const QColor Surface   (0x31, 0x32, 0x44);
    static const QColor Text      (0xcd, 0xd6, 0xf4);
    static const QColor Subtext   (0xa6, 0xad, 0xc8);
    static const QColor Accent    (0xcb, 0xa6, 0xf7);
    static const QColor Green     (0xa6, 0xe3, 0xa1);
    static const QColor Red       (0xf3, 0x8b, 0xa8);
    static const QColor Yellow    (0xf9, 0xe2, 0xaf);
    static const QColor Hover     (0x45, 0x47, 0x5a);
}

class RippleButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(qreal rippleRadius READ rippleRadius WRITE setRippleRadius)
    Q_PROPERTY(qreal rippleOpacity READ rippleOpacity WRITE setRippleOpacity)

public:
    explicit RippleButton(const QString &text, QWidget* parent = nullptr) : QPushButton(text, parent) {
        setCursor(Qt::PointingHandCursor);
    }
    qreal rippleRadius() const { return m_rippleRadius; }
    void setRippleRadius(qreal r) { m_rippleRadius = r; update(); }
    qreal rippleOpacity() const { return m_rippleOpacity; }
    void setRippleOpacity(qreal o) { m_rippleOpacity = o; update(); }

protected:
    void mousePressEvent(QMouseEvent* event) override {
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

    void paintEvent(QPaintEvent* event) override {
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

private:
    QPointF m_ripplePos;
    qreal m_rippleRadius = 0;
    qreal m_rippleOpacity = 0;
};

class MochaMsgBox : public QDialog {
public:
    static void showMsg(QWidget *parent, const QString &title, const QString &text, bool isError = false) {
        QDialog dlg(parent);
        dlg.setWindowTitle(title);
        dlg.setMinimumSize(400, 150);
        
        QString qss = QStringLiteral(R"(
            QDialog { background-color: %1; }
            QLabel { color: %2; font-size: 14px; font-weight: bold; }
            QPushButton { 
                background-color: %3; color: %2; border-radius: 8px; 
                padding: 8px 24px; font-weight: bold; font-size: 14px;
            }
            QPushButton:hover { background-color: %4; }
        )").arg(Mocha::Base.name(), Mocha::Text.name(), Mocha::Surface.name(), Mocha::Hover.name());
        
        dlg.setStyleSheet(qss);

        auto *layout = new QVBoxLayout(&dlg);
        layout->setContentsMargins(20, 20, 20, 20);
        
        auto *lbl = new QLabel(text);
        lbl->setWordWrap(true);
        lbl->setAlignment(Qt::AlignCenter);
        if (isError) {
            lbl->setStyleSheet(QString("color: %1;").arg(Mocha::Red.name()));
        } else {
            lbl->setStyleSheet(QString("color: %1;").arg(Mocha::Green.name()));
        }
        
        auto *btnOk = new RippleButton("OK");
        QObject::connect(btnOk, &QPushButton::clicked, &dlg, &QDialog::accept);
        
        layout->addWidget(lbl);
        layout->addStretch();
        layout->addWidget(btnOk, 0, Qt::AlignCenter);
        
        dlg.exec();
    }
};

class FileInfoBox : public QDialog {
public:
    static void showInfo(QWidget *parent, const QString &filePath) {
        QDialog dlg(parent);
        dlg.setWindowTitle("File Information");
        dlg.setMinimumSize(500, 400);

        QString qss = QStringLiteral(R"(
            QDialog { background-color: %1; }
            QLabel { color: %2; font-size: 14px; font-weight: bold; }
            QTextEdit {
                background-color: %3; color: %2; border-radius: 8px;
                padding: 10px; font-size: 13px; border: 1px solid %4;
            }
            QPushButton { 
                background-color: %3; color: %2; border-radius: 8px; 
                padding: 8px 24px; font-weight: bold; font-size: 14px;
            }
            QPushButton:hover { background-color: %5; }
        )").arg(Mocha::Base.name(), Mocha::Text.name(), Mocha::Surface.name(), Mocha::Hover.name(), Mocha::Accent.name());
        
        dlg.setStyleSheet(qss);

        auto *layout = new QVBoxLayout(&dlg);
        layout->setContentsMargins(20, 20, 20, 20);
        
        QFileInfo fi(filePath);
        QString sizeStr = QString::number(fi.size() / (1024.0 * 1024.0), 'f', 2) + " MB";
        
        QString basicInfo = QString("File Name: %1\nSize: %2\nCreated: %3")
                                .arg(fi.fileName(), sizeStr, fi.birthTime().toString("yyyy-MM-dd HH:mm:ss"));
        
        auto *lbl = new QLabel(basicInfo);
        layout->addWidget(lbl);

        QProcess ffprobe;
        ffprobe.start("ffprobe", {"-hide_banner", filePath});
        ffprobe.waitForFinished();
        QString probeInfo = ffprobe.readAllStandardError(); 

        auto *textEdit = new QTextEdit();
        textEdit->setReadOnly(true);
        textEdit->setText(probeInfo.isEmpty() ? "No additional media info available." : probeInfo);
        layout->addWidget(textEdit);
        
        auto *btnOk = new RippleButton("Close");
        QObject::connect(btnOk, &QPushButton::clicked, &dlg, &QDialog::accept);
        
        layout->addWidget(btnOk, 0, Qt::AlignCenter);
        
        dlg.exec();
    }
};

class ModernConverterWindow : public QWidget {
    Q_OBJECT

public:
    ModernConverterWindow(const QString &profile, const QString &inputFile, QWidget *parent = nullptr)
        : QWidget(parent), m_profile(profile), m_inputFile(inputFile)
    {
        setWindowTitle("Media Mesh - Processing");
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

        fetchMediaDuration();
    }

protected:
    void paintEvent(QPaintEvent*) override {
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

    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::MouseMove) {
            m_targetGlowPos = mapFromGlobal(static_cast<QMouseEvent*>(event)->globalPosition().toPoint());
        }
        return QWidget::eventFilter(watched, event);
    }

private slots:
    void onFfmpegReadyRead() {
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

        if (speedMatch.hasMatch()) {
            m_valSpeed->setText(speedMatch.captured(1) + "x");
        }
        
        if (frameMatch.hasMatch()) {
            m_valFrame->setText(frameMatch.captured(1));
        }

        QTime elapsed(0, 0, 0);
        elapsed = elapsed.addMSecs(m_elapsedTimer.elapsed());
        m_valElapsed->setText(elapsed.toString("HH:mm:ss"));
    }

    void onFfmpegFinished(int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
            m_progressBar->setValue(100);
            m_progressBar->setFormat("100%");
            m_valETA->setText("00:00:00");
            MochaMsgBox::showMsg(this, "Success", "Conversion completed successfully!\n\n" + m_outputFile, false);
        } else {
            MochaMsgBox::showMsg(this, "Error", "Conversion failed or was canceled.", true);
        }
        close();
    }

    void cancelConversion() {
        if (m_ffmpegProcess && m_ffmpegProcess->state() == QProcess::Running) {
            m_ffmpegProcess->kill();
        }
        close();
    }

private:
    QString m_profile, m_extension, m_inputFile, m_outputFile;
    double m_totalDuration = 0.0;
    QProcess *m_ffmpegProcess;
    QElapsedTimer m_elapsedTimer;
    QPointF m_currentGlowPos, m_targetGlowPos;
    QTimer* m_glowTimer;
    QProgressBar *m_progressBar;
    QLabel *m_valSpeed, *m_valElapsed, *m_valETA, *m_valFrame;

    QString getOutputExtension(const QString &profile) {
        if (profile.startsWith("mp4")) return "mp4";
        if (profile.startsWith("mkv")) return "mkv";
        if (profile.startsWith("webm")) return "webm";
        if (profile.startsWith("davinci") || profile.startsWith("prores")) return "mov";
        if (profile.startsWith("av1")) return "mkv";
        if (profile.startsWith("jpg") || profile.startsWith("jpeg")) return "jpg";
        return profile; 
    }

    void applyStyle() {
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

    void setupUI() {
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

        auto *titleLbl = new QLabel(QString("Converting Format: %1").arg(m_profile.toUpper()));
        titleLbl->setObjectName("titleLabel");
        titleLbl->setAlignment(Qt::AlignCenter);
        cardLayout->addWidget(titleLbl);

        QFileInfo fi(m_inputFile);
        auto *fileLbl = new QLabel(fi.fileName());
        fileLbl->setObjectName("fileLabel");
        fileLbl->setAlignment(Qt::AlignCenter);
        cardLayout->addWidget(fileLbl);

        m_progressBar = new QProgressBar();
        m_progressBar->setRange(0, 100);
        m_progressBar->setValue(0);
        m_progressBar->setFormat("0%");
        cardLayout->addWidget(m_progressBar);

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

    void fetchMediaDuration() {
        QProcess ffprobe;
        QStringList args = {"-v", "error", "-show_entries", "format=duration", 
                            "-of", "default=noprint_wrappers=1:nokey=1", m_inputFile};
        ffprobe.start("ffprobe", args);
        ffprobe.waitForFinished();
        m_totalDuration = ffprobe.readAllStandardOutput().trimmed().toDouble();
        startFFmpeg();
    }

    void startFFmpeg() {
        m_ffmpegProcess = new QProcess(this);
        connect(m_ffmpegProcess, &QProcess::readyReadStandardError, this, &ModernConverterWindow::onFfmpegReadyRead);
        connect(m_ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ModernConverterWindow::onFfmpegFinished);

        QStringList args; args << "-y";
        if (m_profile.contains("_cuda")) { args << "-hwaccel" << "cuda"; if (m_profile.endsWith("_full")) args << "-hwaccel_output_format" << "cuda"; }
        else if (m_profile.contains("_nvenc")) { args << "-hwaccel" << "cuda"; }
        else if (m_profile.contains("_amd")) { args << "-hwaccel" << "vaapi" << "-hwaccel_device" << "/dev/dri/renderD128"; if (m_profile.endsWith("vaapi") || m_profile.endsWith("_full")) args << "-hwaccel_output_format" << "vaapi"; }

        args << "-i" << m_inputFile;

        if (m_profile == "mp4" || m_profile == "mp4_cuda" || m_profile == "mp4_amd") args << "-c:v" << "libx264" << "-c:a" << "aac" << "-movflags" << "+faststart";
        else if (m_profile == "mp4_nvenc") args << "-c:v" << "h264_nvenc" << "-c:a" << "aac" << "-movflags" << "+faststart";
        else if (m_profile == "mp4_amd_vaapi") args << "-vf" << "format=nv12,hwupload" << "-c:v" << "h264_vaapi" << "-c:a" << "aac" << "-movflags" << "+faststart";
        else if (m_profile == "mkv" || m_profile == "mkv_cuda" || m_profile == "mkv_amd") args << "-c:v" << "libx264" << "-c:a" << "aac";
        else if (m_profile == "mkv_nvenc") args << "-c:v" << "h264_nvenc" << "-c:a" << "aac";
        else if (m_profile == "mkv_amd_vaapi") args << "-vf" << "format=nv12,hwupload" << "-c:v" << "h264_vaapi" << "-c:a" << "aac";
        else if (m_profile == "webm" || m_profile == "webm_cuda" || m_profile == "webm_amd") args << "-c:v" << "libvpx-vp9" << "-b:v" << "0" << "-crf" << "30" << "-c:a" << "libopus";
        else if (m_profile == "webm_nvenc") args << "-c:v" << "av1_nvenc" << "-preset" << "slow" << "-c:a" << "libopus";
        else if (m_profile == "webm_amd_vaapi") args << "-vf" << "format=nv12,hwupload" << "-c:v" << "vp9_vaapi" << "-c:a" << "libopus";
        else if (m_profile.startsWith("davinci")) {
            if (m_profile == "davinci_cuda_full") args << "-vf" << "scale_cuda=format=yuv422p";
            else if (m_profile == "davinci_amd_full") args << "-vf" << "hwdownload,format=nv12,format=yuv422p";
            else args << "-pix_fmt" << "yuv422p";
            args << "-c:v" << "dnxhd" << "-profile:v" << "dnxhr_sq" << "-c:a" << "pcm_s16le";
        } else if (m_profile.startsWith("prores")) {
            if (m_profile == "prores_cuda_full" || m_profile == "prores_amd_full") args << "-vf" << "hwdownload,format=nv12,format=yuv422p10le";
            else args << "-pix_fmt" << "yuv422p10le";
            args << "-c:v" << "prores_ks" << "-profile:v" << "2" << "-vendor" << "apl0" << "-bits_per_mb" << "8000" << "-c:a" << "pcm_s16le";
        } else if (m_profile == "av1") args << "-hide_banner" << "-loglevel" << "error" << "-stats" << "-c:v" << "libsvtav1" << "-preset" << "6" << "-crf" << "30" << "-c:a" << "libopus";
        else if (m_profile == "aac") args << "-vn" << "-c:a" << "aac" << "-b:a" << "192k";
        else if (m_profile == "mp3") args << "-vn" << "-c:a" << "libmp3lame" << "-b:a" << "192k";
        else if (m_profile == "srt" || m_profile == "vtt") args << "-hide_banner" << "-loglevel" << "error";
        else if (m_profile == "gif") args << "-vf" << "fps=15,scale=720:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse";
        else if (m_profile == "jpg" || m_profile == "jpeg") args << "-q:v" << "2";
        else if (m_profile == "ico") args << "-vf" << "scale=256:256:flags=lanczos";
        else if (m_profile == "bmp") args << "-hide_banner" << "-loglevel" << "error" << "-stats" << "-pix_fmt" << "rgb24";
        else if (m_profile == "webp") args << "-c:v" << "libwebp" << "-q:v" << "75";

        args << m_outputFile;
        m_elapsedTimer.start();
        m_ffmpegProcess->start("ffmpeg", args);
    }
};

class HomeWindow : public QWidget {
    Q_OBJECT
public:
    HomeWindow(QWidget *parent = nullptr) : QWidget(parent) {
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

protected:
    void paintEvent(QPaintEvent*) override {
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

    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::MouseMove) {
            m_targetGlowPos = mapFromGlobal(static_cast<QMouseEvent*>(event)->globalPosition().toPoint());
        }
        return QWidget::eventFilter(watched, event);
    }

private:
    QPointF m_currentGlowPos, m_targetGlowPos;
    QTimer* m_glowTimer;
    QLineEdit *m_pathInput;
    QComboBox *m_profileCombo;

    void applyStyle() {
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

    void setupUI() {
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
};

int main(int argc, char *argv[]) {
    qputenv("QT_QPA_PLATFORMTHEME", "xdgdesktopportal");

    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, true);

    QString profile = "";
    QString inputFile = "";

    for (int i = 1; i < argc; ++i) {
        QString arg = argv[i];
        if (arg.startsWith("-")) {
            profile = arg.mid(1).toLower();
        } else {
            inputFile = arg;
        }
    }

    if (!inputFile.isEmpty() && !profile.isEmpty()) {
        if (!QFileInfo::exists(inputFile)) {
            MochaMsgBox::showMsg(nullptr, "Error", "The provided input file does not exist.\n\n" + inputFile, true);
            return 1;
        }
        ModernConverterWindow window(profile, inputFile);
        window.show();
        return app.exec();
    }

    HomeWindow home;
    home.show();

    return app.exec();
}

#include "main.moc"