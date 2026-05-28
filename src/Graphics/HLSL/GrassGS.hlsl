// GrassGS.hlsl
#include "Common.hlsli"
#define GRASS_WIDTH 0.15f

struct GS_IN
{
    float3 worldPos : POSITION;
    float height : HEIGHT;
    float2 uv : TEXCOORD;
}; // GS_IN

struct GS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD1;
}; // GS_OUT

[maxvertexcount(24)] // 십자형: 2방향 * 2삼각형 * 3버텍스 * 2면 = 24
void main(point GS_IN input[1],
          inout TriangleStream<GS_OUT> stream)
{
    float3 root = input[0].worldPos;
    float height = input[0].height;

    // 바람 계산 (끝부분만 흔들림)
    float windStrength = 0.3f;
    float windSpeed = 2.0f;
    float windX = sin(TIME * windSpeed + root.x * 0.5f + root.z * 0.3f) * windStrength;
    float windZ = cos(TIME * windSpeed * 0.7f + root.z * 0.4f) * windStrength * 0.5f;

    float4 view = mul(float4(0, 0, 0, 1), cView); // 미사용, 빌보드용

    // 두 방향으로 십자형 생성
    float3 dirs[2] =
    {
        float3(1, 0, 0), // X축 방향
        float3(0, 0, 1) // Z축 방향
    };

    [unroll]
    for (int d = 0; d < 2; ++d)
    {
        float3 right = dirs[d] * GRASS_WIDTH;
        float3 top = float3(windX, height, windZ); // 끝부분 바람

        // 4개 버텍스 (쿼드)
        float3 v[4] =
        {
            root - right, // 왼쪽 아래
            root + right, // 오른쪽 아래
            root - right + top, // 왼쪽 위 (바람 적용)
            root + right + top // 오른쪽 위 (바람 적용)
        };

        float2 uvs[4] =
        {
            float2(0, 1),
            float2(1, 1),
            float2(0, 0),
            float2(1, 0)
        };

        float3 normal = normalize(cross(right, float3(0, 1, 0)));

        // 앞면
        GS_OUT p;
        int triIdx[6] = { 0, 2, 1, 1, 2, 3 };
        [unroll]
        for (int i = 0; i < 6; ++i)
        {
            p.worldPos = v[triIdx[i]];
            p.position = mul(mul(float4(v[triIdx[i]], 1.0f), cView), cProjection);
            p.uv = uvs[triIdx[i]];
            p.normal = normal;
            stream.Append(p);
        }
        stream.RestartStrip();

        // 뒷면 (컬링 없이 양면 렌더링)
        int triIdxBack[6] = { 1, 2, 0, 3, 2, 1 };
        [unroll]
        for (int j = 0; j < 6; ++j)
        {
            p.worldPos = v[triIdxBack[j]];
            p.position = mul(mul(float4(v[triIdxBack[j]], 1.0f), cView), cProjection);
            p.uv = uvs[triIdxBack[j]];
            p.normal = -normal;
            stream.Append(p);
        }
        stream.RestartStrip();
    }
} // main