//
//  RaytracerView.h
//  Raytracer
//
//  Created by Jeremy Bridon on 4/25/20.
//  Copyright © 2020 Jeremy Bridon. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface RaytracerView : NSView

// Assign an image that came from the latest update from the renderer
- (void) updateImage: (CGImageRef) image;

@end

NS_ASSUME_NONNULL_END
