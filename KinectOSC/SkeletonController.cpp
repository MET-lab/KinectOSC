//
//  SkeletonController.cpp
//  KinectOSC
//
//  Created by Jeff Gregorio on 2/18/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#include "SkeletonController.h"

SkeletonController::SkeletonController() {
    
    confThresh_ = 0.6;
    
    noteMap_[0] = 52;
    noteMap_[1] = 55;
    noteMap_[2] = 57;
    noteMap_[3] = 59;
    noteMap_[4] = 62;
    noteMap_[5] = 64;
    noteMap_[6] = 67;
    noteMap_[7] = 69;
    noteMap_[8] = 71;
    noteMap_[9] = 74;
    noteMap_[10] = 76;
    noteMap_[11] = 79;
    
    footRegion_[0] = 0;
    footRegion_[1] = 0;
    
    display_ = NULL;
    deviceOpen_ = false;
    tracking_ = false;
    userInFrame_ = false;
    sendOsc_ = false;
    
    generateRegionBoundaries();
}

SkeletonController::~SkeletonController() {
    
    nite::NiTE::shutdown();
}

bool SkeletonController::init() {
    
    /* Initialize OpenNI */
    if (openni::OpenNI::initialize() != openni::STATUS_OK) {
        printf("%s: OpenNI initialization failed\n%s\n",
               __PRETTY_FUNCTION__, openni::OpenNI::getExtendedError());
        return false;
    }
    
    /* Initialize NiTE */
    if (nite::NiTE::initialize() != nite::STATUS_OK) {
        printf("%s: OpenNI initialization failed\n", __PRETTY_FUNCTION__);
        return false;
    }
    
    return true;
}

vector<string> SkeletonController::getAvailableDeviceNames() {
    
    vector<string> names;
    
    /* Get device info for any available devices */
    openni::Array<openni::DeviceInfo> devs;
    openni::OpenNI::enumerateDevices(&devs);
    
    for (int i = 0; i < devs.getSize(); i++) {
        printf(" - Device %d:\n", i);
        printf(" -------------------------------------\n");
        printf("      Name:  %s\n", devs[i].getName());
        printf("       URI:  %s\n", devs[i].getUri());
        printf("    Vendor:  %s\n", devs[i].getVendor());
        printf(" -------------------------------------\n");
        string str(devs[i].getName());
        names.push_back(str);
    }
    
    return names;
}

bool SkeletonController::openDeviceAtIndex(int idx) {
    
    if (deviceOpen_) {
        device_.close();
//        userTracker_.destroy();
    }
    
    bool rVal = true;
    openni::Status status;
    
    /* Get device info for any available devices */
    openni::Array<openni::DeviceInfo> devs;
    openni::OpenNI::enumerateDevices(&devs);
    
    status = device_.open(devs[idx].getUri());
    if (status != openni::STATUS_OK) {
        printf("%s: Failed to open device\n%s\n",
               __PRETTY_FUNCTION__, openni::OpenNI::getExtendedError());
        rVal = false;
    }
    else {
        /* Inintialize user tracker */
        if (userTracker_.create(&device_) != nite::STATUS_OK) {
            printf("%s: Failed to create user tracker\n", __PRETTY_FUNCTION__);
            rVal = false;
        }
        else
            deviceOpen_ = true;
    }
    
    return rVal;
}

void SkeletonController::closeDevice() {
    
    if (deviceOpen_) {
        device_.close();
    }
}

bool SkeletonController::beginTracking() {
    
    /* Make sure a device is open */
    if (!deviceOpen_) {
        printf("%s: No device is open\n", __PRETTY_FUNCTION__);
        return false;
    }
    
    /* Make sure we're not already tracking */
    if (tracking_) {
        printf("%s: Already tracking skeleton\n", __PRETTY_FUNCTION__);
        return false;
    }
    
    /* Create the data mutex */
    pthread_mutex_init(&dataMutex_, NULL);
    
    /* Create the thread and set the callback */
    if (pthread_create(&dataThread_, NULL, staticTracSkeleton, (void *)this) != 0) {
        printf("%s: Error setting callback\n", __PRETTY_FUNCTION__);
        return false;
    }
    else
        printf("\nTracking...\n");
    
    tracking_ = true;
    shouldStop_ = false;
    
    return true;
}

