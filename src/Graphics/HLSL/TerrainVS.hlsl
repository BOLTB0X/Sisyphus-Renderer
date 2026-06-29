// TerrainVS.hlsl
struct VS_IN
{
    float3 posW : POSITION;
    float2 uv : TEXCOORD;
}; // VS_IN

struct VS_OUT
{
    float3 posW : POSITION;
    float2 uv : TEXCOORD;
}; // VS_OUT

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    output.posW = input.posW;
    output.uv = input.uv;
    
    return output;
} // main