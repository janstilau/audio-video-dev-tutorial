#ifndef FFMPEGS_H
#define FFMPEGS_H

extern "C" {
#include <libavformat/avformat.h>
}

// 相比较 采样率, 采样格式, 声道, 增加了一个文件路径的信息.
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
