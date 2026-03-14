// SkyBoxVS.hlsl
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

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
    
    // 뷰 행렬에서 이동(Translation) 성분 제거
    // 이렇게 하면 큐브 메쉬가 카메라를 완벽하게 따라다님
    float4x4 viewNoTranslation = viewMatrix;
    viewNoTranslation._41_42_43 = 0.0f;
    
    //  월드 변환 없이 바로 뷰/투영 적용 (입력된 큐브 로컬 좌표 기준)
    float4 viewPos = mul(input.position, viewNoTranslation);
    output.position = mul(viewPos, projectionMatrix);

    // 3. 핵심! Z = W 트릭 (깊이 강제 설정)
    // 픽셀 셰이더 단계에서 깊이는 Z/W 로 계산됩니다.
    // Z와 W를 똑같이 맞춰주면 Z/W = 1.0 (최대 깊이)가 되어 항상 Far Plane에 밀착되어 그려집니다.
    output.position.z = output.position.w;

    // 4. 큐브맵 샘플링을 위한 로컬 좌표 전달
    output.localPos = input.position.xyz;

    return output;
} // main