bool SkeletonController::stopTracking() {
    
    /* Make sure we're actually tracking */
    if (!tracking_) {
        printf("%s: Not currently tracking\n", __PRETTY_FUNCTION__);
        return false;
    }
    else {
        sendAllNotesOff();
        printf("\nTracking ended\n");
    }
    
    /* Set the flag and wait for the tracking loop to finish */
    shouldStop_ = true;
    pthread_join(dataThread_, NULL);
    tracking_ = false;
    
    return true;
}

void SkeletonController::setNoteMap(const char *scale, const char *tonality, const char *key, int octave) {
    
    vector<int> intervals;
    
    /* Determine scale intervals for scale and tonality selections */
    if (!strcmp(scale, "Diatonic")) {
        if (!strcmp(tonality, "Major")) {
            intervals = {2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19};
        }
        else if (!strcmp(tonality, "Minor")) {
            intervals = {2, 3, 5, 7, 8, 10, 12, 14, 15, 17, 19};
        }
        else printf("%s: Unrecognized tonality \"%s\"\n", __PRETTY_FUNCTION__, tonality);
    }
    else if (!strcmp(scale, "Pentatonic")) {
        if (!strcmp(tonality, "Major")) {
            intervals = {2, 4, 7, 9, 12, 14, 16, 19, 21, 24, 26};
        }
        else if (!strcmp(tonality, "Minor")) {
            intervals = {3, 5, 7, 10, 12, 15, 17, 19, 22, 24, 27};
        }
        else printf("%s: Unrecognized tonality \"%s\"\n", __PRETTY_FUNCTION__, tonality);
    }
    else if (!strcmp(scale, "Chromatic")) {
        intervals = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    }
    
    /* Compute the base (C = 0 through B = 11) */
    int base;
    
    if (!strcmp(key, "C") || !strcmp(key, "c"))
        base = 0;
    if (!strcmp(key, "C#") || !strcmp(key, "c#") || !strcmp(key, "Db") || !strcmp(key, "db"))
        base = 1;
    if (!strcmp(key, "D") || !strcmp(key, "d"))
        base = 2;
    if (!strcmp(key, "D#") || !strcmp(key, "d#") || !strcmp(key, "Eb") || !strcmp(key, "eb"))
        base = 3;
    if (!strcmp(key, "E") || !strcmp(key, "e"))
        base = 4;
    if (!strcmp(key, "F") || !strcmp(key, "f"))
        base = 5;
    if (!strcmp(key, "F#") || !strcmp(key, "f#") || !strcmp(key, "Gb") || !strcmp(key, "gb"))
        base = 6;
    if (!strcmp(key, "G") || !strcmp(key, "g"))
        base = 7;
    if (!strcmp(key, "G#") || !strcmp(key, "g#") || !strcmp(key, "Ab") || !strcmp(key, "ab"))
        base = 8;
    if (!strcmp(key, "A") || !strcmp(key, "a"))
        base = 9;
    if (!strcmp(key, "A#") || !strcmp(key, "a#") || !strcmp(key, "Bb") || !strcmp(key, "bb"))
        base = 10;
    if (!strcmp(key, "B") || !strcmp(key, "b"))
        base = 11;
    
    base += octave*12;
    
    for (int i = 0; i < 12; i++) {
        noteMap_[i] = base + intervals[i];
    }
}

nite::UserTracker SkeletonController::userTracker() const {
    
    nite::UserTracker rv;
    
    /* Make sure we're tracking so the skeleton exists */
    if (userTracker_.isValid()) {
        rv = userTracker_;
    }
    
    return rv;
}

