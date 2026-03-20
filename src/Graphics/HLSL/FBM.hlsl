// https://github.com/chihirobelmo/volumetric-cloud-for-directx11/tree/main
// https://www.shadertoy.com/view/ttcSD8
// https://www.shadertoy.com/view/Xttcz2
// https://www.shadertoy.com/view/3d3fWN
#include "Maths.hlsl"
#define PI     3.14159265359;
#define TWO_PI 6.28318530718;
#define UI0    1597334673U
#define UI1    3812015801U
#define UI2    uint2(UI0, UI1)
#define UI3    uint3(UI0, UI1, 2798796415U)
#define UIF    (1. / float(0xffffffffU))

int hash(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
} // hash

float hashToFloat(uint x)
{
    return (x & 0x00FFFFFF) / 16777216.0;
} // hashToFloat

uint hash3(uint3 p)
{
    return hash(p.x ^ hash(p.y ^ hash(p.z)));
} // hash3

uint hash4(uint4 p)
{
    return hash(p.x ^ hash(p.y ^ hash(p.z ^ hash(p.w))));
} // hash4

float hash01(uint x)
{
    return (x & 0x00FFFFFF) / 16777216.0;
} // hash01

float3 hash33(float3 p)
{
    uint3 q = uint3(int3(p)) * UI3;
    q = (q.x ^ q.y ^ q.z) * UI3;
    return -1. + 2. * float3(q) * UIF;
} // hash33

float3 hash33h(float3 p)
{
    p = fmod(p, 289.0);
    p = p * 0.1031;
    p = p - floor(p);
    p = p * (p + 33.33);
    p = p - floor(p);
    return -1.0 + 2.0 * p;
} // hash33h

float hash13(float3 p)
{
    uint3 q = uint3(int3(p)) * UI3;
    q *= UI3;
    uint n = (q.x ^ q.y ^ q.z) * UI0;
    return float(n) * UIF;
} // hash13

float hash12(float2 p)
{
    uint2 q = uint2(int2(p)) * UI2;
    uint n = (q.x ^ q.y) * UI0;
    return float(n) * UIF;
} // hash12

float fade(float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
} // fade

float4 fade(float4 t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
} // fade

float3 grad(uint3 p)
{
    float h = hashToFloat(hash3(p)) * 6.2831853;
    return float3(cos(h), sin(h), cos(h * 0.37));
} // grad

float4 grad4(uint4 p)
{
    float h1 = hash01(hash4(p));
    float h2 = hash01(hash4(p + 11));
    float h3 = hash01(hash4(p + 37));

    float a = h1 * TWO_PI;
    float b = h2 * TWO_PI;
    float c = h3 * TWO_PI;

    float4 g;
    g.x = cos(a);
    g.y = sin(a);
    g.z = cos(b);
    g.w = sin(b);

    return normalize(g);
} // grad4

float Perlin4D(float4 p)
{
    int4 i0 = (int4) floor(p);
    int4 i1 = i0 + 1;

    float4 f = frac(p);
    float4 u = fade(f);

    float n0000 = dot(grad4(i0), f);
    float n1000 = dot(grad4(int4(i1.x, i0.y, i0.z, i0.w)), f - float4(1, 0, 0, 0));
    float n0100 = dot(grad4(int4(i0.x, i1.y, i0.z, i0.w)), f - float4(0, 1, 0, 0));
    float n1100 = dot(grad4(int4(i1.x, i1.y, i0.z, i0.w)), f - float4(1, 1, 0, 0));

    float n0010 = dot(grad4(int4(i0.x, i0.y, i1.z, i0.w)), f - float4(0, 0, 1, 0));
    float n1010 = dot(grad4(int4(i1.x, i0.y, i1.z, i0.w)), f - float4(1, 0, 1, 0));
    float n0110 = dot(grad4(int4(i0.x, i1.y, i1.z, i0.w)), f - float4(0, 1, 1, 0));
    float n1110 = dot(grad4(int4(i1.x, i1.y, i1.z, i0.w)), f - float4(1, 1, 1, 0));

    float n0001 = dot(grad4(int4(i0.x, i0.y, i0.z, i1.w)), f - float4(0, 0, 0, 1));
    float n1001 = dot(grad4(int4(i1.x, i0.y, i0.z, i1.w)), f - float4(1, 0, 0, 1));
    float n0101 = dot(grad4(int4(i0.x, i1.y, i0.z, i1.w)), f - float4(0, 1, 0, 1));
    float n1101 = dot(grad4(int4(i1.x, i1.y, i0.z, i1.w)), f - float4(1, 1, 0, 1));

    float n0011 = dot(grad4(int4(i0.x, i0.y, i1.z, i1.w)), f - float4(0, 0, 1, 1));
    float n1011 = dot(grad4(int4(i1.x, i0.y, i1.z, i1.w)), f - float4(1, 0, 1, 1));
    float n0111 = dot(grad4(int4(i0.x, i1.y, i1.z, i1.w)), f - float4(0, 1, 1, 1));
    float n1111 = dot(grad4(i1), f - 1.0);

    float nx000 = lerp(n0000, n1000, u.x);
    float nx100 = lerp(n0100, n1100, u.x);
    float nx010 = lerp(n0010, n1010, u.x);
    float nx110 = lerp(n0110, n1110, u.x);

    float nx001 = lerp(n0001, n1001, u.x);
    float nx101 = lerp(n0101, n1101, u.x);
    float nx011 = lerp(n0011, n1011, u.x);
    float nx111 = lerp(n0111, n1111, u.x);

    float nxy00 = lerp(nx000, nx100, u.y);
    float nxy10 = lerp(nx010, nx110, u.y);
    float nxy01 = lerp(nx001, nx101, u.y);
    float nxy11 = lerp(nx011, nx111, u.y);

    float nxyz0 = lerp(nxy00, nxy10, u.z);
    float nxyz1 = lerp(nxy01, nxy11, u.z);

    return lerp(nxyz0, nxyz1, u.w) * 0.5 + 0.5;
} // Perlin4D

