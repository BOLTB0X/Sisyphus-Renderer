#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include "Components/QuadTree.h"
#include "Resources/ConstantBufferType.h"
// Utils
#include "SharedConstants/BuffersConstants.h"

class Frustum;
class Texture;

class Ground {
public:
    struct InitParams {
        ID3D11Device*             device;
        HWND                      hwnd;
        std::shared_ptr<Texture>  heightMapTex;
		ID3D11ShaderResourceView* farawayGrassSRV;
		ID3D11ShaderResourceView* colSRV;
		ID3D11ShaderResourceView* ambSRV;
		ID3D11ShaderResourceView* norSRV;
		ID3D11ShaderResourceView* rouSRV;
		ID3D11ShaderResourceView* disSRV;
        ID3D11SamplerState*       linearSampler;

		InitParams() : device(nullptr), hwnd(nullptr), heightMapTex(nullptr),
            farawayGrassSRV(nullptr), colSRV(nullptr), ambSRV(nullptr),
            norSRV(nullptr), rouSRV(nullptr), disSRV(nullptr),
            linearSampler(nullptr) {
        }
    }; // InitParams

    struct RenderParams {
        DirectX::XMFLOAT3         cameraPosition;
        float                     time;
        Frustum*                  frustum;

        RenderParams() : cameraPosition(0.0f, 0.0f, 0.0f), time(0.0f), frustum(nullptr) {
        }
    }; // RenderParams

public:
    Ground();
    ~Ground();

    bool Init(const InitParams&);
    void Render(ID3D11DeviceContext*, const RenderParams&);
    void DrawIndexed(ID3D11DeviceContext*);

    void                                        OnGui();
    DirectX::XMMATRIX                           GetWorldMatrix();
    float                                       GetHeightAt(float, float) const;
    const std::vector<QuadTree::QuadTreeNode*>& GetVisibleNodes() const;

private:
    struct GroundBuffer {
        DirectX::XMFLOAT3 darkSand;
        float             padding1;
        DirectX::XMFLOAT3 lightSand;
        float             cameraDist;

        GroundBuffer() {
            darkSand = SharedConstants::BuffersConstants::DARK_SAND;
            padding1 = 0.0f;
            lightSand = SharedConstants::BuffersConstants::LIGHT_SAND;
            cameraDist = SharedConstants::BuffersConstants::DIST;
        }
    }; // GroundBuffer

    struct WorldBuffer {
        DirectX::XMMATRIX world;

        WorldBuffer() {
            world = DirectX::XMMatrixIdentity();
        }
    }; // WorldBuffer;

private:
    bool InitShader(ID3D11Device*, HWND);

    bool UpdateGroundBuffer(ID3D11DeviceContext*);
    void GenerateTerrainGrid(int, int, float, std::vector<QuadTree::TerrainVertex>&, std::vector<UINT>&);

private:
    std::unique_ptr<QuadTree>                  m_quadTree;
    std::vector<QuadTree::QuadTreeNode*>       m_visibleNodes;
    std::vector<QuadTree::QuadTreeNode*>       m_allLeafNodes;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_worldBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_groundBuffer;

    WorldBuffer                                m_worldData;
    GroundBuffer                               m_GoundData;
    GroundBuffer                               m_prevGoundData;
    std::shared_ptr<Texture>                   m_heightMap;
    ID3D11ShaderResourceView*                  m_farawayGrassSRV;
    ID3D11ShaderResourceView*                  m_colSRV;
    ID3D11ShaderResourceView*                  m_ambSRV;
    ID3D11ShaderResourceView*                  m_norSRV;
    ID3D11ShaderResourceView*                  m_rouSRV;
    ID3D11ShaderResourceView*                  m_disSRV;
    ID3D11ShaderResourceView*                  m_objectShadowSRV;
    ID3D11ShaderResourceView*                  m_terrainShadowSRV;
    ID3D11SamplerState*                        m_linearSampler;

	float                                      m_quadMaxLeng;
	float                                      m_quadScale;
    float                                      m_heightScale;
}; // Ground