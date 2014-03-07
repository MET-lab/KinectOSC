//
//  CustomOpenGLView.h
//
//  Created by Andrew McPherson on 6/6/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "KeyboardDisplay.h"


@interface CustomOpenGLView : NSOpenGLView {
	OpenGLDisplayBase *display;
}

- (void)drawRect:(NSRect)dirtyRect;
//- (float)setKeyboardRangeFrom:(int)lowest to:(int)highest;
- (void)updateIfNeeded;
- (void)setDisplay:(OpenGLDisplayBase *)newDisplay;
- (OpenGLDisplayBase *)display;

@end
