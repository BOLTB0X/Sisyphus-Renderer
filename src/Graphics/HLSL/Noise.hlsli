// Noise.hlsli
// https://www.shadertoy.com/view/MdGfzh
// https://github.com/NadirRoGue/RenderEngine/tree/master/RenderEngine/RenderEngine/shaders/clouds/generation
#ifndef _NOISE_HLSLI_
#define _NOISE_HLSLI_

#include "Common.hlsli"
#include "Maths.hlsli"

static const float frequenceMul[6u] = { 2.0f, 8.0f, 14.0f, 20.0f, 26.0f, 32.0f };

float hash(int n)
{
    return frac(sin(float(n) + 1.951f) * 43758.5453123f);
} // hash

float hash_12(float2 p)
{
    p = 50.0 * frac(p * 0.3183099f);
    return frac(p.x * p.y * (p.x + p.y));
} // hash12

float hash_13(float3 p3)
{
    p3 = frac(p3 * 1031.1031f);
    p3 += dot(p3, p3.yzx + 19.19f);
    return frac((p3.x + p3.y) * p3.z);
} // hash13

float2 hash_22(float2 p)
{
    float n = sin(dot(p, float2(113.0f, 1.0f)));
    p = frac(float2(2097152.0f, 262144.0f) * n) * 2.0f - 1.0f;
    return p;
} // hash22

float3 hash_33(float3 p3)
{
    p3 = frac(p3 * float3(.1031f, .1030f, .0973f));
    p3 += dot(p3, p3.yxz + 19.19f);
    return frac((p3.xxy + p3.yxx) * p3.zyx);
} // hash33

float noise(float3 x)
{
    float3 p = floor(x);
    float3 f = frac(x);

    f = f * f * (3.0f - 2.0f * f);
    float n = p.x + p.y * 57.0f + 113.0f * p.z;
    return lerp(
		lerp(
			lerp(hash(int(n + 0.0f)), hash(int(n + 1.0f)), f.x),
			lerp(hash(int(n + 57.0f)), hash(int(n + 58.0f)), f.x),
			f.y),
		lerp(
			lerp(hash(int(n + 113.0f)), hash(int(n + 114.0f)), f.x),
			lerp(hash(int(n + 170.0f)), hash(int(n + 171.0f)), f.x),
			f.y),
		f.z);
} // noise

float grad_noise2D(float2 f)
{
    float2 e = float2(0.0, 1.0);
    float2 p = floor(f);
    f -= p;
    float2 w = f * f * (3.0 - 2.0 * f); // Cubic smoothing
    
    float c = lerp(
        lerp(dot(hash_22(p + e.xx), f - e.xx), dot(hash_22(p + e.yx), f - e.yx), w.x),
        lerp(dot(hash_22(p + e.xy), f - e.xy), dot(hash_22(p + e.yy), f - e.yy), w.x),
        w.y
    );
    return c * 0.5 + 0.5; // [0, 1] 범위로 정규화
} // grad_noise2D

float voronoi2(float2 st)
{
    float2 i_st = floor(st); // 격자 아이디
    float2 f_st = frac(st); // 격자 내 좌표 (0.0 ~ 1.0)

    float m_dist = 1.0; // 최소 거리

    // 주변 9개 격자(현재 격자 + 이웃)를 탐색
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            // 이웃 격자의 위치
            float2 neighbor = float2(float(x), float(y));

            // 난수 생성 (이웃 격자 기반)
            float2 point2 = random2(i_st + neighbor);

            // 난수 기반의 포인트 위치 (0.0 ~ 1.0)
            point2 = 0.5 + 0.5 * sin(6.2831f * point2); // 움직이는 세포

            // 벡터 계산
            float2 diff = neighbor + point2
            - f_st;

            // 거리 계산 (유클리드 거리)
            float dist = length(diff);

            // 최소 거리 갱신
            m_dist = min(m_dist, dist);
        }
    }
    return m_dist;
} // voronoi2

float voronoi3(float3 p, float cellCount)
{
    float3 pCell = p * cellCount;
    float d = 1.0e10f;
    for (int xo = -1; xo <= 1; xo++)
    {
        for (int yo = -1; yo <= 1; yo++)
        {
            for (int zo = -1; zo <= 1; zo++)
            {
                float3 tp = floor(pCell) + float3(xo, yo, zo);

                tp = pCell - tp - noise(fmod(tp, cellCount / 1.0f));

                d = min(d, dot(tp, tp));
            }
        }
    }
    d = min(d, 1.0);
    d = max(d, 0.0f);

    return d;
} // voronoi3

