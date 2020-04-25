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

// Shape is a union - could sub-class, but keeping it simple for now
class Shape
{
public:
    
};

// Scene has a collection of shapes
class Scene
{
public:
    
};

// Camera describes location, fov, target resolution, etc.
class Camera
{
public:
    
    Camera() = default;
    Camera(int2 resolution);
    
    int2 resolution() const;
    
private:
    
    int2 _resolution = simd_make_int2(100, 100);
};

// Raytracer is the main rendering service. Takes a scene, camera, and renders it out
class Raytracer
{
public:
    
    Raytracer(const Camera& camera);
    ~Raytracer();
    
    // Start rendering: this is a background operation, non-blocking
    void renderAsync();
    bool isComplete();
    
    // Query current render buffers. This locks the async rendering work,
    // so it is expensive.
    CGImageRef copyRenderImage();
    
private:
    
    // Camera in scene to render from
    Camera _camera;
    
    // Backing image buffer
    os_unfair_lock _backingBufferLock;
    float4* _backingBuffer;
    
    // Background queue we're doing the dispatch_apply from
    dispatch_queue_t _workQueue;
    
    // Work item
    struct WorkItem
    {
        int2 pixelPos;
        Ray ray;
    };
    
    // Do work given a work item. Returns color contribution
    float4 work(const WorkItem* workItem) const;
    
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
