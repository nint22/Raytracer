//
//  RaytracerView.m
//  Raytracer
//
//  Created by Jeremy Bridon on 4/25/20.
//  Copyright © 2020 Jeremy Bridon. All rights reserved.
//

#import "RaytracerView.h"

@interface RaytracerView ()
{
    CGImageRef _image;
}
@end

@implementation RaytracerView

- (void) updateImage: (CGImageRef) image
{
    // Release old, retain new
    CGImageRelease( _image );
    _image = image;
    CGImageRetain( _image );
    
    // Redraw
    [self setNeedsDisplay: true];
}

- (void) drawRect: (NSRect) dirtyRect
{
    // If no image, ignore..
    if( _image == NULL )
        return;
    
    // Scale given image to fit aspect ratio
    const size_t imageWidth = CGImageGetWidth( _image );
    const CGFloat viewWidth = [self bounds].size.width;
    CGFloat widthRatio = viewWidth / imageWidth;
    
    const size_t imageHeight = CGImageGetHeight( _image );
    const CGFloat viewHeight = [self bounds].size.height;
    CGFloat heightRatio = viewHeight / imageHeight;
    
    CGFloat ratio = fmin( widthRatio, heightRatio );
    
    // Center
    CGContextRef context = [[NSGraphicsContext currentContext] CGContext];
    const CGFloat dx = viewWidth / 2 - ( imageWidth * ratio ) / 2;
    const CGFloat dy = viewHeight / 2 - ( imageHeight * ratio ) / 2;
    CGContextTranslateCTM( context, dx, dy );
    
    // Scale
    CGContextScaleCTM( context, ratio, ratio );
    
    // Draw image in question
    CGContextDrawImage( context, CGRectMake( 0, 0, imageWidth, imageHeight ), _image );
}

@end
