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
    CGFloat widthRatio = [self bounds].size.width / imageWidth;
    
    const size_t imageHeight = CGImageGetHeight( _image );
    CGFloat heightRatio = [self bounds].size.height / imageHeight;
    
    CGFloat ratio = fmin( widthRatio, heightRatio );
    
    // TODO: Center image, scale
    CGContextRef context = [[NSGraphicsContext currentContext] CGContext];
    
    // Draw image in question
    CGContextDrawImage( context, CGRectMake( 0, 0, imageWidth, imageHeight ), _image );
}

@end
