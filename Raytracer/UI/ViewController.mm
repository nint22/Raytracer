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

    // Camera position
    float3 cameraPos = simd_make_float3( 13, 2, 3 );
    float3 cameraTarget = simd_make_float3( 0, 0, 0 );
    float3 cameraUp = simd_make_float3( 0, -1, 0 );
    
    // Defocus blur
    float aperature = 0.1;
    float distanceToFocus = 10;
    
    // Setup a camera
    Camera camera( simd_make_int2( 1600, 800 ), cameraPos, cameraTarget, cameraUp, 20, aperature, distanceToFocus );
    camera.setSampleCount( 200 );
    camera.setMaxBounceCount( 50 );
    
    // Create a scene
    Scene scene;
    
    // Sphere that looks like ground
    Sphere* sphere = new Sphere(1000);
    sphere->setPosition( simd_make_float3( 0, -1000, -1 ) );
    sphere->setMaterial( new LambertianMaterial( simd_make_float3( 0.5, 0.5, 0.5 ) ) );
    scene.shapes.push_back(sphere);

    // Glass sphere
    sphere = new Sphere( 1.0 );
    sphere->setPosition( simd_make_float3( 0, 1, 0 ) );
    sphere->setMaterial( new DielectricMaterial( 1.5 ) );
    scene.shapes.push_back(sphere);

    // Metalic spheres
    sphere = new Sphere( 1.0 );
    sphere->setPosition( simd_make_float3( -4, 1, 0 ) );
    sphere->setMaterial( new LambertianMaterial( random_float3() * random_float3() ) );
    scene.shapes.push_back(sphere);

    // Diffuse
    sphere = new Sphere( 1.0 );
    sphere->setPosition( simd_make_float3( 4, 1, 0 ) );
    sphere->setMaterial( new MetalMaterial( simd_make_float3( 0.7, 0.6, 0.5 ), 0.0 ) );
    scene.shapes.push_back(sphere);
    
    // Add a pair of light
    sphere = new Sphere( 0.5 );
    sphere->setPosition( simd_make_float3( 0.5, 2, 0.5 ) );
    sphere->setMaterial( new DiffuseLightMaterial( simd_make_float3( 4, 4, 4 ) ) );
    scene.shapes.push_back(sphere);
    
    sphere = new Sphere( 0.5 );
    sphere->setPosition( simd_make_float3( 2, 2, 2 ) );
    sphere->setMaterial( new DiffuseLightMaterial( simd_make_float3( 4, 4, 4 ) ) );
    scene.shapes.push_back(sphere);
    
    // Create a bunch of random spheres..
    for( int y = -11; y < 11; y++ )
    {
        for( int x = -11; x < 11; x++ )
        {
            const float3 position = simd_make_float3( x + 0.9 * random_float(), 0.2, y + 0.9 * random_float() );
            if( simd_length( position - simd_make_float3( 4, 0.2, 0 ) ) > 0.9 )
            {
                const float materialChoice = random_float();
                if( materialChoice < 0.8 )
                {
                    // Diffuse
                    sphere = new Sphere( 0.2 );
                    sphere->setPosition( position );
                    sphere->setMaterial( new LambertianMaterial( random_float3() * random_float3() ) );
                    scene.shapes.push_back(sphere);
                }
                else if( materialChoice < 0.95 )
                {
                    // Metalic spheres
                    sphere = new Sphere( 0.2 );
                    sphere->setPosition( position );
                    sphere->setMaterial( new MetalMaterial( random_float3( 0.5, 1.0 ), random_float( 0.0, 0.5 ) ) );
                    scene.shapes.push_back(sphere);
                }
                else
                {
                    // Glass sphere
                    sphere = new Sphere( 0.2 );
                    sphere->setPosition( position );
                    sphere->setMaterial( new DielectricMaterial( 1.5 ) );
                    scene.shapes.push_back(sphere);
                }
            }
        }
    }
    
    // Do any additional setup after loading the view.
    _raytracer = new Raytracer( camera, scene );
    
    // Start rendering right away
    _raytracer->renderAsync();
    
    // Start a timer that tries to sync a preview every second or so..
    _syncTimer = [NSTimer scheduledTimerWithTimeInterval: 0.2 repeats: true block: ^(NSTimer *timer) {
        
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
            
            // Save image out to /tmp/raytracing.png
            CGImageRef image = raytracer->copyRenderImage();
            
            CFURLRef url = (__bridge CFURLRef)[NSURL fileURLWithPath: @"/tmp/raytracing.png"];
            CGImageDestinationRef destination = CGImageDestinationCreateWithURL( url, kUTTypePNG, 1, NULL );
            CGImageDestinationAddImage( destination, image, NULL );
            
            if( CGImageDestinationFinalize( destination ) == false )
            {
                NSLog( @"Failed to write image!" );
                CFRelease( destination );
                CGImageRelease( image );
                return;
            }
            
            CFRelease( destination );
            CGImageRelease( image );
        }
    }];
}

- (void) dealloc
{
    delete _raytracer;
}

@end
