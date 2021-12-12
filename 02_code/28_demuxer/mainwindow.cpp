#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_audioButton_clicked() {
    /*
     * 编解码这件事, 是专门开启一个线程在做这件事.
     * 这是值的做的, 因为本来这就是一个耗时操作.
     */
    _audioThread = new AudioThread(this);
    _audioThread->start();
}
