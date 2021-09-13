#include "playthread.h"

#include <SDL2/SDL.h>
#include <QDebug>
#include <QFile>

/*
 * https://cloud.tencent.com/developer/article/1608832
 *
 * 慕课网李超.
实际上，所有的音频播放都遵守着一个原则，就是当声卡将要播放的声音输出到扬声器时，它首先会通过回调函数，向你要它一部分声频数据，然后拿着这部分音频数据去播放。等播放完了，它会再向你要下一部分。

至于要的数据的多少，什么时候向你要，这些都是由声卡决定的。对于我们上层应用来说，这些都是由底层 API 决定的。

为什么会出现这种情况呢？为什么播放音频与我们一般的逻辑相反呢？这是因为声卡会严格按照音频的播放时间进行播放，不会多一秒，也不会少一秒。正因为它能准确的计算出时间来，而应用层是不知道这个时间的，所以我们必须按照声卡的要求给它喂数据，而不能依据自己的性子来。

那么有人会问，为什么声卡可以精准的计算出播放时间来呢？这是因为在播放之前我们给它设置了采样率、通道数、采样大小等参数，通过这些参数它就可以计算出时间来。

我们来做个计算，假设采样率是 48000, 双通道，采样大小是 16bit，那么一秒种的数据是多少呢？ 48000*2*16=1536000. 反过来，如果我们有一段 8M 的数据，那么声卡就知道它能播放 5秒多的声音。

上面的一大段文字描述，实际上只是想说明一个道理，就是要播放的声音数据，是声卡主动要的，不能由上层直接设置。这是通过回调函数来实现的。
 */

#define FILENAME "/Users/liugq01/res/in.pcm"
// 采样率
#define SAMPLE_RATE 44100
// 采样格式
#define SAMPLE_FORMAT AUDIO_S16LSB
// 采样大小
#define SAMPLE_SIZE SDL_AUDIO_BITSIZE(SAMPLE_FORMAT)
// 声道数
#define CHANNELS 2
// 音频缓冲区的样本数量
#define SAMPLES 1024
// 每个样本占用多少个字节
#define BYTES_PER_SAMPLE ((SAMPLE_SIZE * CHANNELS) >> 3)
// 文件缓冲区的大小
#define BUFFER_SIZE (SAMPLES * BYTES_PER_SAMPLE)

typedef struct {
    int len = 0;
    int pullLen = 0;
    Uint8 *data = nullptr;
} AudioBuffer;

PlayThread::PlayThread(QObject *parent) : QThread(parent) {
    connect(this, &PlayThread::finished,
            this, &PlayThread::deleteLater);

}

PlayThread::~PlayThread() {
    disconnect();
    requestInterruption();
    quit();
    wait();

    qDebug() << this << "析构了";
}

// 等待音频设备回调(会回调多次)
void pull_audio_data(void *userdata,
                     // 需要往stream中填充PCM数据
                     Uint8 *stream,
                     // 希望填充的大小(samples * format * channels / 8)
                     int len
                    ) {
    qDebug() << "pull_audio_data" << len;

    // 清空stream（静音处理）
    SDL_memset(stream, 0, len);

    // 取出AudioBuffer
    AudioBuffer *buffer = (AudioBuffer *) userdata;

    // 文件数据还没准备好
    if (buffer->len <= 0) return;

    // 取len、bufferLen的最小值（为了保证数据安全，防止指针越界）
    buffer->pullLen = (len > buffer->len) ? buffer->len : len;

    // 填充数据
    SDL_MixAudio(stream,
                 buffer->data,
                 buffer->pullLen,
                 SDL_MIX_MAXVOLUME);
    buffer->data += buffer->pullLen;
    buffer->len -= buffer->pullLen;
}

/*
SDL播放音频有2种模式：
Push（推）：【程序】主动推送数据给【音频设备】
Pull（拉）：【音频设备】主动向【程序】拉取数据
*/
void PlayThread::run() {
    // 初始化Audio子系统
    if (SDL_Init(SDL_INIT_AUDIO)) {
        qDebug() << "SDL_Init error" << SDL_GetError();
        return;
    }

    // 音频参数
    SDL_AudioSpec spec;
    // 采样率
    spec.freq = SAMPLE_RATE;
    // 采样格式（s16le）
    spec.format = SAMPLE_FORMAT;
    // 声道数
    spec.channels = CHANNELS;
    // 音频缓冲区的样本数量（这个值必须是2的幂）
    spec.samples = SAMPLES;
    // 回调
    spec.callback = pull_audio_data;
    // 传递给回调的参数
    AudioBuffer buffer;
    spec.userdata = &buffer;

    // 打开设备
    if (SDL_OpenAudio(&spec, nullptr)) {
        qDebug() << "SDL_OpenAudio error" << SDL_GetError();
        // 清除所有的子系统
        SDL_Quit();
        return;
    }

    // 打开文件
    QFile file(FILENAME);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << FILENAME;
        // 关闭设备
        SDL_CloseAudio();
        // 清除所有的子系统
        SDL_Quit();
        return;
    }

    // 开始播放（0是取消暂停）
    SDL_PauseAudio(0);

    // 存放从文件中读取的数据
    Uint8 data[BUFFER_SIZE];
    while (!isInterruptionRequested()) {
        // 只要从文件中读取的音频数据，还没有填充完毕，就跳过
        if (buffer.len > 0) continue;

        buffer.len = file.read((char *) data, BUFFER_SIZE);

        // 文件数据已经读取完毕
        if (buffer.len <= 0) {
            // 剩余的样本数量
            int samples = buffer.pullLen / BYTES_PER_SAMPLE;
            int ms = samples * 1000 / SAMPLE_RATE;
            SDL_Delay(ms);
            break;
        }

        // 读取到了文件数据
        buffer.data = data;
    }

    // 关闭文件
    file.close();

    // 关闭设备
    SDL_CloseAudio();

    // 清除所有的子系统
    SDL_Quit();
}
