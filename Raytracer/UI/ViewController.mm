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

    // Setup a camera
    Camera camera( simd_make_int2( 600, 300 ) );
    camera.setSampleCount( 100 );
    camera.setMaxBounceCount( 50 );
    
    // Setup our scene
    Scene scene;
    
    // Dead-center sphere
    Sphere* sphere = new Sphere(0.5);
    sphere->setPosition( simd_make_float3( 0, 0, -1 ) );
    sphere->setMaterial( new LambertianMaterial( simd_make_float3( 0.7, 0.3, 0.3 ) ) );
    scene.shapes.push_back(sphere);
    
    // Sphere that looks like ground
    sphere = new Sphere(100);
    sphere->setPosition( simd_make_float3( 0, -100.5, -1 ) );
    sphere->setMaterial( new LambertianMaterial( simd_make_float3( 0.8, 0.8, 0.0 ) ) );
    scene.shapes.push_back(sphere);
    
    // Metalic spheres
    sphere = new Sphere(0.5);
    sphere->setPosition( simd_make_float3( 1, 0, -1 ) );
    sphere->setMaterial( new MetalMaterial( simd_make_float3( 0.8, 0.6, 0.2 ), 0.5 ) );
    scene.shapes.push_back(sphere);
    
    sphere = new Sphere(0.5);
    sphere->setPosition( simd_make_float3( -1, 0, -1 ) );
    sphere->setMaterial( new MetalMaterial( simd_make_float3( 0.8, 0.8, 0.8 ), 1.0 ) );
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
            // Change window title
            NSWindow* window = [[self view] window];
            [window setTitle: @"Complete!"];
            
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