float PerlinPeriodic(float3 p, int frequency = 1)
{
    float3 a = p * frequency * TWO_PI;

    float4 p4;
    p4.x = cos(a.x);
    p4.y = sin(a.x);
    p4.z = cos(a.y);
    p4.w = sin(a.y);

    float n1 = Perlin4D(p4 + float4(0, 0, cos(a.z), sin(a.z)));
    float n2 = Perlin4D(p4 + float4(0, 0, cos(a.z + 1.7), sin(a.z + 1.7)));

    return 1.0 - (n1 + n2) * 0.5;
} // PerlinPeriodic

float WorleyPeriodic(float3 p, int frequency)
{
    p *= frequency;

    int3 cell = (int3) floor(p);
    float3 f = frac(p);

    float minDist = 1e6;

    [fastopt]
    for (int z = -1; z <= 1; z++)
        [fastopt]
        for (int y = -1; y <= 1; y++)
            [fastopt]
            for (int x = -1; x <= 1; x++)
            {
                int3 c = (cell + int3(x, y, z) + frequency) % frequency;
                uint3 cu = (uint3) c;

                float3 rand = float3(hashToFloat(hash3(cu + 1)), hashToFloat(hash3(cu + 2)), hashToFloat(hash3(cu + 3)));

                float3 d = float3(x, y, z) + rand - f;
                minDist = min(minDist, dot(d, d));
            }

    return 1.0 - sqrt(minDist);
} // WorleyPeriodic

float valueNoise(float3 x, float freq)
{
    float3 i = floor(x);
    float3 f = frac(x);
    f = f * f * (3. - 2. * f);
	
    return lerp(lerp(lerp(hash13(fmod(i + float3(0, 0, 0), freq)),
                          hash13(fmod(i + float3(1, 0, 0), freq)), f.x),
                     lerp(hash13(fmod(i + float3(0, 1, 0), freq)),
                          hash13(fmod(i + float3(1, 1, 0), freq)), f.x), f.y),
                lerp(lerp(hash13(fmod(i + float3(0, 0, 1), freq)),
                          hash13(fmod(i + float3(1, 0, 1), freq)), f.x),
                     lerp(hash13(fmod(i + float3(0, 1, 1), freq)),
                          hash13(fmod(i + float3(1, 1, 1), freq)), f.x), f.y), f.z);
} // valueNoise

float gradientNoise(float3 p)
{
    float3 i = floor(p);
    float3 f = frac(p);

    float3 u = f * f * (3.0 - 2.0 * f);

    return lerp(lerp(lerp(dot(hash33h(i + float3(0, 0, 0)), f - float3(0, 0, 0)),
                          dot(hash33h(i + float3(1, 0, 0)), f - float3(1, 0, 0)), u.x),
                     lerp(dot(hash33h(i + float3(0, 1, 0)), f - float3(0, 1, 0)),
                          dot(hash33h(i + float3(1, 1, 0)), f - float3(1, 1, 0)), u.x), u.y),
                lerp(lerp(dot(hash33h(i + float3(0, 0, 1)), f - float3(0, 0, 1)),
                          dot(hash33h(i + float3(1, 0, 1)), f - float3(1, 0, 1)), u.x),
                     lerp(dot(hash33h(i + float3(0, 1, 1)), f - float3(0, 1, 1)),
                          dot(hash33h(i + float3(1, 1, 1)), f - float3(1, 1, 1)), u.x), u.y), u.z);
} // gradientNoise

