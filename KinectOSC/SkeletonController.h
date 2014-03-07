//
//  SkeletonController.h
//  KinectOSC
//
//  Created by Jeff Gregorio on 2/18/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//
//
#ifndef __KinectOSC____SkeletonController__
#define __KinectOSC____SkeletonController__

#include <iostream>
#include <vector>
#include <cmath>
#include <math.h>
#include <string>
#include <vector>

#include "NiTE.h"
#include "lo/lo.h"

#include "KeyboardDisplay.h"

#include "KinectDisplay.h"
#include "OscController.h"

#define MAX_USERS 1
//bool g_visibleUsers[MAX_USERS] = {false};
//nite::SkeletonState g_skeletonStates[MAX_USERS] = {nite::SKELETON_NONE};

using namespace std;

class SkeletonController {
    
struct Point {
    float x;
    float y;
};
    
public:
    
    SkeletonController();
    ~SkeletonController();
    
    bool init();
    vector<string> getAvailableDeviceNames();
    bool openDeviceAtIndex(int idx);
    void closeDevice();
    
    bool beginTracking();
    bool stopTracking();
    
    /* Setters */
    void setDisplay(KinectDisplay *display) { display_ = display; }
    void setKeyboardDisplay(KeyboardDisplay *kbDisplay) { kbDisplay_ = kbDisplay; }
    void enableOscTransmit()  { sendOsc_ = true; }
    void disableOscTransmit() { sendOsc_ = false; }
    void setOscSender(OscController *oscSender) { oscSender_ = oscSender; }
    void setNoteMap(const char *scale, const char *tonality, const char *key, int octave);
    
    /* Getters */
    bool isTracking() { return tracking_; }
    bool deviceIsOpen() { return deviceOpen_; }
    nite::UserTracker userTracker() const;
    
private:
    
    /* Main callback */
    void *trackSkeleton();
    static void *staticTracSkeleton(void *arg) {
        return ((SkeletonController *)arg)->trackSkeleton();
    }
    
    void generateRegionBoundaries();
    
    void estimateHeight(nite::Skeleton skeleton);
    void trackFoot(const char *LR, float x);
    void trackHands(nite::SkeletonJoint left, nite::SkeletonJoint right);
    void trackRightKnee(nite::SkeletonJoint knee, nite::SkeletonJoint foot);
    void sendNoteOn(int noteNumber, int velocity);
    void sendIntensity(int noteNumber, float value);
    void sendBrightness(int noteNumber, float value);
    void sendAllNotesOff();
    
    float getJointDistance(nite::SkeletonJoint j1, nite::SkeletonJoint j2);
    
private:
    
    vector<Point> *p0_;      // Starting points of region boundary lines
    vector<Point> *p1_;      // End points
    
//    vector<float> heightEst_;   // Height estimations for the current user
    float userHeight_;
    float confThresh_;
        
    OscController *oscSender_;
    int noteMap_[12];
    int footRegion_[2];
    
    KinectDisplay *display_;
    float frameWidth_;
    float frameHeight_;
    
    KeyboardDisplay *kbDisplay_;
    
    openni::Device device_;
    nite::Status niteStatus_;
    nite::UserTracker userTracker_;
    
    pthread_t dataThread_;
    pthread_mutex_t dataMutex_;
    bool deviceOpen_;
    bool tracking_;
    bool userInFrame_;
    bool shouldStop_;
    bool sendOsc_;
};

#endif /* defined(__KinectOSC____SkeletonController__) */
