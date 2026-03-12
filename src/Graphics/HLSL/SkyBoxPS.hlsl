// SkyBoxPS.hlsl
TextureCube tCubeMap : register(t0);
SamplerState sLinear : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 localPos : TEXCOORD0;
}; // PS_INPUT

float4 main(PS_INPUT input) : SV_TARGET
{
    return tCubeMap.Sample(sLinear, normalize(input.localPos));
} // main