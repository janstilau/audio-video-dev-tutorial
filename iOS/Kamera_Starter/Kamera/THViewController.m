
#import "THViewController.h"
#import "THCameraController.h"
#import "THPreviewView.h"
#import <AVFoundation/AVFoundation.h>
#import "THFlashControl.h"
#import "THCameraModeView.h"
#import "THOverlayView.h"
#import <MobileCoreServices/MobileCoreServices.h>
#import "NSTimer+Additions.h"

@interface THViewController () <THPreviewViewDelegate>

// 由 cameraMode 来记录, 当前所处的状态.
// 然后在用户点击操作的时候, 控制 Session 进行不同的操作.
// 其实, 对于 Session 来说, 它并不关心当前是否在录制的是图片还是视频, 因为它面向的是原始数据.
// 图片, 仅仅是一帧的视频而已.
@property (nonatomic) THCameraMode cameraMode;
@property (strong, nonatomic) NSTimer *timer;

@property (strong, nonatomic) THCameraController *cameraController;

@property (weak, nonatomic) IBOutlet THPreviewView *previewView;
@property (weak, nonatomic) IBOutlet THOverlayView *overlayView;

@property (weak, nonatomic) IBOutlet UIButton *thumbnailButton;

@end

@implementation THViewController

- (IBAction)flashControlChanged:(id)sender {
    NSInteger mode = [(THFlashControl *)sender selectedMode];
    if (self.cameraMode == THCameraModePhoto) {
        self.cameraController.flashMode = mode;
    } else {
        self.cameraController.torchMode = mode;
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [self setupSessioon];
    [self configureViews];
    [self addNotificationObserver];
}

- (void)setupSessioon {
    self.cameraMode = THCameraModeVideo;
    self.cameraController = [[THCameraController alloc] init];
    
    NSError *error;
    if ([self.cameraController setupSession:&error]) {
        // View 本身, 直接添加到 VC 上, 是没有太大问题的.
        // 在控制类, 初始化好相关逻辑之后, 完成和 View 的挂钩.
        // 这也是, 为什么将控制类, View 类分离, 是一个好的策略.
        // 如果, 将太多的控制逻辑, 写到 View 层. 那么 View 层的状态不好管理. 
        [self.previewView setSession:self.cameraController.captureSession];
        self.previewView.delegate = self;
        [self.cameraController startSession];
    } else {
        NSLog(@"Error: %@", [error localizedDescription]);
    }
}

- (void)configureViews {
    self.previewView.tapToFocusEnabled = self.cameraController.cameraSupportsTapToFocus;
    self.previewView.tapToExposeEnabled = self.cameraController.cameraSupportsTapToExpose;
    
    self.previewView.layer.borderColor = [[UIColor redColor] CGColor];
    self.previewView.layer.borderWidth = 2;
    
    self.overlayView.layer.borderColor = [[UIColor greenColor] CGColor];
    self.overlayView.layer.borderWidth = 2;
}

- (void)addNotificationObserver {
    // 在保存截屏之后, 发送一个通知给外界.
    // 在 View 上, 更新 Thumbnail 的显示.
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(updateThumbnail:)
                                                 name:THThumbnailCreatedNotification
                                               object:nil];
}

- (void)updateThumbnail:(NSNotification *)notification {
    UIImage *image = notification.object;
    [self.thumbnailButton setBackgroundImage:image forState:UIControlStateNormal];
    self.thumbnailButton.layer.borderColor = [UIColor whiteColor].CGColor;
    self.thumbnailButton.layer.borderWidth = 1.0f;
}

- (IBAction)showCameraRoll:(id)sender {
    UIImagePickerController *controller = [[UIImagePickerController alloc] init];
    controller.sourceType = UIImagePickerControllerSourceTypePhotoLibrary;
    controller.mediaTypes = @[(NSString *)kUTTypeImage, (NSString *)kUTTypeMovie];
    [self presentViewController:controller animated:YES completion:nil];
}

- (AVAudioPlayer *)playerWithResource:(NSString *)resourceName {
    NSURL *url = [[NSBundle mainBundle] URLForResource:resourceName withExtension:@"caf"];
    AVAudioPlayer *player = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:nil];
    [player prepareToPlay];
    player.volume = 0.1f;
    return player;
}

- (IBAction)cameraModeChanged:(id)sender {
    self.cameraMode = [sender cameraMode];
}

- (IBAction)swapCameras:(id)sender {
    if ([self.cameraController switchCameras]) {
        BOOL hidden = NO;
        if (self.cameraMode == THCameraModePhoto) {
            hidden = !self.cameraController.cameraHasFlash;
        } else {
            hidden = !self.cameraController.cameraHasTorch;
        }
        self.overlayView.flashControlHidden = hidden;
        self.previewView.tapToExposeEnabled = self.cameraController.cameraSupportsTapToExpose;
        self.previewView.tapToFocusEnabled = self.cameraController.cameraSupportsTapToFocus;
        [self.cameraController resetFocusAndExposureModes];
    }
}

- (IBAction)captureOrRecord:(UIButton *)sender {
    if (self.cameraMode == THCameraModePhoto) {
        [self.cameraController captureStillImage];
    } else {
        if (!self.cameraController.isRecording) {
            dispatch_async(dispatch_queue_create("com.tapharmonic.kamera", NULL), ^{
                [self.cameraController startRecording];
                [self startTimer];
            });
        } else {
            [self.cameraController stopRecording];
            [self stopTimer];
        }
        sender.selected = !sender.selected;
    }
}

- (void)startTimer {
    [self.timer invalidate];
    self.timer = [NSTimer timerWithTimeInterval:0.5
                                         target:self
                                       selector:@selector(updateTimeDisplay)
                                       userInfo:nil
                                        repeats:YES];
    [[NSRunLoop mainRunLoop] addTimer:self.timer forMode:NSRunLoopCommonModes];
}

// 倒计时, 是根据 ViewController 里面的数据计算的, 而不是真正的数据部分得到的值.
// 这样不会太准确.
- (void)updateTimeDisplay {
    CMTime duration = self.cameraController.recordedDuration;
    NSUInteger time = (NSUInteger)CMTimeGetSeconds(duration);
    NSInteger hours = (time / 3600);
    NSInteger minutes = (time / 60) % 60;
    NSInteger seconds = time % 60;
    
    NSString *format = @"%02i:%02i:%02i";
    NSString *timeString = [NSString stringWithFormat:format, hours, minutes, seconds];
    self.overlayView.statusView.elapsedTimeLabel.text = timeString;
}

- (void)stopTimer {
    [self.timer invalidate];
    self.timer = nil;
    self.overlayView.statusView.elapsedTimeLabel.text = @"00:00:00";
}

- (BOOL)prefersStatusBarHidden {
    return YES;
}

- (void)tappedToFocusAtPoint:(CGPoint)point {
    [self.cameraController focusAtPoint:point];
}

- (void)tappedToExposeAtPoint:(CGPoint)point {
    [self.cameraController exposeAtPoint:point];
}

- (void)tappedToResetFocusAndExposure {
    [self.cameraController resetFocusAndExposureModes];
}

@end
