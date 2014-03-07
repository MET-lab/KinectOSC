//
//  AppDelegate.h
//  KinectOSC
//
//  Created by Jeff Gregorio on 2/18/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "CustomOpenGLView.h"

#include <iostream>

#include "OpenNI.h"

#include "SkeletonController.h"
#include "KinectGLView.h"
#include "OscController.h"

@interface AppDelegate : NSObject <NSApplicationDelegate> {
    
    /* Kinect */
    KinectDisplay *kinectDisplay_;
    SkeletonController *skeletonController_;
    IBOutlet KinectGLView *kinectGLView_;
    IBOutlet NSPopUpButton *deviceSelection_;
    IBOutlet NSButton *trackingStartButton_;
    IBOutlet NSPopUpButton *scaleSelection_;
    IBOutlet NSPopUpButton *tonalitySelection_;
    IBOutlet NSPopUpButton *keySelection_;
    IBOutlet NSPopUpButton *octaveSelection_;
    
    /* Logging */
    IBOutlet NSTextView *logView_;
	NSPipe *logPipe;
	NSFileHandle *logPipeReadHandle;
	NSDictionary *logStringAttributes;
    
    /* Keyboard display */
    IBOutlet CustomOpenGLView *keyboardGLView_;
    KeyboardDisplay *keyboardDisplay_;
    
    /* OSC */
    OscController *oscSender_;
    IBOutlet NSButton *oscOutputEnableButton_;
    IBOutlet NSTextField *oscOutputPortNumber_;
    IBOutlet NSTextField *oscOutputServerAddress_;
    
    IBOutlet NSSplitView *ioLogSplitView_;
}

@property (assign) IBOutlet NSWindow *window;

- (IBAction)deviceSelected:(id)sender;
- (IBAction)trackingStartButtonPressed:(NSButton *)sender;
- (IBAction)setNoteMap:(id)sender;
- (IBAction)oscOutputEnableButtonPressed:(NSButton *)sender;
- (IBAction)oscOutputPortNumberChanged:(NSTextField *)sender;
- (IBAction)oscOutputServerAddressChanged:(NSTextField *)sender;

- (void)setAvailableDevices;

@end








