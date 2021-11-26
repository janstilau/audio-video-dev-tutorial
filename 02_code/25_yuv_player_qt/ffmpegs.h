#ifndef FFMPEGS_H
#define FFMPEGS_H

extern "C" {
#include <libavutil/avutil.h>
}

typedef struct {
    char *pixels; // 缓存区的指针
    int width; // 分辨率信息
    int height; // 分辨率信息
    AVPixelFormat format; // 缓存区内数据格式.
} RawVideoFrame;

typedef struct {
    const char *filename; // 文件路径信息
    int width; // 分辨率信息
    int height; // 分辨率信息
    AVPixelFormat format; // 文件内的数据格式.
} RawVideoFile;

class FFmpegs {
public:
    FFmpegs();
    static void convertRawVideo(RawVideoFrame &in,
                                RawVideoFrame &out);
    static void convertRawVideo(RawVideoFile &in,
                                RawVideoFile &out);
};

#endif // FFMPEGS_H
