#include "audiothread.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>

/*
 *
 * ffmpeg -f avfoundation -list_devices true -i ''
 *
[AVFoundation indev @ 0x7fba26b043c0] AVFoundation video devices:
[AVFoundation indev @ 0x7fba26b043c0] [0] FaceTime高清摄像头（内建）
[AVFoundation indev @ 0x7fba26b043c0] [1] Capture screen 0
[AVFoundation indev @ 0x7fba26b043c0] AVFoundation audio devices:
[AVFoundation indev @ 0x7fba26b043c0] [0] Cast Audio
[AVFoundation indev @ 0x7fba26b043c0] [1] Cast Audio (UI Sounds)
[AVFoundation indev @ 0x7fba26b043c0] [2] Built-in Microphone
 */

extern "C" {
// 设备
#include <libavdevice/avdevice.h>
// 格式
#include <libavformat/avformat.h>
// 工具（比如错误处理）
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
}

#ifdef Q_OS_WIN
// 格式名称
#define FMT_NAME "dshow"
// 设备名称
#define DEVICE_NAME "audio=线路输入 (3- 魅声T800)"
// PCM文件名
#define FILEPATH "F:/"
#else
#define FMT_NAME "avfoundation"
#define DEVICE_NAME ":2"
#define FILEPATH "/Users/liugq01/QtCodeHub/audio-video-dev-tutorial/output/"
#endif

/*
 * ffplay -ar 16000 -ac 1 -f s16le out.pcm
 */

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

void showSpec(AVFormatContext *ctx) {
    // 获取输入流
    AVStream *stream = ctx->streams[0];
    // 获取音频参数
    AVCodecParameters *params = stream->codecpar;
    // 声道数
    qDebug() <<  "channels: " <<  params->channels;
    // 采样率
    qDebug() << "sample_rate: " << params->sample_rate;
    // 采样格式
    qDebug() << "format: " << params->format;
    // 每一个样本的一个声道占用多少个字节
    qDebug() << "位深度: " << av_get_bytes_per_sample((AVSampleFormat) params->format);
}

// 当线程启动的时候（start），就会自动调用run函数
// run函数中的代码是在子线程中执行的
// 耗时操作应该放在run函数中
/*
 *  Thread 可以看做是模板方法的一个应用实例.
 *  Start 方法内, 会开启一个新的线程, 来去执行 Thread 里面固定的方法调用的流程, 其中就有 run 方法.
 *  run 之前, 是一些环境的准备工作, run 之后, 又是一些环境的准备工作.
 *  Thread 的子类创建者, 也就是业务开发者, 并不需要了解这些细节. 只要在 Run 方法内部, 编写这个 Thread 相关的业务代码就可以了.
 */
void AudioThread::run() {
    qDebug() << this << "开始执行----------";

    // 获取输入格式对象
    AVInputFormat *fmt = av_find_input_format(FMT_NAME);
    if (!fmt) {
        qDebug() << "获取输入格式对象失败" << FMT_NAME;
        return;
    }

    // 格式上下文（将来可以利用上下文操作设备）
    AVFormatContext *ctx = nullptr;
    // 打开设备
    int ret = avformat_open_input(&ctx, DEVICE_NAME, fmt, nullptr);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "打开设备失败" << errbuf;
        return;
    }

    // 打印一下录音设备的参数信息
    showSpec(ctx);

    // 文件名
    QString filename = FILEPATH;

    filename += QDateTime::currentDateTime().toString("MM_dd_HH_mm_ss");
    filename += ".pcm";
    QFile file(filename);

    // 打开文件
    // WriteOnly：只写模式。如果文件不存在，就创建文件；如果文件存在，就会清空文件内容
    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "文件打开失败" << filename;

        // 关闭设备
        avformat_close_input(&ctx);
        return;
    }

    // 数据包
    //    AVPacket pkt;
    /*
     * 使用循环的方式, 不断地读取输出的音频录制数据.
     * 然后, 将这些数据, 用作业务处理.
     * AVRecorder 封装的, 应该就是这些逻辑.
     * AVRecorder 中, 控制时长的属性, 应该也就是不算的记录, 当前已经存储的 Packet 的数量, 根据采样率计算出当前已经录制的时长, 当发现, 已经超时之后, 停止录制的过程.
     */

    AVPacket *pkt = av_packet_alloc();
    while (!isInterruptionRequested()) {
        // 不断采集数据
        //        ret = av_read_frame(ctx, &pkt);
        ret = av_read_frame(ctx, pkt);

        if (ret == 0) { // 读取成功
            // 将数据写入文件
            //            file.write((const char *) pkt.data, pkt.size);
            file.write((const char *) pkt->data, pkt->size);
            file.flush();
        } else if (ret == AVERROR(EAGAIN)) { // 资源临时不可用
            continue;
        } else { // 其他错误
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof (errbuf));
            qDebug() << "av_read_frame error" << errbuf << ret;
            break;
        }

        // 必须要加，释放pkt内部的资源
        //        av_packet_unref(&pkt);
        av_packet_unref(pkt);
    }
    // 释放资源
    // 关闭文件
    file.close();

    // 释放资源
    av_packet_free(&pkt);

    // 关闭设备
    avformat_close_input(&ctx);

    qDebug() << this << "正常结束----------";
}

void AudioThread::setStop(bool stop) {
    _stop = stop;
}
