//
//  ViewController.h
//  Raytracer
//
//  Created by Jeremy Bridon on 4/25/20.
//  Copyright © 2020 Jeremy Bridon. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "RaytracerView.h"

@interface ViewController : NSViewController

@property (nonatomic, weak) IBOutlet RaytracerView* raytracerView;

@end

