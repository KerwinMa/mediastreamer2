/*
 mediastreamer2 library - modular sound and video processing and streaming
 Copyright (C) 2006-2014 Belledonne Communications, Grenoble

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#if TARGET_OS_IPHONE

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CFRunLoop.h>
#include "mediastreamer2_tester.h"


int g_argc;
char** g_argv;
void stop_handler(int sig) {
    return;
}

static void* _apple_main(void* data) {
    NSString *bundlePath = [[[NSBundle mainBundle] bundlePath] retain];
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentPath = [[paths objectAtIndex:0] retain];
    NSLog(@"Bundle path: %@", bundlePath);
    NSLog(@"Document path: %@", documentPath);

    mediastreamer2_tester_set_file_root( [bundlePath UTF8String] );
    mediastreamer2_tester_set_writable_dir( [documentPath UTF8String] );
    apple_main(g_argc,g_argv);

    [bundlePath release];
    [documentPath release];
    return NULL;
}
int main(int argc, char * argv[]) {
    pthread_t main_thread;
    g_argc=argc;
    g_argv=argv;
    pthread_create(&main_thread,NULL,_apple_main,NULL);
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    int value = UIApplicationMain(0, nil, nil, nil);
    [pool release];
    return value;
    pthread_join(main_thread,NULL);
    return 0;
}


#endif // target IPHONE