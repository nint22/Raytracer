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
#include <stdlib.h>
#include <math.h>

using int2 = simd_int2;

using float2 = simd_float2;
using float3 = simd_float3;
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

inline float random_float()
{
    return (float)rand() / RAND_MAX;
}

inline float random_float(float min, float max)
{
    return min + random_float() * ( max - min );
}

inline float3 random_float3()
{
    return simd_make_float3( random_float(), random_float(), random_float() );
}

inline float3 random_float3(float min, float max)
{
    return simd_make_float3( random_float( min, max ), random_float( min, max ), random_float( min, max ) );
}

inline float3 random_sphere_float3()
{
    while( true )
    {
        float3 p = random_float3( -1, 1 );
        if( simd_length( p ) < 1 )
            return p;
    }
}

inline float3 random_unit_float3()
{
    float a = random_float( 0, 2.0 * M_PI );
    float z = random_float( -1, 1 );
    float r = sqrt( 1.0 - z * z );
    return simd_make_float3( r * cos( a ), r * sin( a ), z );
}

inline float3 reflect(float3 v, float3 n)
{
    return v - 2.0 * simd_dot( v, n ) * n;
}

inline float3 refract(float3 uv, float3 n, float etaiOverEtat)
{
    float cosTheta = simd_dot( -uv, n );
    float3 outParallel = etaiOverEtat * ( uv + cosTheta * n );
    float3 outPerp = -sqrt( 1.0 - simd_length_squared( outParallel ) ) * n;
    return outParallel + outPerp;
}

inline float schlick(float cosine, float idx)
{
    float r0 = ( 1.0 - idx ) / ( 1 + idx );
    r0 *= r0;
    return r0 + ( 1.0 - r0 ) * pow( 1.0 - cosine, 5.0 );
}

#endif /* VectorTypes_h */
