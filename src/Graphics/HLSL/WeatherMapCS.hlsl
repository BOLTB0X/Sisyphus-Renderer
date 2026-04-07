// WeatherMapCS.hlsl
// 참고 : https://github.com/chihirobelmo/volumetric-cloud-for-directx11/blob/main/VolumetricCloud/shaders/CloudMapGenerate.hlsl
#include "FBM.hlsli"
#include "Remap.hlsli"

RWTexture2D<float4> outWeatherMap : register(u0);

cbuffer WeatherMapBuffer : register(b3)
{
    float2 cResolution;
    float  cTime;
    float  cCoverage;
}; // WeatherMapBuffer

static const float TIME_FREQ_MSEC = 60.0 * 60.0 * 1000.0 * 1000.0;

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= (uint) cResolution.x || DTid.y >= (uint) cResolution.y)
    {
        return;
    }
    
    float2 uv = (float2(DTid.xy) + 0.5f) / cResolution;
    float3 uvw = float3(uv * 2.0f - 1.0f, 0.0f);

    float pw = perlin_worley(uvw, 8, 4) * 0.25;
    float w = worley_fbm(uvw, 8, true);
    
    float r = remap_clamp(pw * 0.5f + 0.5f, 1.0f - cCoverage, 1.0f, 0.0f, 1.0f);
    float g = remap_clamp(w * 0.5f + 0.5f, 1.0f - cCoverage, 1.0f, 0.0f, 1.0f);
    outWeatherMap[DTid.xy] = float4(r, g, 0.0f, 1.0f);
} // main