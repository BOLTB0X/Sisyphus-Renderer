#include "Pch.h"
#include "VolumetricCloud.h"
// Utils
#include "Helpers/DebugHelper.h"
#include "Helpers/ShaderHelper.h"
#include "SharedConstants/PathConstants.h"

using namespace DirectX;
using namespace SharedConstants;

VolumetricCloud::VolumetricCloud() {
	m_mesh = std::make_unique<DefaultMesh>();
} // VolumetricCloud

VolumetricCloud::~VolumetricCloud() {
} // ~VolumetricCloud

bool VolumetricCloud::Init(ID3D11Device* device, HWND hwnd) {
	if (!m_mesh->Init(device, 1, DefaultMesh::DefaultMeshType::Quad)) {
		return false;
	}
	if (!InitShader(device, hwnd)) {
		return false;
	}
	return true;
} // Init

void VolumetricCloud::Render(ID3D11DeviceContext* context, const RenderParams& params) {
	if (RenderShader(context, params)) {
		m_mesh->RenderBuffer(context);
		context->DrawIndexed(m_mesh->GetIndexCount(), 0, 0);
	} else {
		DebugHelper::DebugPrint("VolumetricCloud Shader Render 실패!");
	}
} // Render

bool VolumetricCloud::InitShader(ID3D11Device* device, HWND hwnd) {
	using namespace ShaderHelper;
	using namespace ConstantBuffer;
	using namespace PathConstants;

	D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	if (!InitVertexShader(device, hwnd, CLOUDRAY_VS,
		layoutDesc, ARRAYSIZE(layoutDesc), m_vertexShader.GetAddressOf(), m_layout.GetAddressOf())) {
		return false;
	}
	if (!InitPixelShader(device, hwnd, CLOUDRAY_PS, m_pixelShader.GetAddressOf())) {
		return false;
	}

	if (!InitConstantBuffer<CloudBuffer>(device, m_rayBuffer.GetAddressOf())) {
		return false;
	}
	return true;
} // InitShader

bool VolumetricCloud::UpdateCloudBuffer(ID3D11DeviceContext* context, const XMMATRIX& view, const XMMATRIX& projection, const XMFLOAT3& camPos) {
	using namespace ShaderHelper;
	using namespace ConstantBuffer;

	XMVECTOR det;
	XMMATRIX invV = XMMatrixInverse(&det, view);
	XMMATRIX invP = XMMatrixInverse(&det, projection);

	CloudBuffer buffer {};
	buffer.invView = XMMatrixTranspose(invV);
	buffer.invProjection = XMMatrixTranspose(invP);
	buffer.camPos = camPos;
	buffer.padding = 0.0f;

	//if (memcmp(&m_preRayBuffer, &buffer, sizeof(CloudBuffer)) == 0) {
	//	return true;
	//}
	if (!UpdateConstantBuffer(context, m_rayBuffer.Get(), buffer)) {
		return false;
	}
	m_preRayBuffer = buffer;
	return true;
} // UpdateCloudBuffer

bool VolumetricCloud::RenderShader(ID3D11DeviceContext* context, const RenderParams& params) {
	if (!UpdateCloudBuffer(context, params.view, params.projection, params.camPos)) {
		DebugHelper::DebugPrint("VolumetricCloud: Ray Buffer 업데이트 실패!");
		return false;
	}

	context->PSSetConstantBuffers(0, 1, m_rayBuffer.GetAddressOf());
	context->IASetInputLayout(m_layout.Get());

	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	return true;
} // RenderShader