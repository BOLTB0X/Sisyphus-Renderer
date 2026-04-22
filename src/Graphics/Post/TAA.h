#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <directxmath.h>
#include <memory>

class RenderTexture;

class TAA {
public:
	struct InitParams {
		ID3D11Device* device;
		HWND         hwnd;
		std::wstring  vPath;
		std::wstring  pPath;
		int           ScreenWidth;
		int		      ScreenHeight;

		InitParams() : device(nullptr), hwnd(nullptr),
			vPath(L""), pPath(L""),
			ScreenWidth(0), ScreenHeight(0) {
		}
	}; // InitParams

	struct RenderParams {
		ID3D11ShaderResourceView* currentSRV;
		ID3D11SamplerState*       linerSampler;
		float                     blendFactor;
		DirectX::XMFLOAT2         texelSize;

		RenderParams() : currentSRV(nullptr), linerSampler(nullptr),
			blendFactor(0.0f), texelSize(0.0f, 0.0f) {
		}
	}; // RenderParams

public:
	TAA();
	~TAA();

	bool Init(const InitParams&);
	void Render(ID3D11DeviceContext*, const RenderParams&);
	void CopyResource(ID3D11DeviceContext*, ID3D11Texture2D*);

private:
	struct TAABuffer {
		float             blendFactor;
		DirectX::XMFLOAT3 padding1;
		DirectX::XMFLOAT2 texelSize;
		DirectX::XMFLOAT2 padding2;

		TAABuffer() : blendFactor(1.0f), padding1(0.0f, 0.0f, 0.0f),
			texelSize(0.0f, 0.0f), padding2(0.0f, 0.0f) {
		}
	}; // TAABuffer

private:
	std::unique_ptr<RenderTexture>             m_historyRT;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>       m_TAABuffer;
	TAABuffer                                  m_taaData;
}; // TAA
