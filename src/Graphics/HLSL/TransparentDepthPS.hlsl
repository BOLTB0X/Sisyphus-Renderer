// TransparentDepthPS.hlsl
// https://www.rastertek.com/dx11win10tut45.html

SamplerState LinearSampler : register(s0);
Texture2D    AlphaTex : register(t3);

struct PS_IN
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXTURE0;
    float2 tex : TEXCOORD1;
}; // PS_IN

float4 main(PS_IN input) : SV_TARGET
{
    float4 textureColor = AlphaTex.Sample(LinearSampler, input.tex);

    clip(textureColor.a - 0.1f);

    float depthValue = input.depthPosition.z / input.depthPosition.w;
    return float4(depthValue, depthValue, depthValue, 1.0f);
} // main