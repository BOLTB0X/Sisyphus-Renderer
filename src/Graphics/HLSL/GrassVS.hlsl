// GrassVS.hlsl
struct VS_IN
{
    float3 position : POSITION;
    float  height : HEIGHT;
    float2 uv : TEXCOORD;
}; // VS_IN

struct GS_IN
{
    float3 worldPos : POSITION;
    float  height : HEIGHT;
    float2 uv : TEXCOORD;
}; // GS_IN

GS_IN main(VS_IN input)
{
    GS_IN output;
    output.worldPos = input.position;
    output.height = input.height;
    output.uv = input.uv;
    return output;
} // GS_IN