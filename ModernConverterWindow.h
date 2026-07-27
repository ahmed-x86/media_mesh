#pragma once
#include <QWidget>
#include <QProcess>
#include <QElapsedTimer>
#include <QTimer>
#include <QPointF>
#include <QProgressBar>
#include <QLabel>

class ModernConverterWindow : public QWidget {
    Q_OBJECT

public:
    ModernConverterWindow(const QString &profile, const QString &inputFile, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onFfmpegReadyRead();
    void onFfmpegFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void cancelConversion();

private:
    QString m_profile, m_extension, m_inputFile, m_outputFile;
    double m_totalDuration = 0.0;
    
    QProcess *m_ffmpegProcess;
    QElapsedTimer m_elapsedTimer;
    QPointF m_currentGlowPos, m_targetGlowPos;
    QTimer* m_glowTimer;
    
    QProgressBar *m_progressBar;
    QLabel *m_valSpeed, *m_valElapsed, *m_valETA, *m_valFrame;

    QString getOutputExtension(const QString &profile);
    void applyStyle();
    void setupUI();
    void fetchMediaDuration();
    void startFFmpeg();
};