void SkeletonController::generateRegionBoundaries() {
    
//    p0_ = new vector<Point>();
//    p1_ = new vector<Point>();
//    
//    int nRegions = 12;
//    float regionWidth = frameWidth_ / 12;
//    float upperBound = frameHeight_ / 3;
//    
//    vector<double> theta = linspace(2*M_PI/5, 3*M_PI/5, nRegions+1);
//    
//    double x1, x2, xt, y1, y2, yt, m;
//    
//    for (int i = 0; i <= nRegions; i++) {
//        
//        x1 = (i-1) * regionWidth;
//        y1 = 0;
//    
//        x2 = upperBound * cos(theta[i]);
//        y2 = upperBound * sin(theta[i]);
//        
//        x2 += x1;
//        y2 += y1;
//        
//        yt = upperBound;
//        
//        if (theta[i] == M_PI/2) {
//            xt = x1;
//        }
//        else {
//            m = (y2-y1) / (x2-x1);
//            xt = (yt - y2 + m*x2) / m;
//        }
//        
//        Point p0;
//        p0.x = x1;
//        p0.y = y1;
//        p0_->push_back(p0);
//        
//        Point p1;
//        p1.x = x1;
//        p1.y = y1;
//        p0_->push_back(p1);
//    }
}

void *SkeletonController::trackSkeleton() {
    
    nite::Status status;
    nite::UserTrackerFrameRef frame;
    
#pragma mark TEST ME!
    status = userTracker_.readFrame(&frame);
    frameWidth_ = frame.getDepthFrame().getWidth();
    frameHeight_ = frame.getDepthFrame().getHeight();
    
    generateRegionBoundaries();
    
//    printf("Device open = %s\n", deviceOpen_ ? "true" : "false");
    
    while (!shouldStop_) {
        
        /* Read the next frame */
        status = userTracker_.readFrame(&frame);
        if (status != nite::STATUS_OK)
        {
            printf("%s: Get next frame failed\n", __PRETTY_FUNCTION__);
            continue;
        }
        
        /* Get all users in the frame */
        const nite::Array<nite::UserData> &users = frame.getUsers();
        
        /* For each user */
        for (int i = 0; i < users.getSize(); ++i) {
            
            const nite::UserData &user = users[i];
            
            if (user.isLost()) {
                display_->clearUser();
                printf("User lost!\n");
                
                if (sendOsc_)
                    sendAllNotesOff();
            }
            
            if (!user.isVisible()) {
                display_->clearUser();
                
                /* Only send allnotesoff when user just steps out of frame */
                if (userInFrame_) {
                    if (sendOsc_)
                        sendAllNotesOff();
                    userInFrame_ = false;
                }
            }
            else {
                display_->setDrawUser();
                userInFrame_ = true;
            }
            
            if (user.isNew()) {
                userTracker_.startSkeletonTracking(user.getId());
                display_->setDrawUser();
                estimateHeight(user.getSkeleton());
                userInFrame_ = true;
                printf("New User!\n");
            }
            
            /* Check confidence of each joint and update if above a threshold */
            else if (user.getSkeleton().getState() == nite::SKELETON_TRACKED) {
                
                float x, y;     // Depth-scaled joint coordinates
//                printf("... frame...\n");
                
//                estimateHeight(user.getSkeleton());
                
                frameWidth_  = frame.getDepthFrame().getWidth();
                frameHeight_ = frame.getDepthFrame().getHeight();
                
                /* Head */
                nite::SkeletonJoint head = user.getSkeleton().getJoint(nite::JOINT_HEAD);
                if (head.getPositionConfidence() > confThresh_) {
                    userTracker_.convertJointCoordinatesToDepth(head.getPosition().x, head.getPosition().y, head.getPosition().z, &x, &y);
                    display_->updateJoint(nite::JOINT_HEAD, x, y, frameWidth_, frameHeight_);
                }
                
                /* Neck */
                nite::SkeletonJoint neck = user.getSkeleton().getJoint(nite::JOINT_NECK);
                if (neck.getPositionConfidence() > confThresh_) {
                    userTracker_.convertJointCoordinatesToDepth(neck.getPosition().x, neck.getPosition().y, neck.getPosition().z, &x, &y);
                    display_->updateJoint(nite::JOINT_NECK, x, y, frameWidth_, frameHeight_);
                }
                
                /* Torso */
                nite::SkeletonJoint torso = user.getSkeleton().getJoint(nite::JOINT_TORSO);
                if (torso.getPositionConfidence() > confThresh_) {
                    userTracker_.convertJointCoordinatesToDepth(torso.getPosition().x, torso.getPosition().y, torso.getPosition().z, &x, &y);
                    display_->updateJoint(nite::JOINT_TORSO, x, y, frameWidth_, frameHeight_);
                }
                
                /* Shoulders */
                nite::SkeletonJoint shoulderL = user.getSkeleton().getJoint(nite::JOINT_LEFT_SHOULDER);
                if (shoulderL.getPositionConfidence() > confThresh_) {
                    userTracker_.convertJointCoordinatesToDepth(shoulderL.getPosition().x, shoulderL.getPosition().y, shoulderL.getPosition().z, &x, &y);
                    display_->updateJoint(nite::JOINT_LEFT_SHOULDER, x, y, frameWidth_, frameHeight_);
                }
                
                nite::SkeletonJoint shoulderR = user.getSkeleton().getJoint(nite::JOINT_RIGHT_SHOULDER);
                if (shoulderR.getPositionConfidence() > confThresh_) {
                    userTracker_.convertJointCoordinatesToDepth(shoulderR.getPosition().x, shoulderR.getPosition().y, shoulderR.getPosition().z, &x, &y);
                    display_->updateJoint(nite::JOINT_RIGHT_SHOULDER, x, y, frameWidth_, frameHeight_);
                }
                
                /* Elbows */
                nite::SkeletonJoint elbowL = user.getSkeleton().getJoint(nite::JOINT_LEFT_ELBOW);
                if (elbowL.getPositionConfidence() > confThresh_) {
                    userTracker_.convertJointCoordinatesToDepth(elbowL.getPosition().x, elbowL.getPosition().y, elbowL.getPosition().z, &x, &y);
                    display_->updateJoint(nite::JOINT_LEFT_ELBOW, x, y, frameWidth_, frameHeight_);
                }
                
                nite::SkeletonJoint elbowR = user.getSkeleton().getJoint(nite::JOINT_RIGHT_ELBOW);
                if (elbowR.getPositionConfidence() > confThresh_) {
                    userTracker_.convertJointCoordinatesToDepth(elbowR.getPosition().x, elbowR.getPosition().y, elbowR.getPosition().z, &x, &y);
                    display_->updateJoint(nite::JOINT_RIGHT_ELBOW, x, y, frameWidth_, frameHeight_);
                }
                
                /* Hands */
                nite::SkeletonJoint handL = user.getSkeleton().getJoint(nite::JOINT_LEFT_HAND);
                if (handL.getPositionConfidence() > confThresh_) {
                    userTracker_.convertJointCoordinatesToDepth(handL.getPosition().x, handL.getPosition().y, handL.getPosition().z, &x, &y);
                    display_->updateJoint(nite::JOINT_LEFT_HAND, x, y, frameWidth_, frameHeight_);
                }
                
                nite::SkeletonJoint handR = user.getSkeleton().getJoint(nite::JOINT_RIGHT_HAND);
                if (handR.getPositionConfidence() > confThresh_) {
                    userTracker_.convertJointCoordinatesToDepth(handR.getPosition().x, handR.getPosition().y, handR.getPosition().z, &x, &y);
                    display_->updateJoint(nite::JOINT_RIGHT_HAND, x, y, frameWidth_, frameHeight_);
                    
                    if (sendOsc_) {
                        /* Send hand spacing message only if prediction is confident for both hands */
                        if (handL.getPositionConfidence() > confThresh_) {
                            trackHands(handL, handR);
                        }
                    }
                }
                
                /* Hips */
                nite::SkeletonJoint hipL = user.getSkeleton().getJoint(nite::JOINT_LEFT_HIP);
                if (hipL.getPositionConfidence() > confThresh_) {
                    userTracker_.convertJointCoordinatesToDepth(hipL.getPosition().x, hipL.getPosition().y, hipL.getPosition().z, &x, &y);
                    display_->updateJoint(nite::JOINT_LEFT_HIP, x, y, frameWidth_, frameHeight_);
                }
                
                nite::SkeletonJoint hipR = user.getSkeleton().getJoint(nite::JOINT_RIGHT_HIP);
                if (hipR.getPositionConfidence() > confThresh_) {
                    userTracker_.convertJointCoordinatesToDepth(hipR.getPosition().x, hipR.getPosition().y, hipR.getPosition().z, &x, &y);
                    display_->updateJoint(nite::JOINT_RIGHT_HIP, x, y, frameWidth_, frameHeight_);
                }
                
                /* Feet */
                nite::SkeletonJoint footL = user.getSkeleton().getJoint(nite::JOINT_LEFT_FOOT);
                if (footL.getPositionConfidence() > confThresh_) {
                    userTracker_.convertJointCoordinatesToDepth(footL.getPosition().x, footL.getPosition().y, footL.getPosition().z, &x, &y);
                    display_->updateJoint(nite::JOINT_LEFT_FOOT, x, y, frameWidth_, frameHeight_);
                    
                    if (sendOsc_) {
                        trackFoot("L", x);
                    }
                }
                
                nite::SkeletonJoint footR = user.getSkeleton().getJoint(nite::JOINT_RIGHT_FOOT);
                if (footR.getPositionConfidence() > confThresh_) {
                    userTracker_.convertJointCoordinatesToDepth(footR.getPosition().x, footR.getPosition().y, footR.getPosition().z, &x, &y);
                    display_->updateJoint(nite::JOINT_RIGHT_FOOT, x, y, frameWidth_, frameHeight_);
                    
                    if (sendOsc_) {
                        trackFoot("R", x);
                    }
                }
                
                /* Knees */
                nite::SkeletonJoint kneeL = user.getSkeleton().getJoint(nite::JOINT_LEFT_KNEE);
                if (kneeL.getPositionConfidence() > confThresh_) {
                    userTracker_.convertJointCoordinatesToDepth(kneeL.getPosition().x, kneeL.getPosition().y, kneeL.getPosition().z, &x, &y);
                    display_->updateJoint(nite::JOINT_LEFT_KNEE, x, y, frameWidth_, frameHeight_);
                }
                
                nite::SkeletonJoint kneeR = user.getSkeleton().getJoint(nite::JOINT_RIGHT_KNEE);
                if (kneeR.getPositionConfidence() > confThresh_) {
                    userTracker_.convertJointCoordinatesToDepth(kneeR.getPosition().x, kneeR.getPosition().y, kneeR.getPosition().z, &x, &y);
                    display_->updateJoint(nite::JOINT_RIGHT_KNEE, x, y, frameWidth_, frameHeight_);
                    if (footL.getPositionConfidence() > confThresh_) {
                        if (sendOsc_) {
                            /* Right-knee height mapping */
                            estimateHeight(user.getSkeleton());
                            trackRightKnee(kneeR, footL);
                        }
                    }
                }
            } /* If skeleton is tracked */
        } /* For each user */
    } /* while (shouldStop_) */

    return 0;
}

