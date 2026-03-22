#pragma once
#include <wrl/client.h>
#include <string>
#include "Utils/SharedConstants/BuffersConstants.h"

class VolumeTexture;

class NoiseGenerator {
public:
    struct NoiseBuffer {
        // Row 1
        DirectX::XMFLOAT3 textureSize;
        float             perlinFreq;
        // Row 2
        float             worleyFreq;
        float             detailFreqG;
        float             detailFreqB;
        float             detailFreqA;
        // Row 3
        int               octaves;
        float             remapBias;
        DirectX::XMFLOAT2 padding;

        NoiseBuffer() {
            textureSize = SharedConstants::BuffersConstants::TEXTURE_SIZE;
            perlinFreq = SharedConstants::BuffersConstants::PERLIM_FREQ;
            worleyFreq = SharedConstants::BuffersConstants::WORLEY_FREQ;
            detailFreqG = SharedConstants::BuffersConstants::VOL_FREQ_G;
            detailFreqB = SharedConstants::BuffersConstants::VOL_FREQ_B;
            detailFreqA = SharedConstants::BuffersConstants::VOL_FREQ_A;
            octaves = SharedConstants::BuffersConstants::VOL_OCTAVES;
            remapBias = SharedConstants::BuffersConstants::VOL_REMAP_BIAS;
            padding = { 0.0f, 0.0f };
        }

        NoiseBuffer(DirectX::XMFLOAT3 texSize, float perlinF,
            float worleyF, float detailFG, float detailFB, float detailFA,
            int octs, float remapB)
            : textureSize(texSize), perlinFreq(perlinF),
            worleyFreq(worleyF), detailFreqG(detailFG), detailFreqB(detailFB), detailFreqA(detailFA),
            octaves(octs), remapBias(remapB), padding(0.0f, 0.0f) {
        }
    }; // NoiseBuffer

public:
    NoiseGenerator();
    ~NoiseGenerator();

    bool Init(ID3D11Device*, HWND, const std::wstring& path);
    void Generate(ID3D11DeviceContext*, VolumeTexture*, const NoiseBuffer&);

private:
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>        m_noiseBuffer;
    NoiseBuffer                                 m_preNoiseBuffer;
}; // NoiseGenerator