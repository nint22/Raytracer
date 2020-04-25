//
//  VectorTypes.h
//  Raytracer
//
//  Created by Jeremy Bridon on 4/25/20.
//  Copyright © 2020 Jeremy Bridon. All rights reserved.
//

#ifndef VectorTypes_h
#define VectorTypes_h

#include <simd/simd.h>

using int2 = simd_int2;

using float2 = simd_float4;
using float3 = simd_float4;
using float4 = simd_float4;

inline uint8_t clamp( float value, uint8_t min, uint8_t max )
{
    if( value < min )
        return min;
    else if( value > max )
        return max;
    else
        return value;
}

#endif /* VectorTypes_h */
