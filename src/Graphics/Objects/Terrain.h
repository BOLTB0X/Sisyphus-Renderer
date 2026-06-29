#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include "Resources/ConstantBuffer.h"
#include "SharedConstants/BuffersConstants.h"
#include "SharedConstants/CommonConstants.h"

class Frustum;
class Texture;

class Terrain {
public:
    struct TerrainVertex {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT2 texcoord;
    }; // TerrainVertex

    struct InitParams {
        ID3D11Device*             device;
        HWND                      hwnd;
        std::shared_ptr<Texture>  heightMapTex;
        ID3D11ShaderResourceView* colSRV;
        ID3D11ShaderResourceView* sandSRV;
        ID3D11ShaderResourceView* grassSRV;
        ID3D11ShaderResourceView* snowSRV;
        ID3D11ShaderResourceView* diffSRV;
        ID3D11ShaderResourceView* norSRV;
        ID3D11SamplerState*       linearSampler;

        InitParams() : device(nullptr), hwnd(nullptr), heightMapTex(nullptr),
            colSRV(nullptr), sandSRV(nullptr), grassSRV(nullptr),
            snowSRV(nullptr), diffSRV(nullptr), norSRV(nullptr), linearSampler(nullptr) {
        }
    }; // InitParams

    struct RenderParams {
        DirectX::XMFLOAT3         cameraPosition;
        float                     time;
        Frustum*                  frustum;

        RenderParams() : cameraPosition(0.0f, 0.0f, 0.0f), time(0.0f), frustum(nullptr) {}
    }; // RenderParams

public:
    Terrain();
    ~Terrain();

    bool Init(const InitParams&);
    void Render(ID3D11DeviceContext*, const RenderParams&);
    void RenderShadow(ID3D11DeviceContext*);

    void              OnGui();
    DirectX::XMMATRIX GetWorldMatrix();
    float             GetHeightAt(float, float) const;

private:
    struct HeightScaleBuffer {
        float             heightScale;
        DirectX::XMFLOAT3 padding;

        HeightScaleBuffer() : heightScale(SharedConstants::BuffersConstants::HEIGHT_SCALE), padding(0.0f, 0.0f, 0.0f) {
        }
    }; // HeightScaleBuffer

    struct TerrainBlendingBuffer {
        float             waterLevel;
        float             transZone;
        float             grassLimit;
        float             snowHeight;

        float             uv_tiling;
        DirectX::XMFLOAT3 padding;

        TerrainBlendingBuffer() : waterLevel(SharedConstants::CommonConstants::WATER_HEIGHT), transZone(3.0f), grassLimit(0.8f), snowHeight(100.0f),
            uv_tiling(37.0f), padding(0.0f, 0.0f, 0.0f) {
        } // TerrainBlendingBuffer
    }; // TerrainBlendingBuffer

private:
    bool InitShader(ID3D11Device*, HWND);
    void GeneratePatchGrid(int, int, float, std::vector<TerrainVertex>&, std::vector<UINT>&);

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_indexBuffer;
    UINT                                       m_indexCount;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11HullShader>   m_hullShader;
    Microsoft::WRL::ComPtr<ID3D11DomainShader> m_domainShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;

    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_worldBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_tessellationBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_heightScaleBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_terrainBlendingBuffer;
    ConstantBuffer::WorldBuffer                m_worldData;
    ConstantBuffer::TessellationControlBuffer  m_tessellationData;
    HeightScaleBuffer                          m_heightSacleData;
    TerrainBlendingBuffer                      m_terrainBlending;

    std::shared_ptr<Texture>                   m_heightMap;
    ID3D11ShaderResourceView*                  m_colSRV;
    ID3D11ShaderResourceView*                  m_sandSRV;
    ID3D11ShaderResourceView*                  m_grassSRV;
    ID3D11ShaderResourceView*                  m_snowSRV;
    ID3D11ShaderResourceView*                  m_diffSRV;
    ID3D11ShaderResourceView*                  m_norSRV;
    ID3D11SamplerState*                        m_linearSampler;
    int                                        m_patchCountX;
    int                                        m_patchCountZ;
    float                                      m_patchSize;
}; // Terrain