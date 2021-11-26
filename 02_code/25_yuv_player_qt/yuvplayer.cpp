#include "yuvplayer.h"

#include <QDebug>
#include <QPainter>
#include "ffmpegs.h"

extern "C" {
#include <libavutil/imgutils.h>
}

YuvPlayer::YuvPlayer(QWidget *parent) : QWidget(parent) {
    // 设置背景色
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("background: black");
}

YuvPlayer::~YuvPlayer() {
    closeFile();
    freeCurrentImage();
    stopTimer();
}

void YuvPlayer::play() {
    if (_state == Playing) return;

    // 状态可能是：暂停、停止、正常完毕

    _timerId = startTimer(_interval);
    setState(Playing);
}

void YuvPlayer::pause() {
    if (_state != Playing) return;

    // 状态可能是：正在播放

    // 停止定时器
    stopTimer();

    // 改变状态
    setState(Paused);
}

void YuvPlayer::stop() {
    if (_state == Stopped) return;

    // 状态可能是：正在播放、暂停、正常完毕

    // 停止定时器
    stopTimer();

    // 释放图片
    freeCurrentImage();

    // 刷新
    update();

    // 改变状态
    setState(Stopped);
}

bool YuvPlayer::isPlaying() {
    return _state == Playing;
}

/*
 *  这个函数, 主要是为了发送信号出去.
 */
void YuvPlayer::setState(State state) {
    if (state == _state) return;

    if (state == Stopped
            || state == Finished) {
        // 让文件读取指针回到文件首部
        _file->seek(0);
    }

    _state = state;
    emit stateChanged();
}

YuvPlayer::State YuvPlayer::getState() {
    return _state;
}

void YuvPlayer::setYuv(Yuv &yuv) {
    _yuv = yuv;

    // 关闭上一个文件
    closeFile();

    qDebug() << "打开" << yuv.filename;

    // 打开文件
    _file = new QFile(yuv.filename);
    if (!_file->open(QFile::ReadOnly)) {
        qDebug() << "file open error" << yuv.filename;
    }

    // 刷帧的时间间隔
    _interval = 1000 / _yuv.fps;

    // 一帧图片的大小
    _imgSize = av_image_get_buffer_size(yuv.pixelFormat,
                                        yuv.width,
                                        yuv.height,
                                        1);

    // 组件的尺寸
    int w = width();
    int h = height();

    // 计算rect
    int dx = 0;
    int dy = 0;
    int dw = yuv.width;
    int dh = yuv.height;

    /*
     * 拿到, 显示的尺寸, 和 YUV 里面 Img 的尺寸, 然后计算最终 影片的显示的区域.
     * 从这里我们可以看到, 影片就是一个特殊的数据而已, 最终如何显示, 显示的区域在哪里, 完全还是 UI 层可以决定的.
     */
    // 计算目标尺寸
    if (dw > w || dh > h) { // 缩放
        if (dw * h > w * dh) { // 视频的宽高比 > 播放器的宽高比
            dh = w * dh / dw;
            dw = w;
        } else {
            dw = h * dw / dh;
            dh = h;
        }
    }

    // 居中
    dx = (w - dw) >> 1;
    dy = (h - dh) >> 1;

    _dstRect = QRect(dx, dy, dw, dh); // 计算出了最终的显示区域, 然后会在 PaintEvent 里面使用.
    qDebug() << "视频的矩形框" << dx << dy << dw << dh;
}

// 当组件想重绘的时候，就会调用这个函数
// 想要绘制什么内容，在这个函数中实现
void YuvPlayer::paintEvent(QPaintEvent *event) {
    if (!_currentImage) return;

    // 将图片绘制到当前组件上
    QPainter(this).drawImage(_dstRect, *_currentImage);
}

/*
 * 影片播放, 就是在固定的时间频率里面, 不断的刷新显示影片的 View.
 * 这个实现, 目前是放到了 TimerEvent 里面.
 * 每次时间触发后, 读取数据, 然后转换成为对应的 Img 的数据格式, 然后将这个图片, 刷到显示的 View 上.
 */
void YuvPlayer::timerEvent(QTimerEvent *event) {
    // 图片大小
    char data[_imgSize];
    if (_file->read(data, _imgSize) == _imgSize) {
        RawVideoFrame in = {
            data,
            _yuv.width, _yuv.height,
            _yuv.pixelFormat
        };
        RawVideoFrame out = {
            nullptr,
            _yuv.width >> 4 << 4,
                       _yuv.height >> 4 << 4,
                       AV_PIX_FMT_RGB24
        };
        FFmpegs::convertRawVideo(in, out);

        freeCurrentImage();
        _currentImage = new QImage((uchar *) out.pixels,
                                   out.width, out.height, QImage::Format_RGB888);

        // 刷新
        update();
    } else { // 文件数据已经读取完毕
        // 停止定时器
        stopTimer();

        // 正常播放完毕
        setState(Finished);
    }
}

void YuvPlayer::closeFile() {
    if (!_file) return;

    _file->close();
    delete _file;
    _file = nullptr;
}

void YuvPlayer::stopTimer() {
    if (_timerId == 0) return;

    killTimer(_timerId);
    _timerId = 0;
}

void YuvPlayer::freeCurrentImage() {
    if (!_currentImage) return;
    free(_currentImage->bits());
    delete _currentImage;
    _currentImage = nullptr;
}
