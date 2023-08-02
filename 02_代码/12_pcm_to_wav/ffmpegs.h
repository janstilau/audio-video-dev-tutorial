#ifndef FFMPEGS_H
#define FFMPEGS_H

#include <stdint.h>

#define AUDIO_FORMAT_PCM 1
#define AUDIO_FORMAT_FLOAT 3

// WAV文件头（44字节）
typedef struct {
    // RIFF chunk的id
    uint8_t riffChunkId[4] = {'R', 'I', 'F', 'F'};
    // RIFF chunk的data大小，即文件总长度减去8字节, 这 8 字节, 4 个字节是 riffChunkId 的值, 4 个字节是 riffChunkDataSize 占据的 uint32_t 的空间.
    uint32_t riffChunkDataSize;

    // "WAVE"
    uint8_t format[4] = {'W', 'A', 'V', 'E'};

    /* fmt chunk */
    // fmt chunk的id
    uint8_t fmtChunkId[4] = {'f', 'm', 't', ' '};

    // fmt chunk的data大小：存储PCM数据时，是16. 这是一个固定值.代表着 fmt 内容区域的大小.
    // 这个代表的是,  audioFormat + numChannels + sampleRate + byteRate + blockAlign + bitsPerSample 这些值占据的大小.
    uint32_t fmtChunkDataSize = 16;
    // 音频编码，1表示PCM，3表示Floating Point
    uint16_t audioFormat = AUDIO_FORMAT_PCM;
    // 声道数
    uint16_t numChannels;
    // 采样率
    uint32_t sampleRate;
    // 字节率 = sampleRate * blockAlign
    uint32_t byteRate;
    // 一个样本的字节数 = bitsPerSample * numChannels >> 3
    uint16_t blockAlign;
    // 位深度
    uint16_t bitsPerSample;

    /* data chunk */
    // data chunk的id
    uint8_t dataChunkId[4] = {'d', 'a', 't', 'a'};
    // data chunk的data大小：音频数据的总长度，即文件总长度减去文件头的长度(一般是44)
    uint32_t dataChunkDataSize;

    // 实际上, PCM 到 WAV 就是添加一个文件头的信息.
    // 而这个文件头的信息, 一些内容是固定的. 一些内容的值是可以计算出来的.
    // 真正的需要改动的, 其实是文件大小, 内存大小, 采样率, 采样格式, 声道数这些值.
} WAVHeader;

class FFmpegs {
public:
    FFmpegs();
    static void pcm2wav(WAVHeader &header,
                        const char *pcmFilename,
                        const char *wavFilename);
};

#endif // FFMPEGS_H
