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
#include <QMessageBox>
#include <QPainter>

namespace Mocha {
    static const QColor Base      (0x1e, 0x1e, 0x2e);
    static const QColor Surface   (0x31, 0x32, 0x44);
    static const QColor Text      (0xcd, 0xd6, 0xf4);
    static const QColor Subtext   (0xa6, 0xad, 0xc8);
    static const QColor Accent    (0xcb, 0xa6, 0xf7);
    static const QColor Green     (0xa6, 0xe3, 0xa1);
    static const QColor Red       (0xf3, 0x8b, 0xa8);
    static const QColor Yellow    (0xf9, 0xe2, 0xaf);
}

static QString globalStylesheet() {
    return QStringLiteral(R"QSS(
        QWidget {
            font-family: 'Inter', 'Segoe UI', sans-serif;
            background-color: #1e1e2e;
            color: #cdd6f4;
        }
        QProgressBar {
            background-color: #313244;
            border-radius: 8px;
            text-align: center;
            color: #1e1e2e;
            font-weight: bold;
            height: 16px;
        }
        QProgressBar::chunk {
            background-color: #cba6f7;
            border-radius: 8px;
        }
        QPushButton {
            background-color: #313244;
            color: #cdd6f4;
            border: 2px solid #45475a;
            border-radius: 10px;
            padding: 8px 16px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #45475a; border-color: #cba6f7; }
        QPushButton:pressed { background-color: #585b70; }
        QLabel#titleLabel { font-size: 20px; font-weight: bold; color: #cba6f7; }
        QLabel#statVal { font-size: 14px; font-weight: bold; color: #a6e3a1; }
        QLabel#statKey { font-size: 12px; color: #a6adc8; }
    )QSS");
}

class ConverterWindow : public QWidget {
    Q_OBJECT

public:
    ConverterWindow(const QString &profile, const QString &inputFile, QWidget *parent = nullptr)
        : QWidget(parent), m_profile(profile), m_inputFile(inputFile)
    {
        setWindowTitle("FileConverter (ClickMesh Tech)");
        setMinimumSize(500, 320);
        
        m_extension = getOutputExtension(m_profile);

        QFileInfo fi(m_inputFile);
        m_outputFile = fi.absolutePath() + QDir::separator() + fi.completeBaseName() + "." + m_extension;

        setupUI();

        fetchMediaDuration();
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
            m_valETA->setText("00:00:00");
            QMessageBox::information(this, "Success", "Conversion completed successfully!\n" + m_outputFile);
        } else {
            QMessageBox::critical(this, "Error", "Conversion failed or was canceled.");
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
    QString m_profile;
    QString m_extension;
    QString m_inputFile;
    QString m_outputFile;
    double m_totalDuration = 0.0;

    QProcess *m_ffmpegProcess;
    QElapsedTimer m_elapsedTimer;

    QProgressBar *m_progressBar;
    QLabel *m_valSpeed;
    QLabel *m_valElapsed;
    QLabel *m_valETA;
    QLabel *m_valFrame;

    QString getOutputExtension(const QString &profile) {
        if (profile.startsWith("mp4")) return "mp4";
        if (profile.startsWith("mkv")) return "mkv";
        if (profile.startsWith("webm")) return "webm";
        if (profile.startsWith("davinci") || profile.startsWith("prores")) return "mov";
        if (profile.startsWith("av1")) return "mkv";
        if (profile.startsWith("jpg") || profile.startsWith("jpeg")) return "jpg";
        return profile; 
    }

    void setupUI() {
        auto *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(24, 24, 24, 24);
        mainLayout->setSpacing(20);

        auto *titleLbl = new QLabel(QString("Converting Format: %1").arg(m_profile));
        titleLbl->setObjectName("titleLabel");
        mainLayout->addWidget(titleLbl, 0, Qt::AlignHCenter);

        QFileInfo fi(m_inputFile);
        auto *fileLbl = new QLabel(fi.fileName());
        fileLbl->setStyleSheet(QString("color: %1;").arg(Mocha::Subtext.name()));
        mainLayout->addWidget(fileLbl, 0, Qt::AlignHCenter);

        m_progressBar = new QProgressBar();
        m_progressBar->setRange(0, 100);
        m_progressBar->setValue(0);
        mainLayout->addWidget(m_progressBar);

        auto *gridLayout = new QGridLayout();
        gridLayout->setSpacing(15);
        
        auto createStatBox = [&](const QString &title, QLabel *&valLabel) {
            auto *vbox = new QVBoxLayout();
            auto *t = new QLabel(title);
            t->setObjectName("statKey");
            valLabel = new QLabel("--");
            valLabel->setObjectName("statVal");
            vbox->addWidget(t, 0, Qt::AlignHCenter);
            vbox->addWidget(valLabel, 0, Qt::AlignHCenter);
            return vbox;
        };

        gridLayout->addLayout(createStatBox("Speed", m_valSpeed), 0, 0);
        gridLayout->addLayout(createStatBox("Elapsed", m_valElapsed), 0, 1);
        gridLayout->addLayout(createStatBox("ETA", m_valETA), 1, 0);
        gridLayout->addLayout(createStatBox("Frame", m_valFrame), 1, 1);
        
        mainLayout->addLayout(gridLayout);

        auto *cancelBtn = new QPushButton("Cancel");
        connect(cancelBtn, &QPushButton::clicked, this, &ConverterWindow::cancelConversion);
        mainLayout->addWidget(cancelBtn, 0, Qt::AlignHCenter);
    }

    void fetchMediaDuration() {
        QProcess ffprobe;
        QStringList args = {"-v", "error", "-show_entries", "format=duration", 
                            "-of", "default=noprint_wrappers=1:nokey=1", m_inputFile};
        ffprobe.start("ffprobe", args);
        ffprobe.waitForFinished();
        
        QString out = ffprobe.readAllStandardOutput().trimmed();
        m_totalDuration = out.toDouble();
        
        startFFmpeg();
    }

    void startFFmpeg() {
        m_ffmpegProcess = new QProcess(this);
        
        connect(m_ffmpegProcess, &QProcess::readyReadStandardError, this, &ConverterWindow::onFfmpegReadyRead);
        connect(m_ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &ConverterWindow::onFfmpegFinished);

        QStringList args;
        args << "-y";

        if (m_profile.contains("_cuda")) {
            args << "-hwaccel" << "cuda";
            if (m_profile.endsWith("_full")) {
                args << "-hwaccel_output_format" << "cuda";
            }
        } else if (m_profile.contains("_nvenc")) {
            args << "-hwaccel" << "cuda";
        } else if (m_profile.contains("_amd")) {
            args << "-hwaccel" << "vaapi" << "-hwaccel_device" << "/dev/dri/renderD128";
            if (m_profile.endsWith("vaapi") || m_profile.endsWith("_full")) {
                args << "-hwaccel_output_format" << "vaapi";
            }
        }

        args << "-i" << m_inputFile;

        if (m_profile == "mp4" || m_profile == "mp4_cuda" || m_profile == "mp4_amd") {
            args << "-c:v" << "libx264" << "-c:a" << "aac" << "-movflags" << "+faststart";
        } else if (m_profile == "mp4_nvenc") {
            args << "-c:v" << "h264_nvenc" << "-c:a" << "aac" << "-movflags" << "+faststart";
        } else if (m_profile == "mp4_amd_vaapi") {
            args << "-vf" << "format=nv12,hwupload" << "-c:v" << "h264_vaapi" << "-c:a" << "aac" << "-movflags" << "+faststart";
        } else if (m_profile == "mkv" || m_profile == "mkv_cuda" || m_profile == "mkv_amd") {
            args << "-c:v" << "libx264" << "-c:a" << "aac";
        } else if (m_profile == "mkv_nvenc") {
            args << "-c:v" << "h264_nvenc" << "-c:a" << "aac";
        } else if (m_profile == "mkv_amd_vaapi") {
            args << "-vf" << "format=nv12,hwupload" << "-c:v" << "h264_vaapi" << "-c:a" << "aac";
        } else if (m_profile == "webm" || m_profile == "webm_cuda" || m_profile == "webm_amd") {
            args << "-c:v" << "libvpx-vp9" << "-b:v" << "0" << "-crf" << "30" << "-c:a" << "libopus";
        } else if (m_profile == "webm_nvenc") {
            args << "-c:v" << "av1_nvenc" << "-preset" << "slow" << "-c:a" << "libopus";
        } else if (m_profile == "webm_amd_vaapi") {
            args << "-vf" << "format=nv12,hwupload" << "-c:v" << "vp9_vaapi" << "-c:a" << "libopus";
        } else if (m_profile.startsWith("davinci")) {
            if (m_profile == "davinci_cuda_full") {
                args << "-vf" << "scale_cuda=format=yuv422p";
            } else if (m_profile == "davinci_amd_full") {
                args << "-vf" << "hwdownload,format=nv12,format=yuv422p";
            } else {
                args << "-pix_fmt" << "yuv422p";
            }
            args << "-c:v" << "dnxhd" << "-profile:v" << "dnxhr_sq" << "-c:a" << "pcm_s16le";
        } else if (m_profile.startsWith("prores")) {
            if (m_profile == "prores_cuda_full" || m_profile == "prores_amd_full") {
                args << "-vf" << "hwdownload,format=nv12,format=yuv422p10le";
            } else {
                args << "-pix_fmt" << "yuv422p10le";
            }
            args << "-c:v" << "prores_ks" << "-profile:v" << "2" << "-vendor" << "apl0" << "-bits_per_mb" << "8000" << "-c:a" << "pcm_s16le";
        } else if (m_profile == "av1") {
            args << "-hide_banner" << "-loglevel" << "error" << "-stats"
                 << "-c:v" << "libsvtav1" << "-preset" << "6" << "-crf" << "30" << "-c:a" << "libopus";
        } else if (m_profile == "aac") {
            args << "-vn" << "-c:a" << "aac" << "-b:a" << "192k";
        } else if (m_profile == "mp3") {
            args << "-vn" << "-c:a" << "libmp3lame" << "-b:a" << "192k";
        } else if (m_profile == "srt" || m_profile == "vtt") {
            args << "-hide_banner" << "-loglevel" << "error";
        } else if (m_profile == "gif") {
            args << "-vf" << "fps=15,scale=720:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse";
        } else if (m_profile == "jpg" || m_profile == "jpeg") {
            args << "-q:v" << "2";
        } else if (m_profile == "ico") {
            args << "-vf" << "scale=256:256:flags=lanczos";
        } else if (m_profile == "png") {
        } else if (m_profile == "bmp") {
            args << "-hide_banner" << "-loglevel" << "error" << "-stats" << "-pix_fmt" << "rgb24";
        } else if (m_profile == "webp") {
            args << "-c:v" << "libwebp" << "-q:v" << "75";
        }

        args << m_outputFile;

        m_elapsedTimer.start();
        m_ffmpegProcess->start("ffmpeg", args);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyleSheet(globalStylesheet());

    QString profile = "mp4";
    QString inputFile = "";

    for (int i = 1; i < argc; ++i) {
        QString arg = argv[i];
        if (arg.startsWith("-")) {
            profile = arg.mid(1).toLower();
        } else {
            inputFile = arg;
        }
    }

    if (inputFile.isEmpty() || !QFileInfo::exists(inputFile)) {
        QMessageBox::critical(nullptr, "Error", "Please provide a valid input file.\nUsage: ./test_app -davinci_cuda_full /path/to/file");
        return 1;
    }

    ConverterWindow window(profile, inputFile);
    window.show();

    return app.exec();
}

#include "main.moc"