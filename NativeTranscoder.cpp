#include "NativeTranscoder.h"
#include <QDebug>
#include <QTime>

NativeTranscoder::NativeTranscoder(const QString& input, const QString& output, const QString& hwDeviceName, QObject* parent)
    : QThread(parent), m_input(input), m_output(output), m_hwDeviceName(hwDeviceName) {}

NativeTranscoder::~NativeTranscoder() {
    if (m_hwDeviceCtx) {
        av_buffer_unref(&m_hwDeviceCtx);
    }
}

void NativeTranscoder::cancel() {
    m_cancel = true;
}

void NativeTranscoder::setPaused(bool paused) {
    m_paused = paused;
}

bool NativeTranscoder::initHardwareDecoder(AVCodecContext* ctx, const enum AVHWDeviceType type) {
    int err = av_hwdevice_ctx_create(&m_hwDeviceCtx, type, nullptr, nullptr, 0);
    if (err < 0) {
        return false;
    }
    ctx->hw_device_ctx = av_buffer_ref(m_hwDeviceCtx);
    return true;
}

void NativeTranscoder::run() {
    AVFormatContext* ifmt_ctx = nullptr;
    AVFormatContext* ofmt_ctx = nullptr;
    int ret;

    if ((ret = avformat_open_input(&ifmt_ctx, m_input.toUtf8().constData(), nullptr, nullptr)) < 0) {
        emit conversionFinished(false, "Could not open input file.");
        return;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, nullptr)) < 0) {
        emit conversionFinished(false, "Failed to retrieve input stream information.");
        avformat_close_input(&ifmt_ctx);
        return;
    }

    avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr, m_output.toUtf8().constData());
    if (!ofmt_ctx) {
        emit conversionFinished(false, "Could not create output context.");
        avformat_close_input(&ifmt_ctx);
        return;
    }

    int videoStreamIndex = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    int64_t totalFrames = 0;
    if (videoStreamIndex >= 0) {
        totalFrames = ifmt_ctx->streams[videoStreamIndex]->nb_frames;
        if (totalFrames <= 0) {
            totalFrames = (ifmt_ctx->streams[videoStreamIndex]->duration * av_q2d(ifmt_ctx->streams[videoStreamIndex]->time_base)) * 
                          av_q2d(ifmt_ctx->streams[videoStreamIndex]->avg_frame_rate);
        }
    }

    enum AVHWDeviceType hwType = AV_HWDEVICE_TYPE_NONE;
    if (m_hwDeviceName != "CPU") {
        hwType = av_hwdevice_find_type_by_name(m_hwDeviceName.toUtf8().constData());
    }

    for (unsigned i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream* in_stream = ifmt_ctx->streams[i];
        AVStream* out_stream = avformat_new_stream(ofmt_ctx, nullptr);
        if (!out_stream) continue;

        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if (ret < 0) continue;
        out_stream->codecpar->codec_tag = 0;
    }

    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, m_output.toUtf8().constData(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            emit conversionFinished(false, "Could not open output file.");
            avformat_free_context(ofmt_ctx);
            avformat_close_input(&ifmt_ctx);
            return;
        }
    }

    ret = avformat_write_header(ofmt_ctx, nullptr);
    if (ret < 0) {
        emit conversionFinished(false, "Error occurred when opening output file.");
        avio_closep(&ofmt_ctx->pb);
        avformat_free_context(ofmt_ctx);
        avformat_close_input(&ifmt_ctx);
        return;
    }

    AVPacket* pkt = av_packet_alloc();
    int currentFrame = 0;
    QElapsedTimer timer;
    timer.start();

    while (av_read_frame(ifmt_ctx, pkt) >= 0) {
        while (m_paused && !m_cancel) {
            QThread::msleep(100);
        }
        
        if (m_cancel) {
            break;
        }

        AVStream* in_stream = ifmt_ctx->streams[pkt->stream_index];
        AVStream* out_stream = ofmt_ctx->streams[pkt->stream_index];

        pkt->pts = av_rescale_q_rnd(pkt->pts, in_stream->time_base, out_stream->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt->dts = av_rescale_q_rnd(pkt->dts, in_stream->time_base, out_stream->time_base, static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt->duration = av_rescale_q(pkt->duration, in_stream->time_base, out_stream->time_base);
        pkt->pos = -1;

        if (pkt->stream_index == videoStreamIndex) {
            currentFrame++;
            if (currentFrame % 15 == 0) {
                int percent = totalFrames > 0 ? (currentFrame * 100) / totalFrames : 0;
                double speed = currentFrame / (double)qMax(1LL, timer.elapsed() / 1000LL) / av_q2d(in_stream->avg_frame_rate);
                emit progressUpdated(percent, QString::number(speed, 'f', 1) + "x", QString::number(currentFrame));
            }
        }

        ret = av_interleaved_write_frame(ofmt_ctx, pkt);
        av_packet_unref(pkt);
        if (ret < 0) {
            break;
        }
    }

    av_packet_free(&pkt);

    if (!m_cancel) {
        av_write_trailer(ofmt_ctx);
    }

    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&ofmt_ctx->pb);
    }
    
    avformat_free_context(ofmt_ctx);
    avformat_close_input(&ifmt_ctx);

    if (m_cancel) {
        emit conversionFinished(false, "Conversion canceled by user.");
    } else {
        emit conversionFinished(true, "");
    }
}