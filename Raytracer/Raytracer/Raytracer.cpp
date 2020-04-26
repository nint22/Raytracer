//
//  Raytracer.cpp
//  Raytracer
//
//  Created by Jeremy Bridon on 4/25/20.
//  Copyright © 2020 Jeremy Bridon. All rights reserved.
//

#include "Raytracer.h"

#include <limits>
#include <random>
#include <algorithm>
#include <iterator>

#pragma mark Ray Struct

float3 Ray::at(float t) const
{
    return pos + simd_normalize(dir) * t;
}

#pragma mark Material Classes

LambertianMaterial::LambertianMaterial(const float3& albedo)
{
    _albedo = albedo;
}

bool LambertianMaterial::scatter(const Ray& ray, const Hit& hit, float3* attenuation, Ray* scattered) const
{
    scattered->pos = hit.pos;
    scattered->dir = hit.norm + random_unit_float3();
    *attenuation = _albedo;
    return true;
}

MetalMaterial::MetalMaterial(const float3& albedo, float roughness)
{
    _albedo = albedo;
    _roughness = clamp( roughness, 0, 1 );
}

bool MetalMaterial::scatter(const Ray& ray, const Hit& hit, float3* attenuation, Ray* scattered) const
{
    float3 reflected = reflect( simd_normalize( ray.dir), hit.norm );
    scattered->pos = hit.pos;
    scattered->dir = reflected + _roughness * random_unit_float3();
    *attenuation = _albedo;
    return ( simd_dot( scattered->dir, hit.norm ) > 0 );
}

DielectricMaterial::DielectricMaterial(float ri)
{
    _ri = ri;
}

bool DielectricMaterial::scatter(const Ray& ray, const Hit& hit, float3* attenuation, Ray* scattered) const
{
    *attenuation = simd_make_float3( 1, 1, 1 );
    
    float etaiOverEtat = 1.0 / _ri;
    if( hit.isFrontFace == false )
        etaiOverEtat = _ri;
    
    float3 unitDirection = simd_normalize( ray.dir );
    
    float cosTheta = fmin( simd_dot( -unitDirection, hit.norm ), 1.0 );
    float sinTheta = sqrt( 1.0 - cosTheta * cosTheta );
    
    float reflectionPorbability = schlick( cosTheta, etaiOverEtat );
    
    if( etaiOverEtat * sinTheta > 1.0 )
    {
        float3 reflected = reflect(unitDirection, hit.norm);
        scattered->pos = hit.pos;
        scattered->dir = reflected;
    }
    else if( random_float() < reflectionPorbability )
    {
        float3 reflected = reflect(unitDirection, hit.norm);
        scattered->pos = hit.pos;
        scattered->dir = reflected;
    }
    else
    {
        float3 refracted = refract(unitDirection, hit.norm, etaiOverEtat);
        scattered->pos = hit.pos;
        scattered->dir = refracted;
    }
    
    return true;
}

#pragma mark Sphere Class

Sphere::Sphere(float radius)
{
    _radius = radius;
    
    // Default material:
    _material = new LambertianMaterial(simd_make_float3(0.5, 0.5, 0.5));
}

Sphere::~Sphere()
{
    delete _material;
}

float3 Sphere::position() const
{
    return _position;
}

void Sphere::setPosition(const float3& p)
{
    _position = p;
}

float Sphere::radius() const
{
    return _radius;
}

void Sphere::setRadius(float radius)
{
    _radius = radius;
}

IMaterial* Sphere::material() const
{
    return _material;
}

void Sphere::setMaterial(IMaterial* material)
{
    // Delete old, take new
    delete _material;
    _material = material;
}

bool Sphere::hitTest(const Ray& ray, float tmin, float tmax, Hit* hit) const
{
    float3 oc = ray.pos - _position;
    float3 dir = simd_normalize( ray.dir );
    float a = simd_length_squared( dir );
    float half_b = simd_dot( oc, dir );
    float c = simd_length_squared( oc ) - _radius * _radius;
    float discrim = half_b * half_b - a * c;
    if( discrim < 0.0 )
        return false;
    
    // First root...
    float t = ( -half_b - sqrt( discrim ) ) / a;
    if( t >= tmin && t <= tmax )
    {
        float3 position = ray.at( t );
        float3 normal = simd_normalize( position - _position );
        
        if( hit != nullptr )
        {
            hit->pos = position;
            hit->material = _material;
            hit->isFrontFace = ( simd_dot( ray.dir, normal ) < 0.0 );
            hit->norm = hit->isFrontFace ? normal : -normal;
        }
        
        return true;
    }
    
    // Second root...
    t = ( -half_b + sqrt( discrim ) ) / a;
    if( t >= tmin && t <= tmax )
    {
        float3 position = ray.at( t );
        float3 normal = simd_normalize( position - _position );
        
        if( hit != nullptr )
        {
            hit->pos = position;
            hit->material = _material;
            hit->isFrontFace = ( simd_dot( ray.dir, normal ) < 0.0 );
            hit->norm = hit->isFrontFace ? normal : -normal;
        }
        
        return true;
    }
    
    // No roots
    return false;
}

#pragma mark Scene Class

bool Scene::hitTest(const Ray& ray, float tmin, float tmax, Hit* hit) const
{
    // For each shape in the scene...
    float bestDistance = std::numeric_limits<float>::max();
    bool didHit = false;
    for( const IHittable* shape : shapes )
    {
        Hit candidate;
        if( shape->hitTest( ray, tmin, tmax, &candidate ) )
        {
            float hitDistance = simd_length( candidate.pos - ray.pos );
            if( hitDistance < bestDistance )
            {
                bestDistance = hitDistance;
                didHit = true;
                if( hit != nullptr )
                    *hit = candidate;
            }
        }
    }
    
    return didHit;
}

