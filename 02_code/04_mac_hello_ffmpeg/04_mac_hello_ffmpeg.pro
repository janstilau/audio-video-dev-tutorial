QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32: {
    FFMPEG_HOME = ..
    INCLUDEPATH += $${FFMPEG_HOME}/include
    LIBS += -L $${FFMPEG_HOME}/lib \
            -lavcodec \
            -lavdevice \
            -lavfilter \
            -lavformat \
            -lavutil \
            -lpostproc \
            -lswscale \
            -lswresample \
            -lavresample
}

linux: {

}


# 库文件路径
#mac:INCLUDEPATH += $${FFMPEG_HOME}/include

# 需要链接哪些库？默认是链接动态库
#mac:LIBS += -L $${FFMPEG_HOME}/lib \
#        -lavcodec \
#        -lavdevice \
#        -lavfilter \
#        -lavformat \
#        -lavutil \
#        -lpostproc \
#        -lswscale \
#        -lswresample \
#        -lavresample

mac: {
    FFMPEG_HOME = /usr/local/Cellar/ffmpeg/4.3.2_4
# 默认是链接动态库. 如果想要链接静态库, 写 static
# 不过, 这都太恶心了, 写死路径自然而然就可以了.
# 写死路径, 就不用 -L -l 这种写法了.
    INCLUDEPATH += $${FFMPEG_HOME}/include
    LIBS += -L $${FFMPEG_HOME}/lib \
            -lavcodec \
            -lavdevice \
            -lavfilter \
            -lavformat \
            -lavutil \
            -lpostproc \
            -lswscale \
            -lswresample \
            -lavresample
}

# message()可以用来打印

# $${}可以用来取值：.pro中定义的变量

# $$()可以用来取值：系统环境变量中的值
