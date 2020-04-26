//
//  Raytracer.hpp
//  Raytracer
//
//  Created by Jeremy Bridon on 4/25/20.
//  Copyright © 2020 Jeremy Bridon. All rights reserved.
//

#ifndef Raytracer_h
#define Raytracer_h

#include <CoreGraphics/CoreGraphics.h>
#include <dispatch/dispatch.h>
#include <os/lock.h>
#include <vector>

#include "VectorTypes.h"

// Ray has origin and direction
struct Ray
{
    float3 pos;
    float3 dir;
    
    float3 at(float t) const;
};

// Hit has hit position and normal
struct Hit
{
    float3 pos;
    float3 norm;
};

// Interface to do collision testing: any shape class should conform tothis
class IHittable
{
public:
    
    virtual bool hitTest(const Ray& ray, float tmin, float tmax, Hit* hit = nullptr) const = 0;
    
};

// Sphere, conforming to hittable
class Sphere : public IHittable
{
public:
    
    Sphere(float radius);
    
    // Common properties
    float3 position() const;
    void setPosition(const float3& p);
    
    // Sphere properties:
    float radius() const;
    
    // Returns true if a hit was found, and returns that
    // position and normal via optional in/out via argument
    bool hitTest(const Ray& ray, float tmin, float tmax, Hit* hit = nullptr) const override;
    
private:
    
    float3 _position = simd_make_float3( 0, 0, 0 );
    float _radius;
    
};

// Scene has a collection of hittable objects
class Scene
{
public:
    
    // Public for ease
    std::vector< IHittable* > shapes;
    
    // Given a ray, return closest hit test (if any)
    bool hitTest(const Ray& ray, float tmin, float tmax, Hit* hit = nullptr) const;
    
};

// Camera describes location, fov, target resolution, etc.
class Camera
{
public:
    
    Camera() = default;
    Camera(int2 resolution);
    
    int2 resolution() const;
    
    int sampleCount() const;
    void setSampleCount(int sampleCount);
    
    int maxBounceCount() const; // Times a ray can bounce around
    void setMaxBounceCount(int bounceCount);
    
    float3 position() const;
    void setPosition(float3 position);
    
    // Given a UV coordinate, return vector. Is randomized for AA if sample count > 1
    Ray getRay(float2 uv) const;
    
private:
    
    int2 _resolution = simd_make_int2(100, 100);
    
    int _sampleCount = 1;
    int _maxBounceCount = 1;
    
    float3 _position = simd_make_float3(0, 0, 0);
    
    // Camera plane properties
    float3 lowerLeftCornerPosition;
    float3 horizontalVector;
    float3 verticalVector;
};

// Raytracer is the main rendering service. Takes a scene, camera, and renders it out
class Raytracer
{
public:
    
    Raytracer(const Camera& camera, const Scene& scene);
    ~Raytracer();
    
    // Start rendering: this is a background operation, non-blocking
    void renderAsync();
    bool isComplete();
    
    // Query current render buffers. This locks the async rendering work,
    // so it is expensive.
    CGImageRef copyRenderImage();
    
private:
    
    // Camera and scene to render
    Camera _camera;
    Scene _scene;
    
    // Backing image buffer
    os_unfair_lock _backingBufferLock;
    float4* _backingBuffer;
    
    // Background queue we're doing the dispatch_apply from
    dispatch_queue_t _workQueue;
    
    // Work item
    struct WorkItem
    {
        int2 pixelPos;
    };
    
    // Ray testing the scene..
    float4 rayTest(const Ray& ray, int depth = 0) const;
    
    // Work items and lock
    os_unfair_lock _workLock;
    std::vector< WorkItem > _workItems;
    
    // Current state
    enum State {
        Setup,      // Initialized, doing no work
        Active,     // Active work
        Complete,   // All done!
    } _state;
    
    // Final image we've rendered
    CGImageRef _finalImage;
    
};

#endif /* Raytracer_h */