float perlin_4D(float4 Position, float4 rep)
{
    float4 Pi0 = fmod(floor(Position), rep); // 인덱싱을 위한 정수 부분
    float4 Pi1 = fmod(Pi0 + 1.0f, rep); // 정수 부분 + 1, 래핑을 위해 mod 연산
	
    float4 Pf0 = frac(Position); // 좌표의 소수 부분
    float4 Pf1 = Pf0 - 1.0f; // 소수 부분 - 1.0
    
    float4 ix = float4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
    float4 iy = float4(Pi0.y, Pi0.y, Pi1.y, Pi1.y);
    float4 iz0 = Pi0.z;
    float4 iz1 = Pi1.z;
    float4 iw0 = Pi0.w;
    float4 iw1 = Pi1.w;

    float4 ixy = permute(permute(ix) + iy);
    float4 ixy0 = permute(ixy + iz0);
    float4 ixy1 = permute(ixy + iz1);
    float4 ixy00 = permute(ixy0 + iw0);
    float4 ixy01 = permute(ixy0 + iw1);
    float4 ixy10 = permute(ixy1 + iw0);
    float4 ixy11 = permute(ixy1 + iw1);

    float4 gx00 = ixy00 / 7.0f;
    float4 gy00 = floor(gx00) / 7.0f;
    float4 gz00 = floor(gy00) / 6.0f;
    gx00 = frac(gx00) - 0.5f;
    gy00 = frac(gy00) - 0.5f;
    gz00 = frac(gz00) - 0.5f;
    float4 gw00 = 0.75f - abs(gx00) - abs(gy00) - abs(gz00);
    float4 sw00 = step(gw00, 0.0f);
    gx00 -= sw00 * (step(0.0f, gx00) - 0.5f);
    gy00 -= sw00 * (step(0.0f, gy00) - 0.5f);

    float4 gx01 = ixy01 / 7.0f;
    float4 gy01 = floor(gx01) / 7.0f;
    float4 gz01 = floor(gy01) / 6.0f;
    gx01 = frac(gx01) - 0.5f;
    gy01 = frac(gy01) - 0.5f;
    gz01 = frac(gz01) - 0.5f;
    float4 gw01 = 0.75f - abs(gx01) - abs(gy01) - abs(gz01);
    float4 sw01 = step(gw01, 0.0f);
    gx01 -= sw01 * (step(0.0f, gx01) - 0.5f);
    gy01 -= sw01 * (step(0.0f, gy01) - 0.5f);

    float4 gx10 = ixy10 / 7.0f;
    float4 gy10 = floor(gx10) / 7.0f;
    float4 gz10 = floor(gy10) / 6.0f;
    gx10 = frac(gx10) - 0.5f;
    gy10 = frac(gy10) - 0.5f;
    gz10 = frac(gz10) - 0.5f;
    float4 gw10 = 0.75f - abs(gx10) - abs(gy10) - abs(gz10);
    float4 sw10 = step(gw10, 0.0f);
    gx10 -= sw10 * (step(0.0f, gx10) - 0.5f);
    gy10 -= sw10 * (step(0.0f, gy10) - 0.5f);

    float4 gx11 = ixy11 / 7.0f;
    float4 gy11 = floor(gx11) / 7.0f;
    float4 gz11 = floor(gy11) / 6.0f;
    gx11 = frac(gx11) - 0.5f;
    gy11 = frac(gy11) - 0.5f;
    gz11 = frac(gz11) - 0.5f;
    float4 gw11 = 0.75f - abs(gx11) - abs(gy11) - abs(gz11);
    float4 sw11 = step(gw11, 0.0f);
    gx11 -= sw11 * (step(0.0f, gx11) - 0.5f);
    gy11 -= sw11 * (step(0.0f, gy11) - 0.5f);

    float4 g0000 = float4(gx00.x, gy00.x, gz00.x, gw00.x);
    float4 g1000 = float4(gx00.y, gy00.y, gz00.y, gw00.y);
    float4 g0100 = float4(gx00.z, gy00.z, gz00.z, gw00.z);
    float4 g1100 = float4(gx00.w, gy00.w, gz00.w, gw00.w);
    float4 g0010 = float4(gx10.x, gy10.x, gz10.x, gw10.x);
    float4 g1010 = float4(gx10.y, gy10.y, gz10.y, gw10.y);
    float4 g0110 = float4(gx10.z, gy10.z, gz10.z, gw10.z);
    float4 g1110 = float4(gx10.w, gy10.w, gz10.w, gw10.w);
    float4 g0001 = float4(gx01.x, gy01.x, gz01.x, gw01.x);
    float4 g1001 = float4(gx01.y, gy01.y, gz01.y, gw01.y);
    float4 g0101 = float4(gx01.z, gy01.z, gz01.z, gw01.z);
    float4 g1101 = float4(gx01.w, gy01.w, gz01.w, gw01.w);
    float4 g0011 = float4(gx11.x, gy11.x, gz11.x, gw11.x);
    float4 g1011 = float4(gx11.y, gy11.y, gz11.y, gw11.y);
    float4 g0111 = float4(gx11.z, gy11.z, gz11.z, gw11.z);
    float4 g1111 = float4(gx11.w, gy11.w, gz11.w, gw11.w);

    float4 norm00 = taylorInv_sqrt(float4(dot(g0000, g0000), dot(g0100, g0100), dot(g1000, g1000), dot(g1100, g1100)));
    g0000 *= norm00.x;
    g0100 *= norm00.y;
    g1000 *= norm00.z;
    g1100 *= norm00.w;

    float4 norm01 = taylorInv_sqrt(float4(dot(g0001, g0001), dot(g0101, g0101), dot(g1001, g1001), dot(g1101, g1101)));
    g0001 *= norm01.x;
    g0101 *= norm01.y;
    g1001 *= norm01.z;
    g1101 *= norm01.w;

    float4 norm10 = taylorInv_sqrt(float4(dot(g0010, g0010), dot(g0110, g0110), dot(g1010, g1010), dot(g1110, g1110)));
    g0010 *= norm10.x;
    g0110 *= norm10.y;
    g1010 *= norm10.z;
    g1110 *= norm10.w;

    float4 norm11 = taylorInv_sqrt(float4(dot(g0011, g0011), dot(g0111, g0111), dot(g1011, g1011), dot(g1111, g1111)));
    g0011 *= norm11.x;
    g0111 *= norm11.y;
    g1011 *= norm11.z;
    g1111 *= norm11.w;

    float n0000 = dot(g0000, Pf0);
    float n1000 = dot(g1000, float4(Pf1.x, Pf0.y, Pf0.z, Pf0.w));
    float n0100 = dot(g0100, float4(Pf0.x, Pf1.y, Pf0.z, Pf0.w));
    float n1100 = dot(g1100, float4(Pf1.x, Pf1.y, Pf0.z, Pf0.w));
    float n0010 = dot(g0010, float4(Pf0.x, Pf0.y, Pf1.z, Pf0.w));
    float n1010 = dot(g1010, float4(Pf1.x, Pf0.y, Pf1.z, Pf0.w));
    float n0110 = dot(g0110, float4(Pf0.x, Pf1.y, Pf1.z, Pf0.w));
    float n1110 = dot(g1110, float4(Pf1.x, Pf1.y, Pf1.z, Pf0.w));
    float n0001 = dot(g0001, float4(Pf0.x, Pf0.y, Pf0.z, Pf1.w));
    float n1001 = dot(g1001, float4(Pf1.x, Pf0.y, Pf0.z, Pf1.w));
    float n0101 = dot(g0101, float4(Pf0.x, Pf1.y, Pf0.z, Pf1.w));
    float n1101 = dot(g1101, float4(Pf1.x, Pf1.y, Pf0.z, Pf1.w));
    float n0011 = dot(g0011, float4(Pf0.x, Pf0.y, Pf1.z, Pf1.w));
    float n1011 = dot(g1011, float4(Pf1.x, Pf0.y, Pf1.z, Pf1.w));
    float n0111 = dot(g0111, float4(Pf0.x, Pf1.y, Pf1.z, Pf1.w));
    float n1111 = dot(g1111, Pf1);

    float4 fade_xyzw = fade(Pf0);
    float4 n_0w = lerp(float4(n0000, n1000, n0100, n1100), float4(n0001, n1001, n0101, n1101), fade_xyzw.w);
    float4 n_1w = lerp(float4(n0010, n1010, n0110, n1110), float4(n0011, n1011, n0111, n1111), fade_xyzw.w);
    float4 n_zw = lerp(n_0w, n_1w, fade_xyzw.z);
    float2 n_yzw = lerp(float2(n_zw.x, n_zw.y), float2(n_zw.z, n_zw.w), fade_xyzw.y);
    float n_xyzw = lerp(n_yzw.x, n_yzw.y, fade_xyzw.x);
    return float(2.2) * n_xyzw;
} // Perlin4D

