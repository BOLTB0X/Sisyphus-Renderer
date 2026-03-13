// SkyBoxPS.hlsl
TextureCube CubeMapA : register(t0);
TextureCube CubeMapB : register(t1);
SamplerState LinerSampler : register(s0);

cbuffer BlendBuffer : register(b1)
{
    float bBlendFactor;
    float3 bPadding;
}; // BlendBuffer

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 localPos : TEXCOORD0;
}; // PS_INPUT

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 colA = CubeMapA.Sample(LinerSampler, input.localPos).rgb;
    float3 colB = CubeMapB.Sample(LinerSampler, input.localPos).rgb;
    
    float3 finalCol = lerp(colA, colB, bBlendFactor);
    return float4(finalCol, 1.0f);
} // main