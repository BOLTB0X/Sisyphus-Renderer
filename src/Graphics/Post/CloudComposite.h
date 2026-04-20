#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <directxmath.h>
#include "Utils/SharedConstants/PathConstants.h"
#include "Resources/ConstantBufferType.h"

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
	void Render(ID3D11DeviceContext* context, const RenderParams&);

private:
	bool UpdateResolutionBuffer(ID3D11DeviceContext*);

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>       m_resolutionBuffer;
	ConstantBuffer::ResolutionBuffer           m_resolutionData;
	ConstantBuffer::ResolutionBuffer           m_prevResolutionData;
}; // CloudComposite