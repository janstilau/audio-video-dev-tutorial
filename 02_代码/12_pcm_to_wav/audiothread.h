#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H

#include <QThread>

// 在 Qt 里面, QThread 和 iOS 的 NSThread 没有太大的区别, 都是使用对象的方式, 完成了线程的创建工作.
class AudioThread : public QThread {
    Q_OBJECT
private:
    void run();
    bool _stop = false;

public:
    explicit AudioThread(QObject *parent = nullptr);
    ~AudioThread();
    void setStop(bool stop);
signals:

};

#endif // AUDIOTHREAD_H
