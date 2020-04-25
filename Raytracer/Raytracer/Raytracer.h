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

#include "VectorTypes.h"

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
    
};

#endif /* Raytracer_h */