float snoise(float3 v)
{
    const float2 C = float2(1.0 / 6.0, 1.0 / 3.0);
    const float4 D = float4(0.0, 0.5, 1.0, 2.0);

    // First corner
    float3 i = floor(v + dot(v, C.yyy));
    float3 x0 = v - i + dot(i, C.xxx);

    // Other corners
    float3 g = step(x0.yzx, x0.xyz);
    float3 l = 1.0 - g;
    float3 i1 = min(g.xyz, l.zxy);
    float3 i2 = max(g.xyz, l.zxy);


    float3 x1 = x0 - i1 + C.xxx;
    float3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
    float3 x3 = x0 - D.yyy; // -1.0+3.0*C.x = -0.5 = -D.y

    // Permutations
    i = mod289(i);
    float4 p = permute(permute(permute(
             i.z + float4(0.0, i1.z, i2.z, 1.0))
           + i.y + float4(0.0, i1.y, i2.y, 1.0))
           + i.x + float4(0.0, i1.x, i2.x, 1.0));

    // Gradients: 7x7 points over a square, mapped onto an octahedron.
    // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
    float n_ = 0.142857142857; // 1.0/7.0
    float3 ns = n_ * D.wyz - D.xzx;

    float4 j = p - 49.0 * floor(p * ns.z * ns.z); //  mod(p,7*7)

    float4 x_ = floor(j * ns.z);
    float4 y_ = floor(j - 7.0 * x_); // mod(j,N)

    float4 x = x_ * ns.x + ns.yyyy;
    float4 y = y_ * ns.x + ns.yyyy;
    float4 h = 1.0 - abs(x) - abs(y);

    float4 b0 = float4(x.xy, y.xy);
    float4 b1 = float4(x.zw, y.zw);

    //float4 s0 = float4(lessThan(b0,0.0))*2.0 - 1.0;
    //float4 s1 = float4(lessThan(b1,0.0))*2.0 - 1.0;
    float4 s0 = floor(b0) * 2.0 + 1.0;
    float4 s1 = floor(b1) * 2.0 + 1.0;
    float4 sh = -step(h, float4(0, 0, 0, 0));

    float4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    float4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

    float3 p0 = float3(a0.xy, h.x);
    float3 p1 = float3(a0.zw, h.y);
    float3 p2 = float3(a1.xy, h.z);
    float3 p3 = float3(a1.zw, h.w);

    //Normalise gradients
    float4 norm = taylorInvSqrt(float4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    float4 m = max(0.6 - float4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    m = m * m;
    return 42.0 * dot(m * m, float4(dot(p0, x0), dot(p1, x1),
                                dot(p2, x2), dot(p3, x3)));
} // snoise

float worleyNoise(float3 uv, float freq, bool tileable)
{
    float3 id = floor(uv);
    float3 p = frac(uv);
    float minDist = 10000.;
    
    for (float x = -1.; x <= 1.; ++x)
    {
        for (float y = -1.; y <= 1.; ++y)
        {
            for (float z = -1.; z <= 1.; ++z)
            {
                float3 offset = float3(x, y, z);
                float3 h = float3(0., 0., 0.);
                if (tileable)
                    h = hash33(fmod(id + offset, float3(freq, freq, freq))) * .4 + .3; // [.3, .7]
                else
                    h = hash33(id + offset) * .4 + .3; // [.3, .7]
                h += offset;
                float3 d = p - h;
                minDist = min(minDist, dot(d, d));
            }
        }
    }
    
    return 1. - minDist;
} // worleyNoise

float worley(float3 p, float scale)
{
    float3 id = floor(p * scale);
    float3 fd = frac(p * scale);

    float n = 0.;

    float minimalDist = 1.;


    for (float x = -1.; x <= 1.; x++)
    {
        for (float y = -1.; y <= 1.; y++)
        {
            for (float z = -1.; z <= 1.; z++)
            {

                float3 coord = float3(x, y, z);
                float3 rId = hash33(fmod(id + coord, scale)) * 0.5 + 0.5;

                float3 r = coord + rId - fd;

                float d = dot(r, r);

                if (d < minimalDist)
                {
                    minimalDist = d;
                }

            } //z
        } //y
    } //x
    
    return 1.0 - minimalDist;
} // worley

float perlinFbm(float3 p, float freq, int octaves)
{
    float G = .5;
    float amp = 1.;
    float noise = 0.;
    for (int i = 0; i < octaves; ++i)
    {
        noise += amp * valueNoise(p * freq, freq);
        freq *= 2.;
        amp *= G;
    }
    
    return noise;
} // perlinFbm

// Valentin Galea 스타일의 Cloud FBM
// lacunarity: 주파수 배율 (보통 2.0)
// gain: 진폭 감소율 (보통 0.5)
float fbm_clouds(float3 p, float lacunarity, float init_gain, float gain)
{
    float3 pos = p;
    float amplitude = init_gain;
    float totalNoise = 0.0;

    for (int i = 0; i < 5; i++)
    {
        totalNoise += abs(snoise(pos)) * amplitude;
        
        pos *= lacunarity;
        amplitude *= gain;
    }
    return totalNoise;
} // fbm_clouds