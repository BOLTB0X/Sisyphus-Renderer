// PBR.hlsli
#ifndef _PBR_HLSLI_
#define _PBR_HLSLI_
#include "Common.hlsli"

float distribution_GGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = saturate(dot(N, H));
    float denom = (NdotH * NdotH * (a2 - 1.0f) + 1.0f);
    
    return a2 / (PI * denom * denom);
} // distribution_GGX

float geometry_schlick_GGX(float NdotV, float roughness)
{
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    
    return NdotV / (NdotV * (1.0f - k) + k);
} // geometry_schlick_GGX

float geometry_smith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));
    return geometry_schlick_GGX(NdotV, roughness)
         * geometry_schlick_GGX(NdotL, roughness);
} // geometry_smith

float3 fresnel_schlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(saturate(1.0f - cosTheta), 5.0f);
} // fresnel_schlick

#endif