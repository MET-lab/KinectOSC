//
//  KinectDisplay.cpp
//  KinectOSC
//
//  Created by Jeff Gregorio on 2/19/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#include "KinectDisplay.h"

KinectDisplay::KinectDisplay() {
    
    /* Initialize the mutex with default parameters */
    pthread_mutex_init(&displayMutex_, NULL);
    
    needsRender_ = true;
    displayPixelWidth_ = 0;
    displayPixelHeight_ = 0;
    
    drawRegions_ = true;
}

KinectDisplay::~KinectDisplay() {
    
    /* Destroy the display mutey */
    pthread_mutex_destroy(&displayMutex_);
}

void KinectDisplay::setDisplaySize(float width, float height) {
    
    pthread_mutex_lock(&displayMutex_);
    
    displayPixelWidth_ = width;
    displayPixelHeight_ = height;
    
    glViewport(0, 0, displayPixelWidth_, displayPixelHeight_);
    
    pthread_mutex_unlock(&displayMutex_);
    
}

/* Update the joint positions internal to this class, scaling to the interval [-1, 1] for the OpenGL drawing */
void KinectDisplay::updateJoint(nite::JointType jointType, float x, float y, float frameWidth, float frameHeight) {
    
    /* Mirrored coordinates */
    float xM = frameWidth  - x;
    float yM = frameHeight - y;
    
    /* Mapped to [-1, 1] */
    float rX = mapToInterval(xM, 0, frameWidth,  -1, 1);
    float rY = mapToInterval(yM, 0, frameHeight, -1, 1);
    
    switch (jointType) {
        case nite::JOINT_HEAD:
            userSkeleton_.head.x = rX;
            userSkeleton_.head.y = rY;
            break;
            
        case nite::JOINT_NECK:
            userSkeleton_.neck.x = rX;
            userSkeleton_.neck.y = rY;
            break;
            
        case nite::JOINT_TORSO:
            userSkeleton_.torso.x = rX;
            userSkeleton_.torso.y = rY;
            break;
            
        case nite::JOINT_LEFT_SHOULDER:
            userSkeleton_.shoulderL.x = rX;
            userSkeleton_.shoulderL.y = rY;
            break;
            
        case nite::JOINT_RIGHT_SHOULDER:
            userSkeleton_.shoulderR.x = rX;
            userSkeleton_.shoulderR.y = rY;
            break;
            
        case nite::JOINT_LEFT_ELBOW:
            userSkeleton_.elbowL.x = rX;
            userSkeleton_.elbowL.y = rY;
            break;
            
        case nite::JOINT_RIGHT_ELBOW:
            userSkeleton_.elbowR.x = rX;
            userSkeleton_.elbowR.y = rY;
            break;
            
        case nite::JOINT_LEFT_HAND:
            userSkeleton_.handL.x = rX;
            userSkeleton_.handL.y = rY;
            break;
            
        case nite::JOINT_RIGHT_HAND:
            userSkeleton_.handR.x = rX;
            userSkeleton_.handR.y = rY;
            break;
            
        case nite::JOINT_LEFT_HIP:
            userSkeleton_.hipL.x = rX;
            userSkeleton_.hipL.y = rY;
            break;
            
        case nite::JOINT_RIGHT_HIP:
            userSkeleton_.hipR.x = rX;
            userSkeleton_.hipR.y = rY;
            break;
            
        case nite::JOINT_LEFT_KNEE:
            userSkeleton_.kneeL.x = rX;
            userSkeleton_.kneeL.y = rY;
            break;
            
        case nite::JOINT_RIGHT_KNEE:
            userSkeleton_.kneeR.x = rX;
            userSkeleton_.kneeR.y = rY;
            break;
            
        case nite::JOINT_LEFT_FOOT:
            userSkeleton_.footL.x = rX;
            userSkeleton_.footL.y = rY;
            break;
            
        case nite::JOINT_RIGHT_FOOT:
            userSkeleton_.footR.x = rX;
            userSkeleton_.footR.y = rY;
            break;
            
        default:
            break;
    }
    
    needsRender_ = true;
}

void KinectDisplay::render() {
    
    pthread_mutex_lock(&displayMutex_);
    
    /* Dark black background */
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (drawRegions_)
        drawRegions();
    
    if (drawUser_) {
        drawLimb(userSkeleton_.head, userSkeleton_.neck);
        drawLimb(userSkeleton_.neck, userSkeleton_.torso);
        drawLimb(userSkeleton_.neck, userSkeleton_.shoulderL);
        drawLimb(userSkeleton_.neck, userSkeleton_.shoulderR);
        drawLimb(userSkeleton_.shoulderL, userSkeleton_.elbowL);
        drawLimb(userSkeleton_.shoulderR, userSkeleton_.elbowR);
        drawLimb(userSkeleton_.elbowL, userSkeleton_.handL);
        drawLimb(userSkeleton_.elbowR, userSkeleton_.handR);
        drawLimb(userSkeleton_.torso, userSkeleton_.hipL);
        drawLimb(userSkeleton_.torso, userSkeleton_.hipR);
        drawLimb(userSkeleton_.hipL, userSkeleton_.kneeL);
        drawLimb(userSkeleton_.hipR, userSkeleton_.kneeR);
        drawLimb(userSkeleton_.kneeL, userSkeleton_.footL);
        drawLimb(userSkeleton_.kneeR, userSkeleton_.footR);
    }
    
    else {
//        glEnd();
    }
    
    needsRender_ = false;
    glFlush();
    pthread_mutex_unlock(&displayMutex_);
}

void KinectDisplay::drawLimb(Joint j1, Joint j2) {
    
    /* Draw white lines */
    glPolygonMode(GL_FRONT, GL_LINE);
    glColor3f(1.0, 1.0, 1.0);
    
    glBegin(GL_LINES);
    glVertex2f(j1.x, j1.y);
    glVertex2f(j2.x, j2.y);
    glEnd();
}

void KinectDisplay::drawRegions() {
    
//    /* Draw white lines */
//    glPolygonMode(GL_FRONT, GL_LINE);
//    glColor3f(1.0, 1.0, 1.0);
//    
//    for (int i = 0; i <= 12; i++) {
//        
//        glBegin(GL_LINES);
//        glVertex2f(p0_[i].x, p0_[i].y);
//        glVertex2f(p0_[i+1].x, p0_[i+1].y);
//        glVertex2f(p1_[i+1].x, p1_[i+1].y);
//        glVertex2f(p1_[i].x, p1_[i].y);
//        glEnd();
//    }
}

















