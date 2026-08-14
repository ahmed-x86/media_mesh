#pragma once
#include <QThread>
#include <QString>
#include <QElapsedTimer>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
}

class NativeTranscoder : public QThread {
    Q_OBJECT
public:
    NativeTranscoder(const QString& input, const QString& output, const QString& hwDeviceName, QObject* parent = nullptr);
    ~NativeTranscoder();

    void cancel();
    void setPaused(bool paused);

signals:
    void progressUpdated(int percent, const QString& speed, const QString& frame);
    void conversionFinished(bool success, const QString& errorMsg);

protected:
    void run() override;

private:
    QString m_input;
    QString m_output;
    QString m_hwDeviceName;
    
    bool m_cancel = false;
    bool m_paused = false;
    
    AVBufferRef* m_hwDeviceCtx = nullptr;

    bool initHardwareDecoder(AVCodecContext* ctx, const enum AVHWDeviceType type);
};