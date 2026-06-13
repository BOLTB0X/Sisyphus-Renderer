#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include "Components/QuadTree.h"
#include "Resources/ConstantBuffer.h"
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
		ID3D11ShaderResourceView* colSRV;
        ID3D11ShaderResourceView* norSRV;
        ID3D11SamplerState*       linearSampler;

		InitParams() : device(nullptr), hwnd(nullptr), heightMapTex(nullptr),
            colSRV(nullptr), norSRV(nullptr), linearSampler(nullptr) {
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

    void                                        OnGui(ID3D11ShaderResourceView*);
    DirectX::XMMATRIX                           GetWorldMatrix();
    float                                       GetHeightAt(float, float) const;
    const std::vector<QuadTree::QuadTreeNode*>& GetVisibleNodes() const;

private:
    bool InitShader(ID3D11Device*, HWND);
    void GenerateTerrainGrid(int, int, float, std::vector<QuadTree::TerrainVertex>&, std::vector<UINT>&);

private:
    std::unique_ptr<QuadTree>                  m_quadTree;
    std::vector<QuadTree::QuadTreeNode*>       m_visibleNodes;
    std::vector<QuadTree::QuadTreeNode*>       m_allLeafNodes;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_worldBuffer;

    ConstantBuffer::WorldBuffer                m_worldData;
    std::shared_ptr<Texture>                   m_heightMap;
    ID3D11ShaderResourceView*                  m_colSRV;
    ID3D11ShaderResourceView*                  m_norSRV;
    ID3D11ShaderResourceView*                  m_objectShadowSRV;
    ID3D11ShaderResourceView*                  m_terrainShadowSRV;
    ID3D11SamplerState*                        m_linearSampler;

	float                                      m_quadMaxLeng;
	float                                      m_quadScale;
    float                                      m_heightScale;
}; // Ground