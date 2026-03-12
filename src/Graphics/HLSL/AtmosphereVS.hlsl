// AtmosphereVS.hlsl
cbuffer MatrixBuffer : register(b0)
{
    matrix mWorldMatrix;
    matrix mViewMatrix;
    matrix mProjectionMatrix;
}; // MatrixBuffer

struct VS_INPUT
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD0;
}; // VS_INPUT

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 localPos : TEXCOORD0;
}; // PS_INPUT

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
        
    float4x4 viewNoTranslation = mViewMatrix;
    viewNoTranslation._41 = 0.0f;
    viewNoTranslation._42 = 0.0f;
    viewNoTranslation._43 = 0.0f;

    float4 viewPos = mul(input.position, viewNoTranslation);
    output.position = mul(viewPos, mProjectionMatrix);
    
    // Raymarching의 방향성 데이터
    output.localPos = input.position.xyz;
    
    return output;
} // main