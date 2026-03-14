// CloudRayVS.hlsl
struct VS_INPUT
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD0;
}; // VS_INPUT

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD0;
}; // PS_INPUT

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = float4(input.position.xyz, 1.0f);    
    output.texCoord = input.texCoord;
    return output;
} // main