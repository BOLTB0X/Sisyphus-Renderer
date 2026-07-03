// TerrainDepthDS.hlsl
struct PatchTess
{
    float edgeTess[4] : SV_TessFactor;
    float insideTess[2] : SV_InsideTessFactor;
}; // PatchTess

struct HS_OUT
{
    float3 posW : POSITION;
    float2 uv : TEXCOORD;
}; // HS_OUT

Texture2D    HeightMap : register(t0);
SamplerState LinearSampler : register(s0);

cbuffer WorldBuffer : register(b2)
{
    matrix cWorld;
}; // WorldBuffer

cbuffer HeightScaleBuffer : register(b4)
{
    float hHeightScale;
    float3 hPadding;
}; // HeightScaleBuffer

cbuffer LightMatrixBuffer : register(b1)
{
    matrix cLightView;
    matrix cLightProj;
}; // LightMatrixBuffer

#define HEIGHT_SCALE hHeightScale

[domain("quad")]
float4 main(
    PatchTess patchTess,
    float2 domain : SV_DomainLocation,
    const OutputPatch<HS_OUT, 4> patch) : SV_POSITION
{
    float3 bottomPos = lerp(patch[0].posW, patch[1].posW, domain.x);
    float3 topPos = lerp(patch[3].posW, patch[2].posW, domain.x);
    float3 finalPosL = lerp(bottomPos, topPos, domain.y);

    float2 bottomUV = lerp(patch[0].uv, patch[1].uv, domain.x);
    float2 topUV = lerp(patch[3].uv, patch[2].uv, domain.x);
    float2 finalUV = lerp(bottomUV, topUV, domain.y);

    float heightOutput = HeightMap.SampleLevel(LinearSampler, finalUV, 0).r;
    finalPosL.y += heightOutput * HEIGHT_SCALE;

    float3 finalPosW = mul(float4(finalPosL, 1.0f), cWorld).xyz;

    float4 posH = mul(float4(finalPosW, 1.0f), cLightView);
    posH = mul(posH, cLightProj);

    return posH;
} // main