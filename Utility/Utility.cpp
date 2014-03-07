//
//  Utility.cpp
//  KinectOSC
//
//  Created by Jeff Gregorio on 2/21/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#include "Utility.h"

float mapToInterval(float &x, float min0, float max0, float min1, float max1) {
    
    return x * ((max1 - min1) / (max0 - min0)) + (min1 - min0);
}

std::vector<double> linspace(double min, double max, int n)
{
    std::vector<double> result;
    // vector iterator
    int iterator = 0;
    
    for (int i = 0; i <= n-2; i++)
    {
        double temp = min + i*(max-min)/(floor((double)n) - 1);
        result.insert(result.begin() + iterator, temp);
        iterator += 1;
    }
    
    //iterator += 1;
    
    result.insert(result.begin() + iterator, max);
    return result;
}