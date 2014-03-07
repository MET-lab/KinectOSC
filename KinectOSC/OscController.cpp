//
//  OscController.cpp
//  KinectOSC
//
//  Created by Jeff Gregorio on 2/20/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#include "OscController.h"

void OscController::setServerAddress(const char *host, const char *port) {
    
    if (hasAddress_)
        lo_address_free(serverAddress_);
        
    serverAddress_ = lo_address_new(host, port);
    hasAddress_ = true;   
}

void OscController::sendMessage(const char *path) {
    
    if (doLog_)
        printf("OSC: %s\n", path);
    
    lo_send(serverAddress_, path, NULL);
}

void OscController::sendMessage(const char *path, const char *types, ...) {
    
    va_list v;
	
	va_start(v, types);
	lo_message msg = lo_message_new();
	lo_message_add_varargs(msg, types, v);
    
    if (doLog_) {
        printf("OSC: %s\n     %s : ", path, types);
        
        lo_arg **args = lo_message_get_argv(msg);
        
        for(int i = 0; i < lo_message_get_argc(msg); i++) {
			switch(types[i]) {
				case 'i':
					printf("%d ", args[i]->i);
					break;
				case 'f':
					printf("%f ", args[i]->f);
					break;
				default:
					printf("? ");
			}
		}
        printf("\n");
    }
    
    sendMessage(path, types, msg);
    
	lo_message_free(msg);
	va_end(v);
}

void OscController::sendMessage(const char *path, const char *types, const lo_message &message) {
    
    lo_send_message(serverAddress_, path, message);
}