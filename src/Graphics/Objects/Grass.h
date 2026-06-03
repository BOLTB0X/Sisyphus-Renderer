#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <memory>
#include <vector>
// Components
#include "Components/QuadTree.h"
// Utils
#include "SharedConstants/BuffersConstants.h"

class Frustum;

class Grass {
public:
    struct InitParams {
        ID3D11Device*             device;
        HWND                      hwnd;
        ID3D11ShaderResourceView* grass;
        ID3D11SamplerState*       linearSampler;

        InitParams() : device(nullptr), hwnd(0), 
            grass(nullptr), linearSampler(nullptr) {
		}
    }; // InitParams

    struct RenderParams {
        Frustum*                                    frustum;
        const std::vector<QuadTree::QuadTreeNode*>* visibleNodes;

        RenderParams() : frustum(nullptr), visibleNodes(nullptr) {
        }
    }; // RenderParams

public:
    Grass();
    ~Grass();

    bool Init(const InitParams&);
    void Render(ID3D11DeviceContext*, const RenderParams&);
    void RenderFar(ID3D11DeviceContext*, const RenderParams&);
    void OnGui();

private:
    struct GrassBuffer {
        float width;
        float height;
        float windStrength;
        float windSpeed;

        float             dist;
        float             alphaCut;
        DirectX::XMFLOAT2 padding;

        GrassBuffer() {
            using namespace SharedConstants;
            width = BuffersConstants::GRASS_BLADE_WIDTH;
			height = BuffersConstants::GRASS_BLADE_HEIGHT;
			windStrength = BuffersConstants::WIND_STRENGTH;
			windSpeed = BuffersConstants::WIND_SPEED;
            dist = BuffersConstants::DIST;
            alphaCut = BuffersConstants::ALPHA_CUT;
			padding = DirectX::XMFLOAT2(0.0f, 0.0f);
        }
    }; // GrassBuffer

private:
    bool InitShader(ID3D11Device*, HWND);
    bool UpdateGrassBuffer(ID3D11DeviceContext*);

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader>   m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_geometryShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>    m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>    m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>         m_grassBuffer;

    Microsoft::WRL::ComPtr<ID3D11VertexShader>   m_farVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>    m_farPixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>    m_farLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>         m_quadVertexBuffer;

    ID3D11ShaderResourceView*                    m_grassSRV;
    ID3D11SamplerState*                          m_linearSampler;

    GrassBuffer                                  m_grassData;
    GrassBuffer                                  m_prevGrassData;
}; // Grass