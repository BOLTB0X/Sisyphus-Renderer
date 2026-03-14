// CloudRayPS.hlsl
cbuffer RayBuffer : register(b0)
{
    matrix invView;
    matrix invProjection;
    float3 cameraPos;
    float padding;
}; // RayBuffer

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD0;
}; // PS_INPUT

bool rayAABB(float3 rayOrigin, float3 rayDir, float3 boxMin, float3 boxMax, out float tMin, out float tMax)
{
    float3 invDir = 1.0 / rayDir;
    float3 t0 = (boxMin - rayOrigin) * invDir;
    float3 t1 = (boxMax - rayOrigin) * invDir;
    
    float3 tmin_v = min(t0, t1);
    float3 tmax_v = max(t0, t1);
    
    tMin = max(max(tmin_v.x, tmin_v.y), tmin_v.z);
    tMax = min(min(tmax_v.x, tmax_v.y), tmax_v.z);
    
    return tMax > max(tMin, 0.0);
} // rayAABB

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 clipPos = float4(input.texCoord.xy, 1.0f, 1.0f);
    
    float3 boxMin = float3(-20, 5, -20); // 구름 박스 최소값
    float3 boxMax = float3(20, 10, 20); // 구름 박스 최대값
    
    float4 viewPos = mul(clipPos, invProjection);
    viewPos /= viewPos.w; // w 나누기 (원근 투영 복원)
    float3 worldDir = normalize(mul(float4(viewPos.xyz, 0.0f), invView).xyz);
    
    //float3 color = worldDir * 0.5f + 0.5f;
    
    float tNear, tFar;
    if (rayAABB(cameraPos, worldDir, boxMin, boxMax, tNear, tFar))
    {
        // 박스에 부딪힌 경우: 여기서부터 레이마칭 시작!
        // 일단 박스 영역을 시각화하기 위해 색상
        return float4(1, 1, 1, 1);
    }

    // 박스 밖은 배경색(하늘) 출력
    return float4(0.5, 0.7, 1.0, 1.0);
} // main