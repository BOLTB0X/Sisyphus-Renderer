#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <directxmath.h>
#include <memory>
#include "Utils/SharedConstants/PathConstants.h"
#include "Resources/ConstantBufferType.h"

class RenderTexture;

class CloudComposite {
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
		ID3D11ShaderResourceView* sceneSRV;
		ID3D11ShaderResourceView* cloudSRV;
		ID3D11SamplerState*       linerSampler;
	}; // RenderParams

public:
	CloudComposite();
	~CloudComposite();
	bool Init(const InitParams&);
	void Render(ID3D11DeviceContext*, const RenderParams&);

	void                      ClearRT(ID3D11DeviceContext*);
	ID3D11Texture2D*          GetTexture() const;
	ID3D11RenderTargetView*   GetRTV() const;
	ID3D11ShaderResourceView* GetSRV() const;

private:
	bool UpdateResolutionBuffer(ID3D11DeviceContext*);

private:
	std::unique_ptr<RenderTexture>             m_compositeRT;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>       m_resolutionBuffer;
	ConstantBuffer::ResolutionBuffer           m_resolutionData;
	ConstantBuffer::ResolutionBuffer           m_prevResolutionData;
}; // CloudComposite