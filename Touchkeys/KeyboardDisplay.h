/*
 *  KeyboardDisplay.h
 *  keycontrol_cocoa
 *
 *  Created by Andrew McPherson on 6/6/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef KEYBOARD_DISPLAY_H
#define KEYBOARD_DISPLAY_H

#include <iostream>
#include <map>
#include <OpenGL/gl.h>
#include <boost/thread.hpp>
//#include "KeyTouchFrame.h"
#include "OpenGLDisplayBase.h"


// This class uses OpenGL to implement the actual drawing of the piano keyboard graphics.
// Graphics include the current state of each key and the touches on the surface.

class KeyboardDisplay : public OpenGLDisplayBase {
	// Internal data structures and constants
private:
    // Display dimensions, normalized to the width of one white key
    
    const float kWhiteKeyFrontWidth = 1.0;
    const float kBlackKeyWidth = 0.5;
    const float kWhiteKeyFrontLength = 2.3;
    const float kWhiteKeyBackLength = 4.1;
    const float kBlackKeyLength = 4.0;
    const float kInterKeySpacing = 0.1;
    const float kAnalogSliderVerticalSpacing = 0.2;
    const float kAnalogSliderLength = 3.0;
    const float kAnalogSliderWidth = 0.4;
    const float kAnalogSliderMinimumValue = -0.2;
    const float kAnalogSliderMaximumValue = 1.2;
    const float kAnalogSliderZeroLocation = kAnalogSliderLength * (0.0 - kAnalogSliderMinimumValue) / (kAnalogSliderMaximumValue - kAnalogSliderMinimumValue);
    const float kAnalogSliderOneLocation = kAnalogSliderLength * (1.0 - kAnalogSliderMinimumValue) / (kAnalogSliderMaximumValue - kAnalogSliderMinimumValue);
    
    // Individual geometry for C, D, E, F, G, A, B, c'
    
    const float kWhiteKeyBackOffsets[9] = {0, 0.22, 0.42, 0, 0.14, 0.3, 0.44, 0.22, 0};
    const float kWhiteKeyBackWidths[9] = {0.6, 0.58, 0.58, 0.56, 0.56, 0.56, 0.56, 0.58, 1.0};
    
    // Display margins
    
    const float kDisplaySideMargin = 0.4;
    const float kDisplayBottomMargin = 0.8;
    const float kDisplayTopMargin = 0.8;
    
    // Key shape constants
    
    const int kShapeForNote[12] = {0, -1, 1, -1, 2, 3, -1, 4, -1, 5, -1, 6};
    const int kWhiteToChromatic[7] = {0, 2, 4, 5, 7, 9, 11};
    const float kWhiteKeyFrontBackCutoff = (6.5 / 19.0); 
    
    // Touch constants
    const float kDisplayMinTouchSize = 0.1;
    const float kDisplayTouchSizeScaler = 0.5;
    
	typedef struct {
		float locH;
		float locV1;
		float locV2;
		float locV3;
		float size1;
		float size2;
		float size3;
	} TouchInfo;
	
	typedef struct {
		float x;
		float y;
	} Point;
	
public:
	KeyboardDisplay();
	
	// Setup methods for display size and keyboard range
	void setKeyboardRange(int lowest, int highest);
	float keyboardAspectRatio() { return totalDisplayWidth_ / totalDisplayHeight_; }
	void setDisplaySize(float width, float height);
	
	// Drawing methods
	bool needsRender() { return needsUpdate_; }
	void render();
	
	// Interaction methods
	void mouseDown(float x, float y);
	void mouseDragged(float x, float y);
	void mouseUp(float x, float y);
	void rightMouseDown(float x, float y);
	void rightMouseDragged(float x, float y);
	void rightMouseUp(float x, float y);
	
	// Take action associated with clicking a key.  These are called within the mouse
	// methods but may also be called externally.
	void keyClicked(int key);
	void keyRightClicked(int key);
    void setHighlightedKey(int key, bool highlighted);
    void clearHighlightedKeys();
	
	// State-change methods
//	void setTouchForKey(int key, const KeyTouchFrame& touch);
	void clearTouchForKey(int key);
	void clearAllTouches();
    void setAnalogCalibrationStatusForKey(int key, bool isCalibrated);
	void setAnalogValueForKey(int key, float value);
    void clearAnalogData();
    
    void setAnalogSensorsPresent(bool present) { analogSensorsPresent_ = present; }
	void setTouchSensorPresentForKey(int key, bool present);
	void setTouchSensingEnabled(bool enabled);
	
private:
	void drawWhiteKey(float x, float y, int shape, bool first, bool last, bool highlighted);
	void drawBlackKey(float x, float y, bool highlighted);
	
	void drawWhiteTouch(float x, float y, int shape, float touchLocH, float touchLocV, float touchSize);
	void drawBlackTouch(float x, float y, float touchLocH, float touchLocV, float touchSize);
    
    void drawAnalogSlider(float x, float y, bool calibrated, bool whiteKey, float value);
	
	// Indicate the shape of the given MIDI note.  0-6 for white keys C-B, -1 for black keys.
	// We handle unusual shaped keys at the top or bottom of the keyboard separately.
	
	int keyShape(int key) { 
		if(key < 0) return -1;
		return kShapeForNote[key % 12]; 
	}
	
	void refreshViewport();
	
	// Conversion from internal coordinate space to external pixel values and back
	Point screenToInternal(Point& inPoint);
	Point internalToScreen(Point& inPoint);
	
	// Figure out which key (if any) the current point corresponds to
	int keyForLocation(Point& internalPoint);
		
private:
	
	int lowestMidiNote_, highestMidiNote_;			// Which keys should be displayed (use MIDI note numbers)	
	float displayPixelWidth_, displayPixelHeight_;	// Pixel resolution of the surrounding window
	float totalDisplayWidth_, totalDisplayHeight_;	// Size of the internal view (centered around origin)
	bool needsUpdate_;								// Whether the keyboard should be redrawn
	int currentHighlightedKey_;						// What key is being clicked on at the moment
    int highlightedKeys_[2];
	bool touchSensingEnabled_;						// Whether touch-sensitive keys are being used
	bool touchSensingPresentOnKey_[128];			// Whether the key with this MIDI note has a touch sensor
    
    bool analogSensorsPresent_;                     // Whether the given device has analog sensors at all
    bool analogValueIsCalibratedForKey_[128];       // Whether the analog sensor is calibrated on this key
    float analogValueForKey_[128];                  // Latest analog sensor value for each key
	
	std::map<int, TouchInfo> currentTouches_;		// Collection of current touch data
	boost::mutex displayMutex_;						// Synchronize access between data and display threads
};

#endif /* KEYBOARD_DISPLAY_H */