void SkeletonController::estimateHeight(nite::Skeleton skeleton) {
    
    // TO DO: add height estimates to a vector until the values converge
    
//    if (heightEst_.empty())
//        heightEst_.push_back(1000.0);
//    
    float newEst_;
    
    newEst_  = getJointDistance(skeleton.getJoint(nite::JOINT_HEAD),
                                skeleton.getJoint(nite::JOINT_NECK));
    newEst_ += getJointDistance(skeleton.getJoint(nite::JOINT_NECK),
                                skeleton.getJoint(nite::JOINT_TORSO));
    newEst_ += getJointDistance(skeleton.getJoint(nite::JOINT_TORSO),
                                skeleton.getJoint(nite::JOINT_RIGHT_HIP));
    newEst_ += getJointDistance(skeleton.getJoint(nite::JOINT_TORSO),
                                skeleton.getJoint(nite::JOINT_RIGHT_HIP));
    newEst_ += getJointDistance(skeleton.getJoint(nite::JOINT_RIGHT_HIP),
                                skeleton.getJoint(nite::JOINT_RIGHT_KNEE));
    newEst_ += getJointDistance(skeleton.getJoint(nite::JOINT_RIGHT_KNEE),
                                skeleton.getJoint(nite::JOINT_RIGHT_FOOT));
    userHeight_ = newEst_;
}

