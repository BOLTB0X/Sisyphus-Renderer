// Debugs.hlsli
#ifndef _DEBUGS_HLSLI_
#define _DEBUGS_HLSLI_
#include "Common.hlsli"
#include "Atmosphere.hlsli"

float4 raymarch_AABB_debug(float3 ro, float3 rd, float maxDist, float3 boxCenter, float3 boxSize)
{
    // 박스 교차 판정 (AABB)
    float3 boxMin = boxCenter.xyz - boxSize.xyz * 0.5f;
    float3 boxMax = boxCenter.xyz + boxSize.xyz * 0.5f;
    
    float3 invRd = 1.0f / rd;
    float3 t0 = (boxMin - ro) * invRd;
    float3 t1 = (boxMax - ro) * invRd;
    
    float3 tmin = min(t0, t1);
    float3 tmax = max(t0, t1);
    
    float tNear = max(max(tmin.x, tmin.y), tmin.z);
    float tFar = min(min(tmax.x, tmax.y), tmax.z);
    
    // 박스를 빗나갔거나 지형(maxDist)보다 뒤에 있으면 빈 공간 반환
    if (tNear > tFar || tNear > maxDist || tFar < 0.0f)
        return float4(0, 0, 0, 0); // 투명
        
    // 박스에 맞았다면, 박스의 앞면 위치를 구해서 UVW 컬러로 시각화
    tNear = max(tNear, 0.0f);
    float3 hitPos = ro + rd * tNear;
    float3 uvw = get_box_uvw(hitPos, boxCenter.xyz, boxSize.xyz);
    
    return float4(uvw, 0.5f); // RGB에 UVW 값을 넣어 매핑이 잘 되는지 확인!
} // raymarch_AABB_debug

float4 raymarch_spherical_debug(float3 ro_meter, float3 rd, float maxDist, float earthRadius, float minHeight, float maxHeight)
{
    // 지구 곡률이 반영된 껍질 영역 찾기
    float2 cloudT = ray_shell_intersection(ro_meter, rd, earthRadius, minHeight, maxHeight);
    
    float tNear = cloudT.x;
    float tFar = cloudT.y;

    //  유효성 체크
    if (tNear > tFar || tNear > maxDist || tFar < 0.0f)
        return float4(0, 0, 0, 0);

    // 테스트용 시각화
    // 구름 껍질의 진입점 위치를 계산
    float3 hitPos = ro_meter + rd * tNear;

    float distanceDistortion = saturate(tNear / 100000.0); // 100km 기준
    return float4(1.0, 0.2, 0.2, 0.5 * (1.0 - distanceDistortion));
} // raymarch_spherical_debug

float4 check_sun_luminance(float2 uv, float2 sunPos, Texture2D tex, SamplerState samp)
{
    float2 diff = abs(uv - sunPos);
    if (diff.x < 0.005f && diff.y < 0.005f)
        return float4(1, 0, 0, 1);
    return float4(tex.SampleLevel(samp, uv, 0).rgb, 1.0f);
} // check_sun_luminance

#endif // _DEBUGS_HLSLI_