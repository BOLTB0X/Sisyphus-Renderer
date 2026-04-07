// CompositeVS.hlsl
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
}; // VS_OUT

PS_INPUT main(uint vID : SV_VertexID)
{
    PS_INPUT output;
 
    output.uv = float2((vID << 1) & 2, vID & 2);
    output.pos = float4(output.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 1.0f, 1.0f);
    
    return output;
} // main