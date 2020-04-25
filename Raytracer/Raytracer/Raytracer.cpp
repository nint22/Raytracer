//
//  Raytracer.cpp
//  Raytracer
//
//  Created by Jeremy Bridon on 4/25/20.
//  Copyright © 2020 Jeremy Bridon. All rights reserved.
//

#include "Raytracer.h"

#pragma mark Camera Class

Camera::Camera(int2 resolution)
{
    _resolution = resolution;
}

int2 Camera::resolution() const
{
    return _resolution;
}

#pragma mark Raytracer Class

Raytracer::Raytracer(const Camera& camera)
{
    _camera = camera;
}

Raytracer::~Raytracer()
{
    
}

void Raytracer::renderAsync()
{
    
}

bool Raytracer::isComplete()
{
    return false;
}

CGImageRef Raytracer::copyRenderImage()
{
    // TODO: Convert from float4 to int4, etc.
    
    // Create our image backing buffer.
    int2 resolution = _camera.resolution();
    uint32_t* backingBuffer = new uint32_t[ resolution.x * resolution.y ];
    
    // Could be done directly via CGImageCreate(...) but I prefer this method..
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName( kCGColorSpaceSRGB );
    CGContextRef context = CGBitmapContextCreate( backingBuffer, resolution.x, resolution.y, 8, resolution.x * 4, colorSpace, kCGImageAlphaNoneSkipLast );
    CGImageRef image = CGBitmapContextCreateImage( context );
    CGColorSpaceRelease( colorSpace );
    
    // Done with backing buffer, return image
    free( backingBuffer );
    return image;
}
