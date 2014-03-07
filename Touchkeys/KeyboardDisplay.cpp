/*
 *  KeyboardDisplay.cpp
 *  keycontrol_cocoa
 *
 *  Created by Andrew McPherson on 6/6/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "KeyboardDisplay.h"
#include <iostream>
#include <cmath>

KeyboardDisplay::KeyboardDisplay() : lowestMidiNote_(0), highestMidiNote_(0), 
totalDisplayWidth_(1.0), totalDisplayHeight_(1.0), displayPixelWidth_(1.0), displayPixelHeight_(1.0),
needsUpdate_(true), currentHighlightedKey_(-1), touchSensingEnabled_(false), analogSensorsPresent_(true) {
	// Initialize OpenGL settings: 2D only
	  
	//glMatrixMode(GL_PROJECTION);
	//glDisable(GL_DEPTH_TEST);
    
    highlightedKeys_[0] = 0;
    highlightedKeys_[1] = 0;
}

void KeyboardDisplay::setKeyboardRange(int lowest, int highest) {
	if(lowest < 0 || highest < 0)
		return;
	
	displayMutex_.lock();
	
	lowestMidiNote_ = lowest;
	if(keyShape(lowest) < 0)	// Lowest key must always be a white key for display to
		lowest++;				// render properly
	
	highestMidiNote_ = highest;
	
	// Recalculate relevant display parameters
	// Display size is based on the number of white keys
	
	int numKeys = 0;
	for(int i = lowestMidiNote_; i <= highestMidiNote_; i++) {
		if(keyShape(i) >= 0)
			numKeys++;
        if(i >= 0 && i < 128) {
            analogValueForKey_[i] = 0.0;
            analogValueIsCalibratedForKey_[i] = false;
        }
	}
	
	if(numKeys == 0) {
		displayMutex_.unlock();
		return;
	}

	// Width: N keys, N-1 interkey spaces, 2 side margins
	totalDisplayWidth_ = (float)numKeys * (kWhiteKeyFrontWidth + kInterKeySpacing) 
	- kInterKeySpacing + 2.0 * kDisplaySideMargin;
	
	// Height: white key height plus top and bottom margins
	totalDisplayHeight_ = kDisplayTopMargin + kDisplayBottomMargin + kWhiteKeyFrontLength + kWhiteKeyBackLength + kAnalogSliderVerticalSpacing + kAnalogSliderLength;
	
	displayMutex_.unlock();
}

void KeyboardDisplay::setDisplaySize(float width, float height) { 
	displayMutex_.lock();
	displayPixelWidth_ = width; 
	displayPixelHeight_ = height; 
	refreshViewport(); 
	displayMutex_.unlock();
}


// Render the keyboard display

void KeyboardDisplay::render() {
	if(lowestMidiNote_ == highestMidiNote_)
		return;
	
	// Start with a light gray background
	glClearColor(0.8, 0.8, 0.8, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	float invAspectRatio = totalDisplayWidth_ / totalDisplayHeight_; //displayPixelWidth_ / displayPixelHeight_;
	float scaleValue = 2.0 / totalDisplayWidth_;	
	
	glScalef(scaleValue, scaleValue * invAspectRatio, scaleValue);
	glTranslatef(-1.0 / scaleValue, -totalDisplayHeight_ / 2.0, 0);
	glTranslatef(kDisplaySideMargin, kDisplayBottomMargin, 0.0);
	
	displayMutex_.lock();
	
	glPushMatrix();
    
	// Draw the keys themselves first, with analog values if present, then draw the touches
	for(int key = lowestMidiNote_; key <= highestMidiNote_; key++) {
		if(keyShape(key) >= 0) {
			// White keys: draw and move the frame over for the next key
//            bool highlighted = key == highlightedKeys_[0];
			drawWhiteKey(0, 0, keyShape(key), key == lowestMidiNote_, key == highestMidiNote_, key == highlightedKeys_[0] | key == highlightedKeys_[1]);
            // Analog slider should be centered with respect to the back of the white key
            if(analogSensorsPresent_ && keyShape(key) >= 0) {
                float sliderOffset = kWhiteKeyBackOffsets[keyShape(key)] + (kWhiteKeyBackWidths[keyShape(key)] - kAnalogSliderWidth) * 0.5;
                drawAnalogSlider(sliderOffset, kWhiteKeyFrontLength + kWhiteKeyBackLength + kAnalogSliderVerticalSpacing,
                                 analogValueIsCalibratedForKey_[key], true, analogValueForKey_[key]);
            }
			glTranslatef(kWhiteKeyFrontWidth + kInterKeySpacing, 0, 0);
		}
		else {
			// Black keys: draw and leave the frame in place
			int previousWhiteKeyShape = keyShape(key - 1);
			float offsetH = -1.0 + kWhiteKeyBackOffsets[previousWhiteKeyShape] + kWhiteKeyBackWidths[previousWhiteKeyShape];
			float offsetV = kWhiteKeyFrontLength + kWhiteKeyBackLength - kBlackKeyLength;

			glTranslatef(offsetH, offsetV, 0.0);
			drawBlackKey(0, 0, key == highlightedKeys_[0] | key == highlightedKeys_[1]);
            if(analogSensorsPresent_) {
                drawAnalogSlider((kBlackKeyWidth - kAnalogSliderWidth) * 0.5, kBlackKeyLength + kAnalogSliderVerticalSpacing,
                                 analogValueIsCalibratedForKey_[key], false, analogValueForKey_[key]);
            }
			glTranslatef(-offsetH, -offsetV, 0.0);
		}
	}
	
	// Restore to the original location we used when drawing the keys
	glPopMatrix();
	
	// Draw touches
	for(int key = lowestMidiNote_; key <= highestMidiNote_; key++) {
		if(keyShape(key) >= 0) {
			// Check whether there are any current touches for this key
			if(currentTouches_.count(key) > 0) {
				TouchInfo t = currentTouches_[key];
				
				if(t.locV1 >= 0)
					drawWhiteTouch(0, 0, keyShape(key), t.locH, t.locV1, t.size1);
				if(t.locV2 >= 0)
					drawWhiteTouch(0, 0, keyShape(key), t.locH, t.locV2, t.size2);
				if(t.locV3 >= 0)
					drawWhiteTouch(0, 0, keyShape(key), t.locH, t.locV3, t.size3);
			}
			
			glTranslatef(kWhiteKeyFrontWidth + kInterKeySpacing, 0, 0);			
		}
		else {
			// Black keys: draw and leave the frame in place
			int previousWhiteKeyShape = keyShape(key - 1);
			float offsetH = -1.0 + kWhiteKeyBackOffsets[previousWhiteKeyShape] + kWhiteKeyBackWidths[previousWhiteKeyShape];
			float offsetV = kWhiteKeyFrontLength + kWhiteKeyBackLength - kBlackKeyLength;
			
			glTranslatef(offsetH, offsetV, 0.0);
			
			// Check whether there are any current touches for this key
			if(currentTouches_.count(key) > 0) {
				TouchInfo t = currentTouches_[key];
				
				if(t.locV1 >= 0)
					drawBlackTouch(0, 0, t.locH, t.locV1, t.size1);
				if(t.locV2 >= 0)
					drawBlackTouch(0, 0, t.locH, t.locV2, t.size2);
				if(t.locV3 >= 0)
					drawBlackTouch(0, 0, t.locH, t.locV3, t.size3);
			}
			
			glTranslatef(-offsetH, -offsetV, 0.0);
		}
	}	

	needsUpdate_ = false;
	displayMutex_.unlock();
	
	glFlush();
}

// Mouse interaction methods

void KeyboardDisplay::mouseDown(float x, float y) {
	Point mousePoint = {x, y};
	Point scaledPoint = screenToInternal(mousePoint);	
	
	currentHighlightedKey_ = keyForLocation(scaledPoint);
	needsUpdate_ = true;
}

void KeyboardDisplay::mouseDragged(float x, float y) {
	Point mousePoint = {x, y};
	Point scaledPoint = screenToInternal(mousePoint);	
	
	currentHighlightedKey_ = keyForLocation(scaledPoint);
	needsUpdate_ = true;
}

void KeyboardDisplay::mouseUp(float x, float y) {
	Point mousePoint = {x, y};	
	Point scaledPoint = screenToInternal(mousePoint);
	
	// When the mouse is released, see if it was over a key.  If so, take any action
	// associated with clicking that key.
	
	if(currentHighlightedKey_ != -1)
		keyClicked(currentHighlightedKey_);
	
	currentHighlightedKey_ = -1;
	needsUpdate_ = true;	
}

void KeyboardDisplay::rightMouseDown(float x, float y) {
	Point mousePoint = {x, y};	
	Point scaledPoint = screenToInternal(mousePoint);
	
	int key = keyForLocation(scaledPoint);
	if(key != -1)
		keyRightClicked(key);
	
	needsUpdate_ = true;
}

void KeyboardDisplay::rightMouseDragged(float x, float y) {
	Point mousePoint = {x, y};	
	Point scaledPoint = screenToInternal(mousePoint);
}

void KeyboardDisplay::rightMouseUp(float x, float y) {
	Point mousePoint = {x, y};	
	Point scaledPoint = screenToInternal(mousePoint);
}

void KeyboardDisplay::keyClicked(int key) {
	
}

void KeyboardDisplay::keyRightClicked(int key) {
	
}

void KeyboardDisplay::setHighlightedKey(int key, bool highlighted) {
    
//    printf("\n\nSetting key %d to %s\n", key, highlighted ? "true" : "false");
    
    if (highlighted) {
        if (highlightedKeys_[0] == 0)
            highlightedKeys_[0] = key;
        else if (highlightedKeys_[1] == 0)
            highlightedKeys_[1] = key;
    }
    else {
        if (highlightedKeys_[0] == key)
            highlightedKeys_[0] = 0;
        else if (highlightedKeys_[1] == key)
            highlightedKeys_[1] = 0;
    }
    
//    printf("highlightedKeys_[0] = %d\nhighlightedKeys_[1] = %d\n\n", highlightedKeys_[0], highlightedKeys_[1]);
    
    needsUpdate_ = true;
}

void KeyboardDisplay::clearHighlightedKeys() {
    
    highlightedKeys_[0] = 0;
    highlightedKeys_[1] = 0;
    needsUpdate_ = true;
}

// Insert new touch information for the given key and request a display update.

//void KeyboardDisplay::setTouchForKey(int key, const KeyTouchFrame& touch) {
//	if(key < lowestMidiNote_ || key > highestMidiNote_)
//		return;
//	
//	displayMutex_.lock();
//	
//	TouchInfo t = {touch.locH, touch.locs[0], touch.locs[1], touch.locs[2], touch.sizes[0], touch.sizes[1], touch.sizes[2]};
//	currentTouches_[key] = t;
//	
//	needsUpdate_ = true;
//	
//	displayMutex_.unlock();
//}

// Clear touch information for this key

void KeyboardDisplay::clearTouchForKey(int key) {
	displayMutex_.lock();
	
	currentTouches_.erase(key);
	
	needsUpdate_ = true;
	
	displayMutex_.unlock();
}

// Clear all current touch information

void KeyboardDisplay::clearAllTouches() {
	displayMutex_.lock();
	
	currentTouches_.clear();
    
	needsUpdate_ = true;

	displayMutex_.unlock();
}

// Indicate whether the given key is calibrated or not

void KeyboardDisplay::setAnalogCalibrationStatusForKey(int key, bool isCalibrated) {
    if(key < 0 || key > 127)
        return;
    analogValueIsCalibratedForKey_[key] = isCalibrated;
    needsUpdate_ = true;
}

// Set the current value of the analog sensor for the given key.
// Whether calibrated or not, the data should be in the range 0.0-1.0
// with a bit of room for deviation outside that range (i.e. for extra key
// pressure > 1.0, or for resting key states slightly miscalibrated < 0.0).

void KeyboardDisplay::setAnalogValueForKey(int key, float value) {
    if(key < 0 || key > 127)
        return;
    analogValueForKey_[key] = value;
    needsUpdate_ = true;
}

// Clear all the analog data for all keys
void KeyboardDisplay::clearAnalogData() {
    for(int key = 0; key < 128; key++) {
        analogValueForKey_[key] = 0.0;
    }
    needsUpdate_ = true;
}

// Indicate whether a given key has touch sensing capability

void KeyboardDisplay::setTouchSensorPresentForKey(int key, bool present) {
	if(key < 0 || key > 127 || !touchSensingEnabled_)
		return;
	touchSensingPresentOnKey_[key] = present;
}

// Indicate whether touch sensing is active at all on the keyboard.
// Clear all key-specific information on whether a touch-sensing key is connected

void KeyboardDisplay::setTouchSensingEnabled(bool enabled) {
	touchSensingEnabled_ = enabled;
	
	for(int i = 0; i < 128; i++)
		touchSensingPresentOnKey_[i] = false;
}

// Draw the outline of a white key.  Shape ranges from 0-7, giving the type of white key to draw
// Coordinates give the lower-left corner of the key

void KeyboardDisplay::drawWhiteKey(float x, float y, int shape, bool first, bool last, bool highlighted) {
	// First and last keys will have special geometry since there is no black key below
	// Figure out the precise geometry in this case...
	
	float backOffset, backWidth;
	
	if(first) {
		backOffset = 0.0;
		backWidth = kWhiteKeyBackOffsets[shape] + kWhiteKeyBackWidths[shape];
	}
	else if(last) {
		backOffset = kWhiteKeyBackOffsets[shape];
		backWidth = 1.0 - kWhiteKeyBackOffsets[shape];
	}
	else {
		backOffset = kWhiteKeyBackOffsets[shape];
		backWidth = kWhiteKeyBackWidths[shape]; 
	}
	
	// First draw white fill as two squares
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	
	if(highlighted)
		glColor3f(1.0, 0.5, 0.5);
	else
		glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_QUADS);
	
	glVertex2f(x, y);
	glVertex2f(x, y + kWhiteKeyFrontLength);
	glVertex2f(x + kWhiteKeyFrontWidth, y + kWhiteKeyFrontLength);
	glVertex2f(x + kWhiteKeyFrontWidth, y);
	
	glVertex2f(x + backOffset, y + kWhiteKeyFrontLength);
	glVertex2f(x + backOffset, y + kWhiteKeyFrontLength + kWhiteKeyBackLength);
	glVertex2f(x + backOffset + backWidth, y + kWhiteKeyFrontLength + kWhiteKeyBackLength);
	glVertex2f(x + backOffset + backWidth, y + kWhiteKeyFrontLength);
	
	glEnd();
	
	// Now draw the outline as black line segments
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(0.0, 0.0, 0.0);			
	glBegin(GL_POLYGON);
	
	glVertex2f(x, y);
    glVertex2f(x, y + kWhiteKeyFrontLength);
    glVertex2f(x + backOffset, y + kWhiteKeyFrontLength);
    glVertex2f(x + backOffset, y + kWhiteKeyFrontLength + kWhiteKeyBackLength);
    glVertex2f(x + backOffset + backWidth, y + kWhiteKeyFrontLength + kWhiteKeyBackLength);
    glVertex2f(x + backOffset + backWidth, y + kWhiteKeyFrontLength);
    glVertex2f(x + kWhiteKeyFrontWidth, y + kWhiteKeyFrontLength);
    glVertex2f(x + kWhiteKeyFrontWidth, y);

	glEnd();
}

// Draw the outline of a black key, given its lower-left corner

void KeyboardDisplay::drawBlackKey(float x, float y, bool highlighted) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if(highlighted)
		glColor3f(0.5, 0.0, 0.0);
	else
		glColor3f(0.0, 0.0, 0.0);					// Display color black
	glBegin(GL_POLYGON);
	
	glVertex2f(x, y);
    glVertex2f(x, y + kBlackKeyLength);
	glVertex2f(x + kBlackKeyWidth, y + kBlackKeyLength);
	glVertex2f(x + kBlackKeyWidth, y);
	
	glEnd();
}

// Draw a circle indicating a touch on the white key surface

void KeyboardDisplay::drawWhiteTouch(float x, float y, int shape, float touchLocH, float touchLocV, float touchSize) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0, 0.0, 1.0);
	
	glBegin(GL_POLYGON);
	if(/*touchLocV < kWhiteKeyFrontBackCutoff && */touchLocH >= 0.0) { // FIXME: find a more permanent solution
		// Here, the touch is in a location that has both horizontal and vertical information.
		for(int i = 0; i < 360; i += 5) {
			glVertex2f(x + cosf((float)i*3.14159/180.0)*(kDisplayMinTouchSize + touchSize*kDisplayTouchSizeScaler)
					   + touchLocH*kWhiteKeyFrontWidth,
					   y + sinf((float)i*3.14159/180.0)*(kDisplayMinTouchSize + touchSize*kDisplayTouchSizeScaler)
					   + kWhiteKeyFrontLength*(touchLocV/kWhiteKeyFrontBackCutoff));
		}
	}
	else {
		// The touch is in the back part of the key, or for some reason lacks horizontal information
		for(int i = 0; i < 360; i += 5) {
			glVertex2f(x + cosf((float)i*3.14159/180.0)*(kDisplayMinTouchSize + touchSize*kDisplayTouchSizeScaler)
					   + kWhiteKeyBackOffsets[shape] + kWhiteKeyBackWidths[shape]/2,
					   y + sinf((float)i*3.14159/180.0)*(kDisplayMinTouchSize + touchSize*kDisplayTouchSizeScaler) 
					   + kWhiteKeyFrontLength + (kWhiteKeyBackLength*
												 ((touchLocV-kWhiteKeyFrontBackCutoff)/(1.0-kWhiteKeyFrontBackCutoff))));
		}		
	}
	glEnd();
}

