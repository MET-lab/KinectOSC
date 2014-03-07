//
//  KinectDisplay.h
//  KinectOSC
//
//  Created by Jeff Gregorio on 2/19/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#ifndef __KinectOSC__KinectDisplay__
#define __KinectOSC__KinectDisplay__

#include <iostream>
#include <vector>

#include <OpenGL/gl.h>
#include "NiTE.h"

#include "Utility.h"

#define GL_WIN_SIZE_X	1280
#define GL_WIN_SIZE_Y	1024

using namespace std;

/* Joint types */
enum {
    J_HEAD = 0,
    J_NECK,
    J_TORSO,
    J_SHOULDER_L,
    J_SHOULDER_R,
    J_ELBOW_L,
    J_ELBOW_R,
    J_HAND_L,
    J_HAND_R,
    J_HIP_L,
    J_HIP_R,
    J_KNEE_L,
    J_KNEE_R,
    J_FOOT_L,
    J_FOOT_R
};

class KinectDisplay {
    
    /* Skeleton joint positions scaled to interval [-1, 1] */
    struct Joint {
        float x;
        float y;
    };
    
    struct ScaledSkeleton {
        Joint head;
        Joint neck;
        Joint torso;
        Joint shoulderL;
        Joint shoulderR;
        Joint elbowL;
        Joint elbowR;
        Joint handL;
        Joint handR;
        Joint hipL;
        Joint hipR;
        Joint kneeL;
        Joint kneeR;
        Joint footL;
        Joint footR;
    };
    
public:
    
    KinectDisplay();
    ~KinectDisplay();
    
    /* Setters */
    void setDisplaySize(float width, float height);
    void updateJoint(nite::JointType jointType, float x, float y, float frameWidth, float frameHeight);
    void setDrawUser()   { drawUser_ = true; }
    void clearUser() { drawUser_ = false; needsRender_ = true; }
    
    /* Getters */
    bool needsRender() { return needsRender_; }
    
    /* Main render method */
    void render();
    
private:
    
    /* Render helper methods */
    void drawLimb(Joint j1, Joint j2);
    
    /* Draw the note region boundaries */
    void drawRegions();
    
private:
        
    ScaledSkeleton userSkeleton_;       // Internal skeleton joint positions scaled to interval [-1, 1
    
    float displayPixelWidth_;
    float displayPixelHeight_;
    
    bool needsRender_;                  //
    bool drawUser_;
    bool drawRegions_;
    
    pthread_mutex_t displayMutex_;      // Mutex needed to synchronize data access and display threads
    
};

#endif /* defined(__KinectOSC__KinectDisplay__) */