void SkeletonController::trackFoot(const char *LR, float x) {
    
    /* Mirror the x coordinate */
    x = frameWidth_ - x;
    
    float inc = frameWidth_ / 12;
    int region;
    
    if (x < 6*inc) {
        
        if (x < 3*inc) {
            
            if (x < 2*inc) {
                
                if (x < inc)
                    region = 0;
                else
                    region = 1;
            }
            else
                region = 2;
            
        }
        else /* (3*inc < x < 6*inc) */ {
            
            if (x < 5*inc) {
                
                if (x < 4*inc)
                    region = 3;
                else
                    region = 4;
            }
            else
                region = 5;
        }
    }
    else /* (x > 6*inc) */{
        
        if (x < 9*inc) {
            
            if (x < 8*inc) {
                
                if (x < 7*inc)
                    region = 6;
                else
                    region = 7;
            }
            else
                region = 8;
            
        }
        else /* (9*inc < x < 11*inc) */ {
            
            if (x < 11*inc) {
                
                if (x < 10*inc)
                    region = 9;
                else
                    region = 10;
                
            }
            else
                region = 11;
        }
    }
    
    /* Checking if left foot has moved to a new region */
    if (strcmp(LR, "L") != 0) {
        
        if (footRegion_[0] != region && footRegion_[1] != region) {
            /* Note off == note on with velocity = 0 */
            sendNoteOn(noteMap_[footRegion_[0]], 0);
            footRegion_[0] = region;
            sendNoteOn(noteMap_[footRegion_[0]], 90);
            sendIntensity(noteMap_[footRegion_[0]], 1.0f);
        }
    }
    /* Checking right foot */
    else if (strcmp(LR, "R") != 0) {
        
        if (footRegion_[1] != region && footRegion_[0] != region) {
            /* Note off == note on with velocity = 0 */
            sendNoteOn(noteMap_[footRegion_[1]], 0);
            footRegion_[1] = region;
            sendNoteOn(noteMap_[footRegion_[1]], 90);
            sendIntensity(noteMap_[footRegion_[1]], 1.0f);
        }
    }
    else
        printf("%s: Specify \"L\" or \"R\" only\n", __PRETTY_FUNCTION__);
    
}

