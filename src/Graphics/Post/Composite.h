#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <directxmath.h>
#include "Utils/SharedConstants/PathConstants.h"

class Composite {
public:
	struct InitParams {
		ID3D11Device* device;
		HWND         hwnd;
		std::wstring  vPath;
		std::wstring  pPath;
		InitParams() : device(nullptr), hwnd(nullptr), 
			vPath(L""), pPath(L"") {
		}
	}; // InitParams

public:
	Composite();
	~Composite();
	bool Init(const InitParams&);
	void Render(ID3D11DeviceContext* context, ID3D11ShaderResourceView*, ID3D11SamplerState*);

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
}; // Composite