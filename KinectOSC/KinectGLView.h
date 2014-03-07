//
//  KinectGLView.h
//  KinectOSC
//
//  Created by Jeff Gregorio on 2/19/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//
//  Objective C wrapper class for the KinectDisplay C++ class

#import <Cocoa/Cocoa.h>
#include "KinectDisplay.h"

@interface KinectGLView : NSOpenGLView {
    
    KinectDisplay *display_;
}

- (void)drawRect:(NSRect)dirtyRect;
- (void)updateIfNeeded;
- (void)setDisplay:(KinectDisplay *)display;
- (KinectDisplay *)getDisplay;

@end