// Draw a circle indicating a touch on the black key surface

void KeyboardDisplay::drawBlackTouch(float x, float y, float touchLocH, float touchLocV, float touchSize) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.0, 1.0, 0.0);
	
	glBegin(GL_POLYGON);
    
    if(touchLocH < 0.0)
        touchLocH = 0.5;

	for(int i = 0; i < 360; i += 5) {
		glVertex2f(x + cosf((float)i*3.14159/180.0)*(kDisplayMinTouchSize + touchSize*kDisplayTouchSizeScaler)
                   + touchLocH * kBlackKeyWidth,
				   y + sinf((float)i*3.14159/180.0)*(kDisplayMinTouchSize + touchSize*kDisplayTouchSizeScaler) + kBlackKeyLength*touchLocV);
	}	

	glEnd();
}

// Draw a slider bar indicating the current key analog position

void KeyboardDisplay::drawAnalogSlider(float x, float y, bool calibrated, bool whiteKey, float value) {
    // First some gray lines indicating the 0.0 and 1.0 marks
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(0.5, 0.5, 0.5);
    
	glBegin(GL_POLYGON);
	glVertex2f(x, y + kAnalogSliderZeroLocation);
    glVertex2f(x, y + kAnalogSliderOneLocation);
	glVertex2f(x + kAnalogSliderWidth, y + kAnalogSliderOneLocation);
	glVertex2f(x + kAnalogSliderWidth, y + kAnalogSliderZeroLocation);
	glEnd();
    
    // Draw a red box at the top for uncalibrated values
    if(!calibrated) {
        glColor3f(1.0, 0.0, 0.0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBegin(GL_POLYGON);
        glVertex2f(x, y + kAnalogSliderOneLocation);
        glVertex2f(x, y + kAnalogSliderLength);
        glVertex2f(x + kAnalogSliderWidth, y + kAnalogSliderLength);
        glVertex2f(x + kAnalogSliderWidth, y + kAnalogSliderOneLocation);
        glEnd();
    }
    
    // Next the filled part indicating the specific value (same color as touches), then the outline
    if(whiteKey)
        glColor3f(1.0, 0.0, 1.0);
    else
        glColor3f(0.0, 1.0, 0.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_POLYGON);
    
    float locationForValue = kAnalogSliderLength * (value - kAnalogSliderMinimumValue) / (kAnalogSliderMaximumValue - kAnalogSliderMinimumValue);
    if(locationForValue < 0.0)
        locationForValue = 0.0;
    if(locationForValue > kAnalogSliderLength)
        locationForValue = kAnalogSliderLength;
    
    // Draw solid box from 0.0 to current value
    glVertex2f(x, y + kAnalogSliderZeroLocation);
    glVertex2f(x, y + locationForValue);
    glVertex2f(x + kAnalogSliderWidth, y + locationForValue);
    glVertex2f(x + kAnalogSliderWidth, y + kAnalogSliderZeroLocation);
	glEnd();
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(0.0, 0.0, 0.0);
    
	glBegin(GL_POLYGON);
	glVertex2f(x, y);
    glVertex2f(x, y + kAnalogSliderLength);
	glVertex2f(x + kAnalogSliderWidth, y + kAnalogSliderLength);
	glVertex2f(x + kAnalogSliderWidth, y);
	glEnd();
}

void KeyboardDisplay::refreshViewport() {
	glViewport(0, 0, displayPixelWidth_, displayPixelHeight_);
}

// Conversion from internal coordinate space to external pixel values and back

// Pixel values go from 0,0 (lower left) to displayPixelWidth_, displayPixelHeight_ (upper right)
// Internal values go from -totalDisplayWidth_/2, -totalDisplayHeight_/2 (lower left)
//   to totalDisplayWidth_/2, totalDisplayHeight_/2 (upper right)

// Pixel value in --> OpenGL value out
KeyboardDisplay::Point KeyboardDisplay::screenToInternal(Point& inPoint) {
	Point out;
	
	out.x = -totalDisplayWidth_*0.5 + (inPoint.x/displayPixelWidth_) * totalDisplayWidth_;
	out.y = -totalDisplayHeight_*0.5 + (inPoint.y/displayPixelHeight_) * totalDisplayHeight_;
	
	return out;
}

// OpenGL value in --> Pixel value out
KeyboardDisplay::Point KeyboardDisplay::internalToScreen(Point& inPoint) {
	Point out;
	
	out.x = ((inPoint.x + totalDisplayWidth_*0.5)/totalDisplayWidth_) * displayPixelWidth_;
	out.y = ((inPoint.y + totalDisplayHeight_*0.5)/totalDisplayHeight_) * displayPixelHeight_;
			  
	return out;
}

// Given an internal-coordinate representation, return the number of the key that it belongs
// in, otherwise return -1 if no key matches.

int KeyboardDisplay::keyForLocation(Point& internalPoint) {
	// First, check that the point is within the overall bounding box of the keyboard
	if(internalPoint.y < -totalDisplayHeight_*0.5 + kDisplayBottomMargin ||
	   internalPoint.y > totalDisplayHeight_*0.5 - kDisplayTopMargin)
		return -1;
	if(internalPoint.x < -totalDisplayWidth_*0.5 + kDisplaySideMargin ||
	   internalPoint.x > totalDisplayWidth_*0.5 - kDisplaySideMargin)
		return -1;
	
	// Now, look for the key region corresponding to this horizontal location
	// hLoc indicates the relative distance from the beginning of the first key
	
	float hLoc = internalPoint.x + totalDisplayWidth_*0.5 - kDisplaySideMargin;
	
	if(hLoc < 0.0)
		return -1;
	
	// normalizedHLoc indicates the index of the white key this touch is near.
	float normalizedHLoc = hLoc / (kWhiteKeyFrontWidth + kInterKeySpacing);
	
	// Two relevant regions: front of the white keys, back of the white keys with black keys
	// Distinguish them by vertical position.
	
	int shapeOfBottomKey = keyShape(lowestMidiNote_);						// White key index of lowest key
	int lowestC = (lowestMidiNote_ / 12) * 12;								// C below lowest key
	int whiteKeyNumber = floorf(normalizedHLoc);							// Number of white key
	int whiteOctaveNumber = (whiteKeyNumber + shapeOfBottomKey) / 7;		// Octave the key is in
	int chromaticKeyNumber = 12 * whiteOctaveNumber + kWhiteToChromatic[(whiteKeyNumber + shapeOfBottomKey) % 7];
	
	// Check if we're on the front area of the white keys, and if so, ignore points located in the gaps
	// between the keys
	
	if(internalPoint.y + totalDisplayHeight_*0.5 - kDisplayBottomMargin <= kWhiteKeyFrontLength) {
		if(normalizedHLoc - floorf(normalizedHLoc) > kWhiteKeyFrontWidth / (kWhiteKeyFrontWidth + kInterKeySpacing))
			return -1;		
		return lowestC + chromaticKeyNumber;		
	}
	else {
		// Back of white keys, or black keys
		
		int whiteKeyShape = keyShape(chromaticKeyNumber);
		if(whiteKeyShape < 0)	// Shouldn't happen
			return -1;
		
		float locRelativeToLeft = (normalizedHLoc - floorf(normalizedHLoc)) * (kWhiteKeyFrontWidth + kInterKeySpacing);
		
		// Check if we are in the back region of the white key.  Handle the lowest and highest notes specially since
		// the white keys are generally wider on account of no adjacent black key.
		if(lowestC + chromaticKeyNumber == lowestMidiNote_) {
			if(locRelativeToLeft <= kWhiteKeyBackOffsets[whiteKeyShape] + kWhiteKeyBackWidths[whiteKeyShape])
				return lowestC + chromaticKeyNumber;
		}
		else if(lowestC + chromaticKeyNumber == highestMidiNote_) {
			if(locRelativeToLeft >= kWhiteKeyBackOffsets[whiteKeyShape])
				return lowestC + chromaticKeyNumber;
		}
		else if(locRelativeToLeft >= kWhiteKeyBackOffsets[whiteKeyShape] &&
		   locRelativeToLeft <= kWhiteKeyBackOffsets[whiteKeyShape] + kWhiteKeyBackWidths[whiteKeyShape]) {
			return lowestC + chromaticKeyNumber;	
		}
		
		// By now, we've established that we're not on top of a white key.  See if we align to a black key.
		// Watch the vertical gap between white and black keys
		if(internalPoint.y + totalDisplayHeight_*0.5 - kDisplayBottomMargin <=
		   kWhiteKeyFrontLength + kWhiteKeyBackLength - kBlackKeyLength)
			return -1;
		
		// Is there a black key below this white key?
		if(keyShape(chromaticKeyNumber - 1) < 0) {		
			if(locRelativeToLeft <= kWhiteKeyBackOffsets[whiteKeyShape] - kInterKeySpacing &&
			   lowestC + chromaticKeyNumber > lowestMidiNote_)
				return lowestC + chromaticKeyNumber - 1;
		}
		// Or is there a black key above this white key?
		if(keyShape(chromaticKeyNumber + 1) < 0) {
			if(locRelativeToLeft >= kWhiteKeyBackOffsets[whiteKeyShape] + kWhiteKeyBackWidths[whiteKeyShape] + kInterKeySpacing
			   && lowestC + chromaticKeyNumber < highestMidiNote_)
				return lowestC + chromaticKeyNumber + 1;
		}
	}

	// If all else fails, assume we're not on any key
	return -1;
}