// CompositePS.hlsl
Texture2D    LowResCloudTex : register(t0);
Texture2D<float> mainDepth : register(t1);
SamplerState LinearSampler : register(s0);

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
}; // PS_INPUT

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 cloudColor = LowResCloudTex.Sample(LinearSampler, input.uv);
    return cloudColor;
    //return float4(1, 0, 0, 1);
} // main