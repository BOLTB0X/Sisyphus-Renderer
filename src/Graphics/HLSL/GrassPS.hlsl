// GrassPS.hlsl
#include "Common.hlsli"

Texture2D GrassTex : register(t0);
SamplerState LinearSampler : register(s0);

struct GS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD1;
}; // GS_OUT

float4 main(GS_OUT input) : SV_TARGET
{
    float4 col = GrassTex.Sample(LinearSampler, input.uv);

    // 알파 컷아웃
    clip(col.a - 0.3f);

    // 라이팅
    float3 N = normalize(input.normal);
    float3 L = normalize(-LIGHT_DIRECTION);
    float diff = saturate(dot(N, L)) * 0.7f + 0.3f; // 0.3 ambient

    // 높이 기반 색상 그라디언트 (뿌리는 어둡게, 끝은 밝게)
    float heightFactor = saturate(input.uv.y); // uv.y=1이 뿌리, 0이 끝
    float3 baseColor = col.rgb * lerp(0.4f, 1.0f, 1.0f - heightFactor);

    return float4(baseColor * diff, col.a);
} // main