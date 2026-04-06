// VolumeSlicerCS.hlsl
Texture3D<float4> SrcVolume : register(t0);
SamplerState LinearSam : register(s0);

RWTexture2D<float4> OutSliceXY : register(u0); // XY 평면 (Front)
RWTexture2D<float4> OutSliceYZ : register(u1); // YZ 평면 (Side)
RWTexture2D<float4> OutSliceXZ : register(u2); // XZ 평면 (Top)

cbuffer SlicerParams : register(b0)
{
    float  sliceDepth;
    float3 sPadding;
}; // SlicerParams

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint outW, outH;
    OutSliceXY.GetDimensions(outW, outH);

    // 스레드가 텍스처 범위를 벗어나면 종료
    if (id.x >= outW || id.y >= outH)
        return;
    float2 uv = float2(id.xy) / float2(outW - 1, outH - 1);

    float3 uvwXY = float3(uv.x, uv.y, sliceDepth);
    float3 uvwYZ = float3(sliceDepth, uv.x, uv.y);
    float3 uvwXZ = float3(uv.x, sliceDepth, uv.y);

    // 3D 텍스처에서 값(밀도)을 샘플링
    float4 colXY = SrcVolume.SampleLevel(LinearSam, uvwXY, 0);
    float4 colYZ = SrcVolume.SampleLevel(LinearSam, uvwYZ, 0);
    float4 colXZ = SrcVolume.SampleLevel(LinearSam, uvwXZ, 0);

    // 샘플링된 값을 2D 텍스처(UAV)에 기록
    OutSliceXY[id.xy] = float4(colXY.r, colXY.r, colXY.r, 1.0f);
    OutSliceYZ[id.xy] = float4(colYZ.r, colYZ.r, colYZ.r, 1.0f);
    OutSliceXZ[id.xy] = float4(colXZ.r, colXZ.r, colXZ.r, 1.0f);
} // main