
#import "THPreviewView.h"
#import "THOverlayView.h"

// 这个类, 没有什么作用. 就是将两个 View 放置了起来.

@interface THCameraView : UIView

@property (weak, nonatomic, readonly) THPreviewView *previewView;
@property (weak, nonatomic, readonly) THOverlayView *controlsView;

@end
