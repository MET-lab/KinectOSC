//
//  AppDelegate.mm
//  KinectOSC
//
//  Created by Jeff Gregorio on 2/18/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import "AppDelegate.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    
    /* Redirect stdout to our log window */
	logPipe = [NSPipe pipe];
	logPipeReadHandle = [logPipe fileHandleForReading];
	dup2([[logPipe fileHandleForWriting] fileDescriptor], fileno(stdout)) ;
	[[NSNotificationCenter defaultCenter]   addObserver: self
                                               selector: @selector (handleNotification:)
                                                   name: NSFileHandleReadCompletionNotification
                                                 object: logPipeReadHandle];
	[logPipeReadHandle readInBackgroundAndNotify];
    
    
    /* Initialize the kinect skeleton display and pass it to the open GL view */
    kinectDisplay_ = new KinectDisplay();
    [kinectGLView_ setDisplay:kinectDisplay_];
    [NSTimer scheduledTimerWithTimeInterval:1.0/30.0 target:kinectGLView_ selector:@selector(updateIfNeeded) userInfo:nil repeats:YES];
    
    /* Intialize the SkeletonController and set its display object */
    skeletonController_ = new SkeletonController();
    skeletonController_->setDisplay([kinectGLView_ getDisplay]);
    skeletonController_->init();
    
    /* Pass the cpp keyboard display to the openGL view and skeleton controller */
    keyboardDisplay_ = new KeyboardDisplay();
    [keyboardGLView_ setDisplay:keyboardDisplay_];
    keyboardDisplay_->setKeyboardRange(21, 108);
//    NSSize ratio
    [NSTimer scheduledTimerWithTimeInterval:1.0/30.0 target:keyboardGLView_ selector:@selector(updateIfNeeded) userInfo:nil repeats:YES];
    skeletonController_->setKeyboardDisplay(keyboardDisplay_);
    
    /* Populate the pop-up menu with device names */
    [self setAvailableDevices];
    [self setNoteMap:(id)self];
    
    /* If there's only one device, open it automatically */
    if ([deviceSelection_ numberOfItems] == 1 && [trackingStartButton_ isEnabled]) {
        [self deviceSelected:(id)self];
    }
    
    /* Initialize the OSC sender and tell the skeleton controller to send OSC messages */
    oscSender_ = new OscController();
    const char *address = [[oscOutputServerAddress_ stringValue] cStringUsingEncoding:NSASCIIStringEncoding];
    const char *port = [[oscOutputPortNumber_ stringValue] cStringUsingEncoding:NSASCIIStringEncoding];
    oscSender_->setServerAddress(address, port);
    skeletonController_->setOscSender(oscSender_);
}

- (IBAction)deviceSelected:(id)sender {
    
    if (skeletonController_->isTracking())      // Stop tracking
        skeletonController_->stopTracking();
    
    skeletonController_->closeDevice();         // Close any open device
    
    if (skeletonController_->openDeviceAtIndex((int)[deviceSelection_ indexOfSelectedItem])) {
        printf("Device \"%s\" opened successfully\n", [[deviceSelection_ titleOfSelectedItem] UTF8String]);
    }
    else {
        printf("Error opening device %s\n", [[deviceSelection_ titleOfSelectedItem] UTF8String]);
    }
}

- (IBAction)trackingStartButtonPressed:(NSButton *)sender {
    
    if (sender.state == NSOnState) {
        
        if (!skeletonController_->deviceIsOpen())
            [self deviceSelected:(id)self];
        
        skeletonController_->beginTracking();
        [trackingStartButton_ setTitle:@"Stop"];
    }
    else {
        skeletonController_->stopTracking();
        kinectDisplay_->clearUser();
        [trackingStartButton_ setTitle:@"Start"];
    }
}

- (IBAction)setNoteMap:(id)sender {
    
    skeletonController_->setNoteMap([[[scaleSelection_ selectedItem] title] cStringUsingEncoding:NSASCIIStringEncoding], [[[tonalitySelection_ selectedItem] title] cStringUsingEncoding:NSASCIIStringEncoding], [[[keySelection_ selectedItem] title] cStringUsingEncoding:NSASCIIStringEncoding], (int)[octaveSelection_ indexOfSelectedItem]);
    
    if ([[[scaleSelection_ selectedItem] title]  isEqual: @"Chromatic"]) {
        [tonalitySelection_ setEnabled:NO];
    }
    else [tonalitySelection_ setEnabled:YES];
}

- (IBAction)oscOutputEnableButtonPressed:(NSButton *)sender {
    
    if ([sender state] == NSOnState) {
        skeletonController_->enableOscTransmit();
        oscSender_->enableLogging();
    }
    
    else {
        skeletonController_->disableOscTransmit();
        oscSender_->disableLogging();
    }
}

- (IBAction)oscOutputPortNumberChanged:(NSTextField *)sender {
    
    if (oscSender_) {
        oscSender_->setServerAddress([[oscOutputServerAddress_ stringValue] cStringUsingEncoding:NSASCIIStringEncoding],[[oscOutputPortNumber_ stringValue] cStringUsingEncoding:NSASCIIStringEncoding]);
    }
}

- (IBAction)oscOutputServerAddressChanged:(NSTextField *)sender {
    
    if (oscSender_) {
        oscSender_->setServerAddress([[oscOutputServerAddress_ stringValue] cStringUsingEncoding:NSASCIIStringEncoding],[[oscOutputPortNumber_ stringValue] cStringUsingEncoding:NSASCIIStringEncoding]);
    }
}

/* Handle incoming log messages (redirected from stdout) */
- (void)handleNotification:(NSNotification *)notification
{
    NSDictionary *attributes = [NSDictionary dictionaryWithObject:[NSColor greenColor]
                                                forKey:NSForegroundColorAttributeName];
    
    NSString *str = [[NSString alloc] initWithData:[[notification userInfo]
                                      objectForKey: NSFileHandleNotificationDataItem]
                                          encoding: NSASCIIStringEncoding];
    
    NSAttributedString* attr = [[NSAttributedString alloc] initWithString:str
                                                               attributes:attributes];
    
    [[logView_ textStorage] appendAttributedString:attr];
    [[logView_ textStorage] setFont:[NSFont fontWithName:@"Menlo" size:11]];
    [logView_ scrollRangeToVisible:NSMakeRange([[logView_ string] length], 0)];
    
    [logPipeReadHandle readInBackgroundAndNotify];
}

- (void)setAvailableDevices {
    
    [deviceSelection_ removeAllItems];
    
    vector<string> names = skeletonController_->getAvailableDeviceNames();
    
    if (!names.empty()) {
        for (int i = 0; i < names.size(); i++) {
        [deviceSelection_ addItemWithTitle:[NSString stringWithFormat:@"%s", names[i].c_str()]];
        [trackingStartButton_ setEnabled:YES];
        }
    }
    else {
        [deviceSelection_ addItemWithTitle:@"No Devices Found"];
        [trackingStartButton_ setEnabled:NO];
    }
}
@end









