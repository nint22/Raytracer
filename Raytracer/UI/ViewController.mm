//
//  ViewController.m
//  Raytracer
//
//  Created by Jeremy Bridon on 4/25/20.
//  Copyright © 2020 Jeremy Bridon. All rights reserved.
//

#import "ViewController.h"

#import "Raytracer.h"

@interface ViewController ()
{
    Raytracer* _raytracer;
    NSTimer* _syncTimer;
}
@end

@implementation ViewController

- (void) viewDidLoad
{
    [super viewDidLoad];

    // Setup our scene
    Camera camera( simd_make_int2( 800, 600 ) );
    
    // Do any additional setup after loading the view.
    _raytracer = new Raytracer( camera );
    
    // Start rendering right away
    _raytracer->renderAsync();
    
    // Start a timer that tries to sync a preview every second or so..
    _syncTimer = [NSTimer scheduledTimerWithTimeInterval: 1 repeats: true block: ^(NSTimer *timer) {
        // Todo: handle completion to avoid syncing after done
        CGImageRef progressImage = self->_raytracer->copyRenderImage();
        [self->_raytracerView updateImage: progressImage];
        CGImageRelease(progressImage);
    }];
}

- (void) dealloc
{
    delete _raytracer;
}

@end
