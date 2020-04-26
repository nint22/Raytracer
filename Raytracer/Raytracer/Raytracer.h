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

// Forward declare material: a hit will always have a reference
// to an object's material.
class IMaterial;

// Hit has hit position and normal
struct Hit
{
    float3 pos;
    float3 norm;
    IMaterial* material = nullptr;
    bool isFrontFace;
};

// Interface to do collision testing: any shape class should conform tothis
class IHittable
{
public:
    
    virtual bool hitTest(const Ray& ray, float tmin, float tmax, Hit* hit = nullptr) const = 0;
    
};

// Materials define how rays scatter: diffuse materials randomze rays a ton,
// but metallic are highly reflective and do near perfect reflections, etc.
class IMaterial
{
public:
    
    virtual ~IMaterial() = default;
    
    virtual bool scatter(const Ray& ray, const Hit& hit, float3* attenuation, Ray* scattered) const = 0;
    
};

// Concrete Lambertian material
class LambertianMaterial : public IMaterial
{
public:
    
    LambertianMaterial(const float3& albedo);
    
    bool scatter(const Ray& ray, const Hit& hit, float3* attenuation, Ray* scattered) const override;
    
private:
    
    float3 _albedo;
    
};

// Concrete metal material
class MetalMaterial : public IMaterial
{
public:

    // 0 roughness = super shiney, 1 roughness = blyrr
    MetalMaterial(const float3& albedo, float roughness);
    
    bool scatter(const Ray& ray, const Hit& hit, float3* attenuation, Ray* scattered) const override;
    
private:
    
    float3 _albedo;
    float _roughness;
};

// Concrete glass material
class DielectricMaterial : public IMaterial
{
public:

    DielectricMaterial(float ri);
    
    bool scatter(const Ray& ray, const Hit& hit, float3* attenuation, Ray* scattered) const override;
    
private:
    
    float _ri;
};


// Sphere, conforming to hittable
class Sphere : public IHittable
{
public:
    
    Sphere(float radius);
    ~Sphere();
    
    // Common properties
    float3 position() const;
    void setPosition(const float3& p);
    
    // Sphere properties:
    float radius() const;
    void setRadius(float radius);
    
    // Material
    IMaterial* material() const;
    void setMaterial(IMaterial* material);
    
    // Returns true if a hit was found, and returns that
    // position and normal via optional in/out via argument
    bool hitTest(const Ray& ray, float tmin, float tmax, Hit* hit = nullptr) const override;
    
private:
    
    float3 _position = simd_make_float3( 0, 0, 0 );
    float _radius = 1.0;
    IMaterial* _material = nullptr; // Set in constructor to a default..
    
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
    Camera(int2 resolution, float fovy);
    
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
    
    float _fovy; // Vertical degrees
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
    float3* _backingBuffer;
    
    // Background queue we're doing the dispatch_apply from
    dispatch_queue_t _workQueue;
    
    // Work item
    struct WorkItem
    {
        int2 pixelPos;
    };
    
    // Ray testing the scene..
    float3 rayTest(const Ray& ray, int depth = 0) const;
    
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
