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
    Camera camera( simd_make_int2( 400, 300 ) );
    
    // Create a scene with a singular sphere in it
    Shape sphere(0.5);
    sphere.setPosition( simd_make_float3( 0, 0, -1 ) );
    
    Scene scene;
    scene.shapes.push_back(sphere);
    
    // Do any additional setup after loading the view.
    _raytracer = new Raytracer( camera, scene );
    
    // Start rendering right away
    _raytracer->renderAsync();
    
    // Start a timer that tries to sync a preview every second or so..
    _syncTimer = [NSTimer scheduledTimerWithTimeInterval: 1 repeats: true block: ^(NSTimer *timer) {
        
        // Retain self...
        Raytracer* raytracer = self->_raytracer;
        
        // Get latest image..
        CGImageRef progressImage = raytracer->copyRenderImage();
        [self->_raytracerView updateImage: progressImage];
        CGImageRelease(progressImage);
        
        // If we're truely done, stop asking to update the backing image..
        if( raytracer->isComplete() )
        {
            [self->_syncTimer invalidate];
            self->_syncTimer = nil;
        }
    }];
}

- (void) dealloc
{
    delete _raytracer;
}

@end
