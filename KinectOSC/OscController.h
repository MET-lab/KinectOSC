//
//  OscController.h
//  KinectOSC
//
//  Created by Jeff Gregorio on 2/20/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#ifndef __KinectOSC__OscController__
#define __KinectOSC__OscController__

#include <iostream>
#include <vector>

#include "lo/lo.h"

class OscController {
    
public:
    
    OscController() { hasAddress_ = false; }
    ~OscController() {}

    void setServerAddress(const char *host, const char *port);
    void enableLogging()  { doLog_ = true; }
    void disableLogging() { doLog_ = false; }
    
    void sendMessage(const char *path);
    void sendMessage(const char *path, const char *types, ...);
    void sendMessage(const char *path, const char *types, const lo_message &message);
    
private:
    
    lo_address serverAddress_;
    bool hasAddress_;
    bool doLog_;
};

#endif /* defined(__KinectOSC__OscController__) */
