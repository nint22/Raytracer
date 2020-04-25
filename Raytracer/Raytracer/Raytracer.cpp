//
//  Raytracer.cpp
//  Raytracer
//
//  Created by Jeremy Bridon on 4/25/20.
//  Copyright © 2020 Jeremy Bridon. All rights reserved.
//

#include "Raytracer.h"

#pragma mark Ray Struct

float3 Ray::at(float t) const
{
    return pos + simd_normalize(dir) * t;
}

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
    
    _backingBuffer = new float4[ camera.resolution().x * camera.resolution().y ];
    _backingBufferLock = OS_UNFAIR_LOCK_INIT;
    
    _workQueue = dispatch_queue_create( "Raytracer queue", dispatch_queue_attr_make_with_qos_class( 0, QOS_CLASS_USER_INITIATED, 0 ) );
    _workLock = OS_UNFAIR_LOCK_INIT;
}

Raytracer::~Raytracer()
{
    delete[] _backingBuffer;
}

void Raytracer::renderAsync()
{
    // Clear our backing buffer
    const size_t backingBufferLength = sizeof( float4 ) * _camera.resolution().x * _camera.resolution().y;
    memset( _backingBuffer, 0, backingBufferLength );
    
    // For now let's define some constants that will eventually come from the camera
    const float3 lower_left_corner = simd_make_float3(-2.0, -1.0, -1.0);
    const float3 horizontal = simd_make_float3(4.0, 0.0, 0.0);
    const float3 vertical = simd_make_float3(0.0, 2.0, 0.0);
    const float3 origin = simd_make_float3(0.0, 0.0, 0.0);
    const float2 f2Resolution = simd_make_float2( _camera.resolution().x, _camera.resolution().y );
    
    // Create all the work we want to complete
    for( int y = 0; y < _camera.resolution().y; y++ )
    {
        for( int x = 0; x < _camera.resolution().x; x++ )
        {
            // Work item at pixel...
            WorkItem workItem;
            workItem.pixelPos = simd_make_int2(x, y);
            
            // UV coordinate
            float2 uv = simd_make_float2(x, y) / f2Resolution;
            workItem.ray.pos = origin;
            workItem.ray.dir = lower_left_corner + uv.x * horizontal + uv.y * vertical;
            
            // Push work
            _workItems.push_back(workItem);
        }
    }
    
    // Async all the work
    dispatch_async(_workQueue, ^{
        dispatch_apply(_workItems.size(), NULL, ^(size_t) {
            
            // Get work
            os_unfair_lock_lock(&_workLock);
            WorkItem workItem = _workItems.back();
            _workItems.pop_back();
            os_unfair_lock_unlock(&_workLock);
            
            // Do work
            float4 color = work(&workItem);
            
            // Store to our backing buffer
            os_unfair_lock_lock(&_backingBufferLock);
            const int x = workItem.pixelPos.x;
            const int y = workItem.pixelPos.y;
            _backingBuffer[ y * _camera.resolution().x + x ] = color;
            os_unfair_lock_unlock(&_backingBufferLock);
        });
    });
}

bool Raytracer::isComplete()
{
    return false;
}

CGImageRef Raytracer::copyRenderImage()
{
    // Create our image backing buffer.
    int2 resolution = _camera.resolution();
    uint32_t* imageBuffer = new uint32_t[ resolution.x * resolution.y ];
    
    // Convert float float4 to int4
    os_unfair_lock_lock(&_backingBufferLock);
    for( int y = 0; y < _camera.resolution().y; y++ )
    {
        for( int x = 0; x < _camera.resolution().x; x++ )
        {
            // Convert from normalized color to
            const int pixelIndex = y * _camera.resolution().x + x;
            
            float4 sourceColor = _backingBuffer[ pixelIndex ];
            
            uint8_t r = clamp( sourceColor.x * 255.0f, 0, 255 );
            uint8_t g = clamp( sourceColor.y * 255.0f, 0, 255 );
            uint8_t b = clamp( sourceColor.z * 255.0f, 0, 255 );
            uint8_t a = 255;
            
            // Back backwards: we're on little-endian architecture
            uint32_t destColor = ( r << 0 ) | ( g << 8 ) | ( b << 16 ) | ( a << 24 );
            
            imageBuffer[ pixelIndex ] = destColor;
        }
    }
    os_unfair_lock_unlock(&_backingBufferLock);
    
    // Could be done directly via CGImageCreate(...) but I prefer this method..
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName( kCGColorSpaceSRGB );
    CGContextRef context = CGBitmapContextCreate( imageBuffer, resolution.x, resolution.y, 8, resolution.x * 4, colorSpace, kCGImageAlphaNoneSkipLast );
    CGImageRef image = CGBitmapContextCreateImage( context );
    CGColorSpaceRelease( colorSpace );
    
    // Done with backing buffer, return image
    free( imageBuffer );
    return image;
}

float4 Raytracer::work(const WorkItem* workItem) const
{
    float3 dir = simd_normalize(workItem->ray.dir);
    float t = 0.5 * ( dir.y + 1.0 );
    float3 color = ( 1.0 - t ) * simd_make_float3(1, 1, 1) + t * simd_make_float3( 0.5, 0.7, 1.0 );
    return simd_make_float4( color, 1 );
}
