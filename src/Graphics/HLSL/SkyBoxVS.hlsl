// SkyBoxVS.hlsl
cbuffer CommonBuffer : register(b0)
{
    // [Row 1]
    matrix cWorld;
    // [Row 2]
    matrix cView;
    // [Row 3]
    matrix cProj;
    // [Row 4]
    float3 cCameraPosition;
    float cPadding1;
    // [Row 5]
    matrix cViewInv;
    // [Row 6]
    matrix cProjInv;
    // [Row 7]
    float3 cLightDirection;
    float cPadding2;
    // [Row 8]
    float4 cLightDiffuse;
}; // CommonBuffer

struct VS_INPUT
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
}; // VS_INPUT

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 localPos : TEXCOORD0;
}; // PS_INPUT

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    
    float4 worldPos = mul(input.position, cWorld);
    float4x4 viewNoTranslation = cView;
    viewNoTranslation._41_42_43 = 0.0f;
    
    //  월드 변환 없이 바로 뷰/투영 적용
    float4 viewPos = mul(worldPos, viewNoTranslation);
    //float4 viewPos = mul(input.position, viewNoTranslation);
    output.position = mul(viewPos, cProj);
    output.position.z = output.position.w;

    // 큐브맵 샘플링을 위한 로컬 좌표 전달
    output.localPos = input.position.xyz;

    return output;
} // main