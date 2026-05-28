#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <memory>
#include <vector>
// Components
#include "Components/QuadTree.h"

class Frustum;

class Grass {
public:
    struct InitParams {
        ID3D11Device* device;
        HWND          hwnd;
    }; // InitParams

    struct RenderParams {
        Frustum*                                    frustum;
        float                                       time;
        const std::vector<QuadTree::QuadTreeNode*>* visibleNodes;

        RenderParams() : frustum(nullptr), time(0.0f), visibleNodes(nullptr) {
        }
    }; // RenderParams

    Grass();
    ~Grass();

    bool Init(const InitParams&);
    void Render(ID3D11DeviceContext*, const RenderParams&);

private:
    bool InitShader(ID3D11Device*, HWND);

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader>   m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_geometryShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>    m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>    m_layout;
}; // Grass