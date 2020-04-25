
# Raytracer

A weekend raytracing project for macOS. Based on [Peter Shirley's](https://raytracing.github.io/books/RayTracingInOneWeekend.html) article.

## Engineering Notes

First, I've created a UI that renders a CGImage that is updated over time. I want this infastructure
to have some sort of live preview of content. Avoiding tasks that aren't directly related to the core
of the project, but this one I think is important for me.

I want to write the raytracer mostly in C++, as an exercise to myself and to avoid the perf costs of Objective-C.
I want to also stick with Apple's tech: simd vector types, dispatch_apply, etc.

## Tasks

- Fix aspect ratio of camera with given texture size..

## Complete

- Add a sphere shape type
- Add simple collision testing to sphere
- Add state machine for checking before / in-flight / complete drawing, avoid double-calling setup, etc.
- Fill with gradient as function of ray with camera setup
- Setup async per-pixel drawing logic
-- Complete via "WorkItem" pattern
- Setup the class structures in C++: raytracer, scene, camera, shape
- Create a simple UI that renders a CGImage from a bitmap
