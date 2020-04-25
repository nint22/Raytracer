//
//  Raytracer.cpp
//  Raytracer
//
//  Created by Jeremy Bridon on 4/25/20.
//  Copyright © 2020 Jeremy Bridon. All rights reserved.
//

#include "Raytracer.h"

#include <limits>

#pragma mark Ray Struct

float3 Ray::at(float t) const
{
    return pos + simd_normalize(dir) * t;
}

#pragma mark Sphere Class

Shape::Shape(float radius)
{
    _type = Sphere;
    _radius = radius;
}

Shape::Type Shape::type() const
{
    return _type;
}

float3 Shape::position() const
{
    return _position;
}

void Shape::setPosition(const float3& p)
{
    _position = p;
}

float Shape::sphereRadius() const
{
    return _radius;
}

bool Shape::hitTest(const Ray& ray, float3* hitPosition, float3* hitNormal) const
{
    if( _type == Sphere )
    {
        float3 oc = ray.pos - _position;
        float3 dir = simd_normalize( ray.dir );
        float a = simd_length_squared( dir );
        float half_b = simd_dot( oc, dir );
        float c = simd_length_squared( oc ) - _radius * _radius;
        float discrim = half_b * half_b - a * c;
        if( discrim < 0.0 )
            return false;
        
        float t = ( -half_b - sqrt( discrim ) ) / a;
        float3 hit = ray.at( t );
        
        if( hitPosition != nullptr )
            *hitPosition = hit;
        
        float3 normal = simd_normalize( hit - simd_make_float3( 0, 0, -1 ) );
        if( hitNormal )
            *hitNormal = normal;
        
        return true;
    }
    else
    {
        assert( false ); // Not supported!
        return false;
    }
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

Raytracer::Raytracer(const Camera& camera, const Scene& scene)
{
    _camera = camera;
    _scene = scene;
    
    _backingBuffer = new float4[ camera.resolution().x * camera.resolution().y ];
    _backingBufferLock = OS_UNFAIR_LOCK_INIT;
    
    _workQueue = dispatch_queue_create( "Raytracer queue", dispatch_queue_attr_make_with_qos_class( 0, QOS_CLASS_USER_INITIATED, 0 ) );
    _workLock = OS_UNFAIR_LOCK_INIT;
    
    _state = Setup;
    _finalImage = nullptr;
}

Raytracer::~Raytracer()
{
    delete[] _backingBuffer;
    if( _finalImage != nullptr )
        CGImageRelease( _finalImage );
}

void Raytracer::renderAsync()
{
    // Ignore if we are no longer in setup state
    if( _state != Setup )
        return;
    
    // Declare we're going to be doing the work
    _state = Active;
    
    // Clear our backing buffer
    const size_t backingBufferLength = sizeof( float4 ) * _camera.resolution().x * _camera.resolution().y;
    memset( _backingBuffer, 0, backingBufferLength );
    
    // Define our horizontal field of view size as a specific unit size..
    const float horizontalFieldOfView = 4;
    const float verticalFieldOfView = ( (float)_camera.resolution().y / _camera.resolution().x ) * horizontalFieldOfView;
    
    // For now let's define some constants that will eventually come from the camera
    const float3 lower_left_corner = simd_make_float3(-horizontalFieldOfView / 2.0, verticalFieldOfView / 2.0, -1.0);
    const float3 horizontal = simd_make_float3(horizontalFieldOfView, 0.0, 0.0);
    const float3 vertical = simd_make_float3(0.0, -verticalFieldOfView, 0.0);
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
        
        // All work complete, post update on main thread. I'm aware of how nasty
        // this is with C++ code because nothing is retaining this object.. This is
        // also making a big assumption that the caller is always on the main thread.
        // Ew ew ew
        dispatch_async(dispatch_get_main_queue(), ^{
            _state = Complete;
        });
    });
}

bool Raytracer::isComplete()
{
    return ( _state == Complete && _finalImage != nullptr );
}

CGImageRef Raytracer::copyRenderImage()
{
    // If we're already done rendering *and* we have a cached final image...
    if( isComplete() )
    {
        // Retain per "copy" contract via function signature
        CGImageRetain( _finalImage );
        return _finalImage;
    }
    
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
    
    // Done with backing buffer
    free( imageBuffer );
    
    // If we're now complete, retain the final image
    if( _state == Complete )
    {
        _finalImage = image;
        CGImageRetain( _finalImage );
    }
    
    // Done with backing buffer, return image
    return image;
}

float4 Raytracer::work(const WorkItem* workItem) const
{
    // For each shape in the scene...
    float bestDistance = std::numeric_limits<float>::max();
    bool didHit = false;
    float4 normalColor = simd_make_float4(0, 0, 0, 1);
    
    for( const Shape& shape : _scene.shapes )
    {
        float3 hitPosition, hitNormal;
        if( shape.hitTest( workItem->ray, &hitPosition, &hitNormal ) )
        {
            float hitDistance = simd_length( hitPosition ); // Todo: - camera.position
            if( hitDistance < bestDistance )
            {
                bestDistance = hitDistance;
                didHit = true;
                normalColor = simd_make_float4( ( hitNormal + 1 ) * 0.5, 1 );
            }
        }
    }
    
    if( didHit )
    {
        return normalColor;
    }
    
    float3 dir = simd_normalize(workItem->ray.dir);
    float t = 0.5 * ( dir.y + 1.0 );
    float3 color = ( 1.0 - t ) * simd_make_float3(1, 1, 1) + t * simd_make_float3( 0.5, 0.7, 1.0 );
    return simd_make_float4( color, 1 );
}
