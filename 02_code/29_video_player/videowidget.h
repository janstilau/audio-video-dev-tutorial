#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QImage>
#include "videoplayer.h"

/**
 * 显示（渲染）视频
 *
 * 这个类, 可以认为是 AVPlayerLayer 的实现.
 * 仅仅是做展示用. 实际的, 读取, 解码, 视频音频同步的工作, 交给 Player 来处理.
 */
class VideoWidget : public QWidget {
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget();

public slots:
    void onPlayerFrameDecoded(VideoPlayer *player,
                              uint8_t *data,
                              VideoPlayer::VideoSwsSpec &spec);
    void onPlayerStateChanged(VideoPlayer *player);

private:
    QImage *_image = nullptr;
    QRect _rect;
    void paintEvent(QPaintEvent *event) override;

    void freeImage();

signals:

};

#endif // VIDEOWIDGET_H
