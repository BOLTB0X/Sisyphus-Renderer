// Ground.hlsli
// https://www.shadertoy.com/view/ld3BzM
#ifndef _GROUND_HLSLI_
#define _GROUND_HLSLI_

#include "Maths.hlsli"
#include "FBM.hlsli"

float sand_layer(float2 p)
{
    float2 q = mul(rot2(3.14159 / 18.0), p);
    q.y += (grad_noise2D(q * 18.0) - 0.5) * 0.05;
    float grad1 = grad_wave(q.y * 80.0, 0.0);
    
    q = mul(rot2(-3.14159 / 20.0), p);
    q.y += (grad_noise2D(q * 12.0) - 0.5) * 0.05;
    float grad2 = grad_wave(q.y * 80.0, 0.5);
    
    q = mul(rot2(3.14159 / 4.0), p);
    float a2 = dot(sin(q * 12.0 - cos(q.yx * 12.0)), float2(0.25, 0.25)) + 0.5;
    float a1 = 1.0 - a2;
    
    // Screen blend
    return 1.0 - (1.0 - grad1 * a1) * (1.0 - grad2 * a2);
} // sand_layer

float get_sand_texture(float2 p, float dist)
{
    p = float2(p.y - p.x, p.x + p.y) * 0.7071 / 4.0;
    
    float c1 = sand_layer(p);
    float2 q = mul(rot2(3.14159 / 12.0), p);
    float c2 = sand_layer(q * 1.25);
    
    c1 = lerp(c1, c2, smoothstep(0.1, 0.9, grad_noise2D(p * 4.0)));
    
    return c1 / (1.0 + dist * dist * 0.015);
} // get_sand_texture

#endif