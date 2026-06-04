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
    float alpha = AlphaTex.Sample(LinearSampler, input.tex).r;
    
    clip(alpha - 0.1f);
    
    return float4(0, 0, 0, 1);
} // main