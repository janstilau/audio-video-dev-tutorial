#include "audiothread.h"

#include <QDebug>
#include "demuxer.h"

extern "C" {
#include <libavutil/imgutils.h>
}

AudioThread::AudioThread(QObject *parent) : QThread(parent) {
    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &AudioThread::finished,
            this, &AudioThread::deleteLater);
}

AudioThread::~AudioThread() {
    // 断开所有的连接
    disconnect();
    // 内存回收之前，正常结束线程
    requestInterruption();
    // 安全退出
    quit();
    wait();
    qDebug() << this << "析构（内存被回收）";
}

void AudioThread::run() {
    AudioDecodeSpec aOut;
    aOut.filename = "F:/res/out.pcm";

    VideoDecodeSpec vOut;
    vOut.filename = "F:/res/out.yuv";

    // 在线程的内部,去执行文件的解析, 转码的工作.
    // 将, 输出的结构, 放在 audioOutput, videoOutput 这两个输出参数里面.
    Demuxer().demux("F:/res/in.mp4", aOut, vOut);

    qDebug() << aOut.sampleRate
             << av_get_channel_layout_nb_channels(aOut.chLayout)
             << av_get_sample_fmt_name(aOut.sampleFmt);

    qDebug() << vOut.width << vOut.height
             << vOut.fps << av_get_pix_fmt_name(vOut.pixFmt);
}