void SkeletonController::trackHands(nite::SkeletonJoint left, nite::SkeletonJoint right) {
    
    float distance = getJointDistance(left, right);
    
    sendIntensity(noteMap_[footRegion_[0]], distance / 1800);
    sendIntensity(noteMap_[footRegion_[1]], distance / 1800);
}

void SkeletonController::trackRightKnee(nite::SkeletonJoint knee, nite::SkeletonJoint foot) {
    
//    float height = 0;
    
//    float brightRange = 3 * getJointDistance(knee, head);
    float value = 3*abs(knee.getPosition().y - foot.getPosition().y) / userHeight_;
    
    if (value < 0) value = 0;
    if (value > 1) value = 1;
    
    value = 1 - value;
    
    sendBrightness(noteMap_[footRegion_[0]], value);
    sendBrightness(noteMap_[footRegion_[1]], value);
}

void SkeletonController::sendNoteOn(int noteNumber, int velocity) {
    
    oscSender_->sendMessage("/mrp/midi", "iii", 144, noteNumber, velocity, LO_ARGS_END);
    kbDisplay_->setHighlightedKey(noteNumber, velocity == 0 ? false : true);
    
    if (velocity == 0)
        kbDisplay_->setAnalogValueForKey(noteNumber, 0);
}

void SkeletonController::sendIntensity(int noteNumber, float value) {
    
    oscSender_->sendMessage("/mrp/quality/intensity", "iif", 0, noteNumber, value, LO_ARGS_END);
    kbDisplay_->setAnalogValueForKey(noteNumber, value);
}

void SkeletonController::sendBrightness(int noteNumber, float value) {
    
    oscSender_->sendMessage("/mrp/quality/brightness", "iif", 0, noteNumber, value, LO_ARGS_END);
}

void SkeletonController::sendAllNotesOff() {
    
    oscSender_->sendMessage("/mrp/allnotesoff");
    kbDisplay_->clearAnalogData();
    kbDisplay_->clearHighlightedKeys();
}

float SkeletonController::getJointDistance(nite::SkeletonJoint j1, nite::SkeletonJoint j2) {
    
    return sqrt(    powf(abs(j1.getPosition().x - j2.getPosition().x), 2) +
                    powf(abs(j1.getPosition().y - j2.getPosition().y), 2) +
                    powf(abs(j1.getPosition().z - j2.getPosition().z), 2)
                );
}




