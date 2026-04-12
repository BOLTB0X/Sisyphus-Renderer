// WeatherMapCS.hlsl
// https://github.com/NadirRoGue/RenderEngine/blob/master/RenderEngine/RenderEngine/shaders/clouds/generation/weather.comp

cbuffer WeatherMapBuffer : register(b3)
{
    float2 wResolution;
    float2 wPadding1;
    float3 wSeed;
    float  wPadding2;
}; // WeatherMapBuffer

RWTexture2D<float4> outWeatherMap : register(u0);

static const float perlinScale = 100.0f;
static const float perlinFrequency = 0.8f;
static const float perlinAmplitude = 0.5f;

float GetRandom2D(float2 st)
{
    return frac(sin(dot(st.xy, float2(12.9898, 78.233) + wSeed.xy)) * 43758.5453123);
} // GetRandom2D

float NoiseInterpolation(/* 텍스처 좌표*/ float2 i_coord, 
                        /* 노이즈의 스케일*/ float i_size)
{
    float2 grid = i_coord * i_size;
    
    float2 randomInput = floor(grid);
    float2 weights = frac(grid);
    
    float p0 = GetRandom2D(randomInput);
    float p1 = GetRandom2D(randomInput + float2(1.0f, 0.0f));
    float p2 = GetRandom2D(randomInput + float2(0.0f, 1.0f));
    float p3 = GetRandom2D(randomInput + float2(1.0f, 1.0f));
    
    weights = smoothstep(0.0f, 1.0f, weights);
    
    // Bilinear Interpolation
    return lerp(lerp(p0, p1, weights.x),
                lerp(p2, p3, weights.x),
                weights.y);
} // NoiseInterpolation

float PerlinNoise(/* 텍스처 좌표*/ float2 uv, 
                /* 노이즈의 스케일*/ float sc,
                /* 주파수*/ float f,
                /* 진폭*/ float a,
                /* 옥타브 수*/ int o)
{
    float noiseValue = 0.0;
    float localAmplitude = a;
    float localFrequency = f;

    for (int index = 0; index < o; index++)
    {
        noiseValue += NoiseInterpolation(uv, sc * localFrequency) * localAmplitude;
        localAmplitude *= 0.25;
        localFrequency *= 3.0;
    }

    return noiseValue * noiseValue;
} // PerlinNoise

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int2 pixel = DTid.xy;

    // 해상도 체크
    if (pixel.x >= (int) wResolution.x || pixel.y >= (int) wResolution.y)
        return;

    float2 uv = float2((float) pixel.x + 2.0f / wResolution.x, (float) pixel.y + 2.0f / wResolution.y);
    float2 suv = float2(uv.x + 5.5f, uv.y + 5.5f);

    // R: Coverage (구름의 밀도 분포)
    // G: Cloud Type (구름의 고도/종류 결정 인자)
    float cloudType = saturate(PerlinNoise(suv, perlinScale * 3.0f, 0.3f, 0.7f, 1.0f));
    float coverage = PerlinNoise(uv, perlinScale * 0.95f, perlinFrequency, perlinAmplitude, 4);

    float4 weather = float4(saturate(coverage), cloudType, 0.0f, 1.0f);

    outWeatherMap[pixel] = weather;
} // main