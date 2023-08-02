#include "playthread.h"

#include <SDL2/SDL.h>
#include <QDebug>
#include <QFile>

#define FILENAME "F:/in.pcm"
#define SAMPLE_RATE 44100
#define SAMPLE_SIZE 16
#define CHANNELS 2

// 音频缓冲区的样本数量
#define SAMPLES 1024
// 每个样本占用多少个字节
#define BYTES_PER_SAMPLE ((SAMPLE_SIZE * CHANNELS) / 8)
// 文件缓冲区的大小
#define BUFFER_SIZE (SAMPLES * BYTES_PER_SAMPLE)

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

int bufferLen;
char *bufferData;

//这个处理策略, 比较统一, 在 iOS, MACOS 系统也是这样的一个策略.
// 使用一个回调函数, 进行内存区域内容的填充, 操作系统会定期调用该函数, 从 stream 里面读取即将进行播放的数据, 交给声卡的驱动程序.
void pull_audio_data(void *userdata,
                     // 需要往stream中填充PCM数据
                     Uint8 *stream,
                     // 希望填充的大小(samples * format * channels / 8)
                     int len
                    ) {
    // 清空stream（静音处理）
    // 从这里来看, 消耗数据被没有置空操作, 所以业务回调里面, 主动完成了置空处理.
    SDL_memset(stream, 0, len);

    // 文件数据还没准备好
    if (bufferLen <= 0) return;

    // 取len、bufferLen的最小值（为了保证数据安全，防止指针越界）
    len = (len > bufferLen) ? bufferLen : len;

    // 填充数据
    // 使用 SDL_MixAudio, 将 bufferData 里面的内容, 填充到了 stream 中.
    // 然后驱动程序会读取 stream 里面的数据. 进行真正的音频播放
    SDL_MixAudio(stream, (Uint8 *) bufferData, len, SDL_MIX_MAXVOLUME);
    bufferData += len;
    bufferLen -= len;
}

/*
SDL播放音频有2种模式：
Push（推）：【程序】主动推送数据给【音频设备】
Pull（拉）：【音频设备】主动向【程序】拉取数据
*/
// 真正的 Thread 的启动函数, 会调用到这里.
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
    spec.format = AUDIO_S16LSB;
    // 声道数
    spec.channels = CHANNELS;
    // 音频缓冲区的样本数量（这个值必须是2的幂）
    spec.samples = 1024;

    // 回调.
    spec.callback = pull_audio_data;
    spec.userdata = 100;

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
    // 我这里就很奇怪, 为什么不把文件读取,放到回调函数的内部. 非常奇怪, 要在这里完成数据的读取. 然后在回到函数里面做消耗.
    //
    char data[BUFFER_SIZE];
    while (!isInterruptionRequested()) {
        // 只要从文件中读取的音频数据，还没有填充完毕，就跳过
        if (bufferLen > 0) continue;

        bufferLen = file.read(data, BUFFER_SIZE);
        // 文件数据已经读取完毕
        if (bufferLen <= 0) break;

        // 读取到了文件数据
        bufferData = data;
    }

//    while (!isInterruptionRequested()) {
//        bufferLen = file.read(data, BUFFER_SIZE);
//        // 文件数据已经读取完毕
//        if (bufferLen <= 0) break;

//        // 读取到了文件数据
//        bufferData = data;

//        // 等待音频数据填充完毕
//        // 只要音频数据还没有填充完毕，就Delay(sleep)
//        while (bufferLen > 0) {

//        }
//    }

    // 关闭文件
    file.close();

    // 关闭设备
    SDL_CloseAudio();

    // 清除所有的子系统
    SDL_Quit();
}
