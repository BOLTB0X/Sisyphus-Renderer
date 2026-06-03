// TransparentDepthPS.hlsl
// https://www.rastertek.com/dx11win10tut45.html

Texture2D    AlbedoTex : register(t0);
SamplerState LinearSampler : register(s0);

struct PS_IN
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXTURE0;
    float2 tex : TEXCOORD1;
}; // PS_IN

cbuffer CheckLeafBuffer : register(b3)
{
    int  isLeaf;
    int3 padding;
};

float4 main(PS_IN input) : SV_TARGET
{
    if (isLeaf == 1)
    {
        float depthValue;
        float4 textureColor;

        textureColor = AlbedoTex.Sample(LinearSampler, input.tex);
        if(textureColor.a > 1.0f)
        {
            depthValue = input.depthPosition.z / input.depthPosition.w;
        }
        else
        {
            discard;
        }

        return float4(depthValue, depthValue, depthValue, 1.0f);
    }
    return float4(0, 0, 0, 1);
} // main