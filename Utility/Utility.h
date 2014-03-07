//
//  Utility.h
//  KinectOSC
//
//  Created by Jeff Gregorio on 2/21/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#ifndef __KinectOSC__Utility__
#define __KinectOSC__Utility__

#include <iostream>
#include <vector>
#include <cmath>

/* Linearly map a number generated from the interval [min0 max0] to its corresponding value on the interval [min1 max1] */
float mapToInterval(float &x, float min0, float max0, float min1, float max1);

std::vector<double> linspace(double min, double max, int n);

#endif /* defined(__KinectOSC__Utility__) */
