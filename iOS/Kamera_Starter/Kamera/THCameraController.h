
#import <AVFoundation/AVFoundation.h>

extern NSString *const THThumbnailCreatedNotification;

@protocol THCameraControllerDelegate <NSObject>

// 1发生错误事件是，需要在对象委托上调用一些方法来处理
- (void)deviceConfigurationFailedWithError:(NSError *)error;
- (void)mediaCaptureFailedWithError:(NSError *)error;
- (void)assetLibraryWriteFailedWithError:(NSError *)error;

@end


// 这个类, 作为整个 Session 相关逻辑的控制类.
// Controller 不一定是 ViewController.

@interface THCameraController : NSObject

@property (weak, nonatomic) id<THCameraControllerDelegate> delegate;
// Readonly. Session 的生命周期, 由自己内部控制.
@property (nonatomic, strong, readonly) AVCaptureSession *captureSession;


// 2 用于设置、配置视频捕捉会话
- (BOOL)setupSession:(NSError **)error;
- (void)startSession;
- (void)stopSession;

// 3 切换不同的摄像头
- (BOOL)switchCameras;
- (BOOL)canSwitchCameras;
@property (nonatomic, readonly) NSUInteger cameraCount;
@property (nonatomic, readonly) BOOL cameraHasTorch; //手电筒
@property (nonatomic, readonly) BOOL cameraHasFlash; //闪光灯
@property (nonatomic, readonly) BOOL cameraSupportsTapToFocus; //聚焦
@property (nonatomic, readonly) BOOL cameraSupportsTapToExpose;//曝光
@property (nonatomic) AVCaptureTorchMode torchMode; //手电筒模式
@property (nonatomic) AVCaptureFlashMode flashMode; //闪光灯模式

// 4 聚焦、曝光、重设聚焦、曝光的方法
- (void)focusAtPoint:(CGPoint)point;
- (void)exposeAtPoint:(CGPoint)point;
- (void)resetFocusAndExposureModes;

// 5 实现捕捉静态图片 & 视频的功能

//捕捉静态图片
- (void)captureStillImage;

//视频录制
//开始录制
- (void)startRecording;

//停止录制
- (void)stopRecording;

//获取录制状态
- (BOOL)isRecording;

//录制时间
- (CMTime)recordedDuration;

@end
