//
//  UIView+Debug.m
//  MCMoego
//
//  Created by JustinLau on 2019/3/25.
//  Copyright © 2019年 Moca Inc. All rights reserved.
//

#if DEBUG

#import "UIView+Debug.h"
#import "UIView+YYAdd.h"

@implementation UIView (Debug)

- (void)addBorderLine {
    [self addBorderLineInWidth:1.5];
    [self addTip:NSStringFromClass([self class])]; // 边框太不明显, 增加文字信息.
}

- (void)addBorderLineInWidth:(CGFloat)width {
    self.layer.borderWidth = width;
    self.layer.borderColor = [[UIColor randomColor] CGColor];
}

- (void)addTip:(NSString *)tip {
    static int kTipTag = 87903;
    UILabel *tipLabel = [self viewWithTag:kTipTag];
    if (!tipLabel) {
        tipLabel = [[UILabel alloc] init];
        tipLabel.font = [UIFont systemFontOfSize:11];
        tipLabel.textColor = [UIColor randomColor];
        tipLabel.origin = CGPointZero;
        [self addSubview:tipLabel];
    }
    tipLabel.text = tip;
    [tipLabel sizeToFit];
}

@end

#endif
