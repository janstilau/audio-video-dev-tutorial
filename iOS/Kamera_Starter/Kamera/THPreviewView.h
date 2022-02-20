
#import <AVFoundation/AVFoundation.h>

/*
 THPreviewView 中, 将用户点击的事件, 全部进行了 Cover.
 在特定事件触发之后, 通过代理通知给外界.
 由外界来处理, 到底应该如何调用 Session 进行变化. 
 */

@protocol THPreviewViewDelegate <NSObject>

- (void)tappedToFocusAtPoint:(CGPoint)point;//聚焦
- (void)tappedToExposeAtPoint:(CGPoint)point;//曝光
- (void)tappedToResetFocusAndExposure;//点击重置聚焦&曝光

@end

@interface THPreviewView : UIView

//session用来关联AVCaptureVideoPreviewLayer 和 激活AVCaptureSession

@property (strong, nonatomic) AVCaptureSession *session;
@property (weak, nonatomic) id<THPreviewViewDelegate> delegate;

@property (nonatomic) BOOL tapToFocusEnabled; //是否聚焦
@property (nonatomic) BOOL tapToExposeEnabled; //是否曝光

@end
