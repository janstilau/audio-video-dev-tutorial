#ifndef FFMPEGS_H
#define FFMPEGS_H

extern "C" {
#include <libavformat/avformat.h>
}

/*
 *
 * 有些音频编码器对输入的原始PCM数据是有特定参数要求的，比如要求必须是44100_s16le_2。但是你提供的PCM参数可能是48000_f32le_1。这个时候就需要先将48000_f32le_1转换成44100_s16le_2，然后再使用音频编码器对转换后的PCM进行编码。
 *
 * 重采样, 和格式转化还是不太一样的.
 *
 */

// 重采样, 是建立在 PCM 的数据的基础上的. 是, 采样率, 采样格式, 声道数的不同. 其中, 采样格式里面, 是包含着位深的信息的.
// 所以, 如果是一个压缩格式 比如 MP3 需要重采样, 需要现将 MP3 解压成为 PCM, 然后重采样, 然后在进行压缩处理
typedef struct {
    const char *filename;
    int sampleRate;
    AVSampleFormat sampleFmt;
    int chLayout;
} ResampleAudioSpec;

class FFmpegs {
public:
    FFmpegs();
    static void resampleAudio(ResampleAudioSpec &in,
                              ResampleAudioSpec &out);

    static void resampleAudio(const char *inFilename,
                              int inSampleRate,
                              AVSampleFormat inSampleFmt,
                              int inChLayout,

                              const char *outFilename,
                              int outSampleRate,
                              AVSampleFormat outSampleFmt,
                              int outChLayout);
};

#endif // FFMPEGS_H