float worley_noise_3D(float3 p, float cellCount)
{
    return voronoi3(p, cellCount);
} // WorleyNoise3D

float perlin_noise_3D(float3 pIn, float frequency, int octafloatount)
{
    float octaveFrenquencyFactor = 2.0f; // 옥타브마다 주파수를 2배로 증가시키는 경우

    float sum = 0.0f;
    float weightSum = 0.0f;
    float weight = 0.5f;
    for (int oct = 0; oct < octafloatount; oct++)
    {
        float4 p = float4(pIn.x, pIn.y, pIn.z, 0.0) * frequency;
        float val = perlin_4D(p, frequency);

        sum += val * weight;
        weightSum += weight;

        weight *= weight;
        frequency *= octaveFrenquencyFactor;
    }

    float noise = (sum / weightSum); // *0.5 + 0.5;;
    noise = min(noise, 1.0f);
    noise = max(noise, 0.0f);
    return noise;
} // PerlinNoise3D

float grad_wave(float x, float offset)
{
    x = abs(frac(x / 6.2831853 + offset - 0.25) - 0.5) * 2.0;
    float x2 = saturate(x * x * (-1.0 + 2.0 * x));
    x = smoothstep(0.0, 1.0, x);
    return lerp(x, x2, 0.15);
} // grad_wave

#endif // _NOISE_HLSLI_