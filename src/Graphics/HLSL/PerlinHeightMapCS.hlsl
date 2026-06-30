// PerlinHeightMapCS.hlsl
RWTexture2D<float> OutHeightMap : register(u0);

static const float2 NOISE_OFFSET = float2(0.0f, 0.0f); // 지형 시작점
static const float NOISE_SCALE = 3.5f; // 노이즈 밀도 (작을수록 산맥이 넓어짐)
static const int OCTAVES = 8; // 디테일 반복 횟수
static const float PERSISTENCE = 0.5f; // 진폭 감소율
static const float LACUNARITY = 2.0f; // 주파수 증가율
static const float2 NOISE_SEED = float2(12.9898f, 78.233f);
static const float NOISE_POWER = 5.2f; // 산봉우리 뾰족함 정도

float2 Hash22(float2 p)
{
    p = float2(dot(p, float2(127.1f, 311.7f)), dot(p, float2(269.5f, 183.3f)));
    p += NOISE_SEED;
    return -1.0f + 2.0f * frac(sin(p) * 43758.5453123f);
} // Hash22

float PerlinNoise(float2 p)
{
    float2 pi = floor(p); // 정수부
    float2 pf = frac(p); // 소수부

    // 부드러운 보간을 위한 Smoothstep (3x^2 - 2x^3)
    float2 w = pf * pf * (3.0f - 2.0f * pf);

    float n00 = dot(Hash22(pi + float2(0.0f, 0.0f)), pf - float2(0.0f, 0.0f));
    float n10 = dot(Hash22(pi + float2(1.0f, 0.0f)), pf - float2(1.0f, 0.0f));
    float n01 = dot(Hash22(pi + float2(0.0f, 1.0f)), pf - float2(0.0f, 1.0f));
    float n11 = dot(Hash22(pi + float2(1.0f, 1.0f)), pf - float2(1.0f, 1.0f));

    // 이중 선형 보간
    float nx0 = lerp(n00, n10, w.x);
    float nx1 = lerp(n01, n11, w.x);
    return lerp(nx0, nx1, w.y);
} // PerlinNoise

float TerrainFBM(float2 p)
{
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < OCTAVES; ++i)
    {
        total += PerlinNoise(p * frequency) * amplitude;
        maxValue += amplitude;
        
        amplitude *= PERSISTENCE;
        frequency *= LACUNARITY;
    }

    return (total / maxValue) * 0.5f + 0.5f;
} // TerrainFBM

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint width, height;
    OutHeightMap.GetDimensions(width, height);

    // 텍스처 범위를 벗어난 스레드는 종료
    if (DTid.x >= width || DTid.y >= height)
        return;

    // 현재 픽셀의 UV 좌표 (0.0 ~ 1.0) 계산
    float2 uv = float2((float) DTid.x / (float) width, (float) DTid.y / (float) height);
    
    // 스케일과 오프셋 적용
    float2 samplePos = (uv + NOISE_OFFSET) * NOISE_SCALE;

    // fBm 노이즈 생성 (0.0 ~ 1.0 범위)
    float noiseValue = TerrainFBM(samplePos);

    // 고도 대비 조절
    noiseValue = pow(abs(noiseValue), NOISE_POWER);

    // 결과 텍스처에 기록
    OutHeightMap[DTid.xy] = noiseValue;
} // main