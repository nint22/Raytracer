
# Raytracer

A weekend raytracing project for macOS. Based on [Peter Shirley's](https://raytracing.github.io/books/RayTracingInOneWeekend.html) article.

## Engineering Notes

First, I've created a UI that renders a CGImage that is updated over time. I want this infastructure
to have some sort of live preview of content. Avoiding tasks that aren't directly related to the core
of the project, but this one I think is important for me.

I want to write the raytracer mostly in C++, as an exercise to myself and to avoid the perf costs of Objective-C.
I want to also stick with Apple's tech: simd vector types, dispatch_apply, etc.

## Tasks

- Create a simple UI that renders a CGImage from a bitmap

## Complete
