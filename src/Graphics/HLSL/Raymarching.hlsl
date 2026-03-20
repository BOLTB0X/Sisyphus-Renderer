// Raymarching.hlsli
#define CLD_MARCH_STEPS 32 
#define CLD_THICK 90.0f
#define CLD_START_HEIGHT 150.0f 
#define CLD_ABSORB_COEFF 1.0f 
// Shadertoy의 Coverage: 낮을수록 구름이 많아지고, 높을수록 맑아짐 (0.3 ~ 0.5 추천)
#define CLD_COVERAGE 0.3125f 
#define CLD_SHARPNESS 0.2f    // 구름 외곽의 부드러운 정도

float GetCloudDensity(float3 pos, float3 windDir, Texture2D tex, SamplerState ss)
{
    // 1. 샘플링 좌표 설정
    float3 p = pos * 0.0015f + windDir;
    
    // 2. FBM (멀티 옥타브로 디테일 누적)
    float dens = 0.0f;
    float weight = 0.5f;
    for (int i = 0; i < 4; i++)
    {
        dens += tex.SampleLevel(ss, p.xz, 0).r * weight;
        p *= 2.5f;
        weight *= 0.5f;
    }

    // 3. 높이에 따른 모양 제어 (Height Gradient)
    // 구름의 하단은 평평하게, 상단은 몽글몽글하게 깎음
    float heightAlpha = (pos.y - CLD_START_HEIGHT) / CLD_THICK;
    float heightMask = smoothstep(0.0, 0.15, heightAlpha) * smoothstep(1.0, 0.7, heightAlpha);
    
    // 4. Contrast 및 Coverage 적용 (핵심!)
    // dens에서 CLD_COVERAGE를 빼서 낮은 값들을 0 이하로 밀어버림
    float finalDensity = max(0, dens - CLD_COVERAGE);
    
    // 5. 부드러운 경계 처리 및 밀도 강화
    finalDensity = smoothstep(0.0, CLD_SHARPNESS, finalDensity);
    
    return finalDensity * heightMask;
}

float4 RenderClouds(float3 viewDir, float3 sunDir, float time, Texture2D tex1, Texture2D tex2, SamplerState ss)
{
    // 지평선 아래는 연산 생략
    if (viewDir.y < 0.05f)
        return float4(0, 0, 0, 0);

    float march_step = CLD_THICK / (float) CLD_MARCH_STEPS;
    float3 rayDirProjected = viewDir / viewDir.y;
    float3 startPos = rayDirProjected * CLD_START_HEIGHT;
    float3 iter = rayDirProjected * march_step;
    
    float3 currentPos = startPos;
    
    // 시간 흐름에 따른 구름 이동 (Shadertoy의 cld_wind_dir 이식)
    float3 windDir = float3(0, 0, -time * 0.002f);

    float T = 1.0f; // 초기 투과율 100%
    float3 C = float3(0, 0, 0);
    float alpha = 0.0f;

    for (int i = 0; i < CLD_MARCH_STEPS; i++)
    {
        // 현재 고도를 0.0 ~ 1.0 사이로 정규화 (Shadertoy의 cloud.height)
        float h = (currentPos.y - startPos.y) / CLD_THICK;
        float density = pow(GetCloudDensity(currentPos, windDir, tex1, ss), 2.0f);
        
        if (density > 0.001f)
        {
            // 빛 투과율 계산 (Beer-Lambert 법칙)
            float Ti = exp(-CLD_ABSORB_COEFF * density * march_step);
            float3 cloudCol = lerp(float3(0.6, 0.7, 0.8), float3(1.0, 1.0, 1.0), density);
            // Shadertoy식 고도 기반 조명: 구름 위쪽이 더 밝게 빛남
            float localLight = exp(h) / 1.95f;
            float3 lightColor = float3(1.0f, 0.95f, 0.9f) * localLight;
            
            C += T * density * march_step * cloudCol;
            T *= Ti;
            alpha += (1.0f - Ti) * (1.0f - alpha);
        }
        
        currentPos += iter;
        
        if (alpha > 0.99f)
            break;
    }

    // 외곽선 부분의 알파값을 부드럽게 깎아주기 위한 컷오프
    float cutoff = dot(viewDir, float3(0, 1, 0));
    return float4(C, alpha * smoothstep(0.0f, 0.2f, cutoff));
}