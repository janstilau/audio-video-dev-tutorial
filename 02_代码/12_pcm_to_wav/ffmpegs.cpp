#include "ffmpegs.h"
#include <QFile>
#include <QDebug>

FFmpegs::FFmpegs() {

}

void FFmpegs::pcm2wav(WAVHeader &header,
                      const char *pcmFilename,
                      const char *wavFilename) {
    // blockAlign 一个样本的字节数.
    header.blockAlign = header.bitsPerSample * header.numChannels >> 3;
    // 一个样本的字节数 * 采样频率, 就是字节率. 也就是 一秒钟会经过多少数据量.
    header.byteRate = header.sampleRate * header.blockAlign;

    // 打开pcm文件
    QFile pcmFile(pcmFilename);
    if (!pcmFile.open(QFile::ReadOnly)) {
        qDebug() << "文件打开失败" << pcmFilename;
        return;
    }

    // 通过 QFile, 可以直接拿到最终的文件大小. 这样在进行转化的时候, 文件头信息就可以直接确定了.
    header.dataChunkDataSize = pcmFile.size();
    // riffChunkDataSize 里面存储的, 是 riff 的 data 部分的 size 大小. 所以就是 pcm 的长度
    header.riffChunkDataSize = header.dataChunkDataSize
                               + sizeof (WAVHeader)
                               - sizeof (header.riffChunkId)
                               - sizeof (header.riffChunkDataSize);

    // 打开wav文件
    QFile wavFile(wavFilename);
    if (!wavFile.open(QFile::WriteOnly)) {
        qDebug() << "文件打开失败" << wavFilename;

        pcmFile.close();
        return;
    }

    // 写入头部
    // 在这个时候, header 里面的数据, 就已经是完整的 PCM 的元信息了.
    wavFile.write((const char *) &header, sizeof (WAVHeader));

    // 写入pcm数据
    char buf[1024];
    int size;
    while ((size = pcmFile.read(buf, sizeof (buf))) > 0) {
        wavFile.write(buf, size);
    }

    // 关闭文件
    pcmFile.close();
    wavFile.close();
}
