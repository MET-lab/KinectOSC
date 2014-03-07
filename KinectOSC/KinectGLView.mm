//
//  KinectGLView.m
//  KinectOSC
//
//  Created by Jeff Gregorio on 2/19/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import "KinectGLView.h"

@implementation KinectGLView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
//	[super drawRect:dirtyRect];
	
    if (display_ == 0)
        return;
    
    NSRect bounds = [self bounds];
    
    display_->setDisplaySize(bounds.size.width, bounds.size.height);
    display_->render();
}

- (void)updateIfNeeded {
    
    if (display_ == 0)
        return;
    
    if (display_->needsRender()) {
        [self setNeedsDisplay:YES];
    }
}

- (void)setDisplay:(KinectDisplay *)display
{
    display_ = display;
}

- (KinectDisplay *)getDisplay
{
	return display_;
}

@end
