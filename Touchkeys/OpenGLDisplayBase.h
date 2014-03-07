//
//  OpenGLDisplayBase.h
//  touchkeys
//
//  Created by Andrew McPherson on 28/01/2013.
//  Copyright (c) 2013 Andrew McPherson. All rights reserved.
//

#ifndef touchkeys_OpenGLDisplayBase_h
#define touchkeys_OpenGLDisplayBase_h

// Virtual base class that implements some basic methods that the OS-specific
// GUI can attach to. Specific displays are subclasses of this

class OpenGLDisplayBase {
public:
    OpenGLDisplayBase() {}
	
	// Setup method for display size
	virtual void setDisplaySize(float width, float height) = 0;
	
	// Drawing methods
	virtual bool needsRender() = 0;
	virtual void render() = 0;
	
	// Interaction methods
	virtual void mouseDown(float x, float y) = 0;
	virtual void mouseDragged(float x, float y) = 0;
	virtual void mouseUp(float x, float y) = 0;
	virtual void rightMouseDown(float x, float y) = 0;
	virtual void rightMouseDragged(float x, float y) = 0;
	virtual void rightMouseUp(float x, float y) = 0;
};

#endif
