#pragma once
#include <d3d11.h>
#include <wrl/client.h>

// 라스터라이저 상태(와이어프레임/솔리드), 깊이 스텐실 상태 등 GPU의 "설정 값"들 그릇
class D3D11State {
public:
    D3D11State();
    D3D11State(const D3D11State& other);
    ~D3D11State();

    bool Init(ID3D11Device* device);

    ID3D11RasterizerState*   GetCullBackState() const;
    ID3D11RasterizerState*   GetCullNone() const;
    ID3D11DepthStencilState* GetDepthState() const;
    ID3D11DepthStencilState* GetDepthNone() const;
    ID3D11DepthStencilState* GetDepthLessEqual() const;
    ID3D11SamplerState*      GetLinearSamplerState() const;
    ID3D11BlendState*        GetBlendState() const;

private:
    bool InitCullBack(ID3D11Device*);
    bool InitWireframe(ID3D11Device*);
	bool InitCullNone(ID3D11Device*);
    bool InitDepth(ID3D11Device*);
    bool InitDepthNone(ID3D11Device*);
    bool InitDepthLess(ID3D11Device*);
    bool InitSampler(ID3D11Device*, D3D11_FILTER,
                     D3D11_TEXTURE_ADDRESS_MODE,
                     ID3D11SamplerState**);
    bool InitBlendState(ID3D11Device*);

private:
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>   m_cullBackState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>   m_wireframeState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>   m_cullNoneState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthNoneState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthLessEqualState;
    Microsoft::WRL::ComPtr<ID3D11SamplerState>      m_linearSamplerState;
    Microsoft::WRL::ComPtr<ID3D11BlendState>        m_blendState;
}; // D3D11State