#pragma mark Camera Class

Camera::Camera(int2 resolution, float3 position, float3 target, float3 up, float fovy,
               float aperature, float focusDistance)
{
    _fovy = fovy;
    _resolution = resolution;
    _lensRadius = aperature / 2.0;
    
    // Degrees to radians
    fovy *= ( M_PI / 180.0 );
    float halfHeight = tan( fovy / 2.0 );
    float halfWidth = ( (float)_resolution.x / _resolution.y ) * halfHeight;
    
    _position = position;
    w = simd_normalize( position - target );
    u = simd_normalize( simd_cross( up, w ) );
    v = simd_cross( w, u );
    
    lowerLeftCornerPosition = position - halfWidth * focusDistance * u - halfHeight * focusDistance * v - focusDistance * w;
    horizontalVector = 2.0 * halfWidth * focusDistance * u;
    verticalVector = 2.0 * halfHeight * focusDistance * v;
}

int2 Camera::resolution() const
{
    return _resolution;
}

int Camera::sampleCount() const
{
    return _sampleCount;
}

void Camera::setSampleCount(int sampleCount)
{
    _sampleCount = sampleCount;
}

int Camera::maxBounceCount() const
{
    return _maxBounceCount;
}

void Camera::setMaxBounceCount(int bounceCount)
{
    _maxBounceCount = bounceCount;
}

float3 Camera::position() const
{
    return _position;
}

void Camera::setPosition(float3 position)
{
    _position = position;
}

Ray Camera::getRay(float2 uv) const
{
    float3 rd = _lensRadius * random_unit_disk();
    float3 offset = u * rd.x + v * rd.y;
    
    Ray ray;
    ray.pos = _position + offset;
    ray.dir = lowerLeftCornerPosition + uv.x * horizontalVector + uv.y * verticalVector - _position - offset;
    return ray;
}

#pragma mark Raytracer Class

Raytracer::Raytracer(const Camera& camera, const Scene& scene)
{
    _camera = camera;
    _scene = scene;
    
    _backingBuffer = new float3[ camera.resolution().x * camera.resolution().y ];
    _backingBufferLock = OS_UNFAIR_LOCK_INIT;
    
    _workQueue = dispatch_queue_create( "Raytracer queue", dispatch_queue_attr_make_with_qos_class( 0, QOS_CLASS_DEFAULT, 0 ) );
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
    
    // Async all the work
    dispatch_async(_workQueue, ^{

        // Clear our backing buffer
        const size_t backingBufferLength = sizeof( float3 ) * _camera.resolution().x * _camera.resolution().y;
        memset( _backingBuffer, 0, backingBufferLength );
        
        // Create all the work we want to complete
        printf( "Setting up render work...\n" );
        for( int y = 0; y < _camera.resolution().y; y++ )
        {
            for( int x = 0; x < _camera.resolution().x; x++ )
            {
                // Declare work on the pixel (and save UV...)
                WorkItem workItem;
                workItem.pixelPos = simd_make_int2(x, y);
                
                // Push work
                _workItems.push_back(workItem);
            }
        }
        
        // Shuffle so we see random points come online across the image..
        // This helps us preview what's going on faster
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(_workItems.begin(), _workItems.end(), g);
        
        // Do the rendering work:
        printf( "Starting render work...\n" );
        dispatch_apply(_workItems.size(), NULL, ^(size_t) {
            
            // Get work
            os_unfair_lock_lock(&_workLock);
            const WorkItem workItem = _workItems.back();
            _workItems.pop_back();
            os_unfair_lock_unlock(&_workLock);
            
            // Do work
            float3 color = simd_make_float3( 0, 0, 0 );

            // Helpful constant
            const float2 f2Resolution = simd_make_float2( _camera.resolution().x, _camera.resolution().y );
            
            // For sample count..
            for( int sampleIndex = 0; sampleIndex < _camera.sampleCount(); sampleIndex++ )
            {
                // Compute UV with possible offset
                float2 uv = simd_make_float2( workItem.pixelPos.x, workItem.pixelPos.y );
                uv.x += ( sampleIndex == 0 ) ? 0 : random_float();
                uv.y += ( sampleIndex == 0 ) ? 0 : random_float();
                
                // Normalize
                uv /= f2Resolution;
                
                // Generate ray through camera with this
                Ray ray = _camera.getRay( uv );
                
                // Do work!
                color += rayTest(ray);
            }
            
            // Normalize color to both the sample count *and* be gamma corrected
            color.x = sqrt( color.x / _camera.sampleCount() );
            color.y = sqrt( color.y / _camera.sampleCount() );
            color.z = sqrt( color.z / _camera.sampleCount() );
            
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
            printf( "Complete!\n" );
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
            
            float3 sourceColor = _backingBuffer[ pixelIndex ];
            
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

float3 Raytracer::rayTest(const Ray& ray, int depth) const
{
    // Ignore if reached max depth
    if( depth >= _camera.maxBounceCount() )
        return simd_make_float3( 0, 0, 0 );
    
    Hit candidate;
    if( _scene.hitTest( ray, 0.001, std::numeric_limits<float>::max(), &candidate ) )
    {
        Ray scatteredRay;
        float3 attenuation;
        if( candidate.material->scatter( ray, candidate, &attenuation, &scatteredRay ) )
        {
            return attenuation * rayTest( scatteredRay, depth + 1 );
        }
    }
    
    float3 dir = simd_normalize(ray.dir);
    float t = 0.5 * ( dir.y + 1.0 );
    return ( 1.0 - t ) * simd_make_float3( 1, 1, 1 ) + t * simd_make_float3( 0.5, 0.7, 1.0 );
}
