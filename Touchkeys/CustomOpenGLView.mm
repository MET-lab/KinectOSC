//
//  CustomOpenGLView.m
//
//  Created by Andrew McPherson on 6/6/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "CustomOpenGLView.h"
#include <OpenGl/gl.h>

@implementation CustomOpenGLView

- (void)drawRect:(NSRect)dirtyRect
{
    if(display == 0)
        return;
    
	NSRect bounds = [self bounds];

	display->setDisplaySize(bounds.size.width, bounds.size.height);
	display->render();
}

/*- (float)setKeyboardRangeFrom:(int)lowest to:(int)highest
{
	display->setKeyboardRange(lowest, highest);
	return display->keyboardAspectRatio();
}*/

- (void)mouseDown:(NSEvent *)event
{
    if(display == 0)
        return;
    
	NSPoint mousePoint = [self convertPoint:[event locationInWindow] fromView:nil];
	display->mouseDown(mousePoint.x, mousePoint.y);
}

- (void)mouseUp:(NSEvent *)event
{
    if(display == 0)
        return;
    
	NSPoint mousePoint = [self convertPoint:[event locationInWindow] fromView:nil];
	display->mouseUp(mousePoint.x, mousePoint.y);
}

- (void)mouseDragged:(NSEvent *)event
{
    if(display == 0)
        return;
    
	NSPoint mousePoint = [self convertPoint:[event locationInWindow] fromView:nil];
	display->mouseDragged(mousePoint.x, mousePoint.y);
}

- (void)rightMouseDown:(NSEvent *)event
{
    if(display == 0)
        return;
    
	NSPoint mousePoint = [self convertPoint:[event locationInWindow] fromView:nil];
	display->rightMouseDown(mousePoint.x, mousePoint.y);
}

- (void)rightMouseUp:(NSEvent *)event
{
    if(display == 0)
        return;
    
	NSPoint mousePoint = [self convertPoint:[event locationInWindow] fromView:nil];
	display->rightMouseUp(mousePoint.x, mousePoint.y);
}

- (void)rightMouseDragged:(NSEvent *)event
{
    if(display == 0)
        return;
    
	NSPoint mousePoint = [self convertPoint:[event locationInWindow] fromView:nil];
	display->rightMouseDragged(mousePoint.x, mousePoint.y);
}

// Check whether the KeyboadrdDisplay object needs updating, and if so, re-render it

- (void)updateIfNeeded
{
    if(display == 0)
        return;
    
	if(display->needsRender()) {
		[self setNeedsDisplay: YES];
	}
}

- (void)setDisplay:(OpenGLDisplayBase *)newDisplay
{
    display = newDisplay;
}

- (OpenGLDisplayBase *)display
{
	return display;
}

@end
