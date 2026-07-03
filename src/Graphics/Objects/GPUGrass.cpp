#include "Pch.h"
#include "GPUGrass.h"
#include "Helpers/ShaderHelper.h"
#include "Helpers/DebugHelper.h"
#include "SharedConstants/PathConstants.h"
#include "SharedConstants/CommonConstants.h"
// dx11
#include <d3dcompiler.h>

// define
#define GRASS_BUFFER_SLOT   3
#define PLACE_BUFFER_SLOT   4
#define WORLD_BUFFER_SLOT   5
#define GRASS_TEXTURE_SLOT  0
#define GRASS_SAMPLER_SLOT  0
#define QUAD_VERTEX_COUNT   4
#define TEX_HEIGHT_SLOT     0
#define TEX_NORMAL_SLOT     1
#define MAX_GRASS_INSTANCES 1000000

using namespace DirectX;
using namespace ShaderHelper;
using namespace SharedConstants;
using namespace DebugHelper;

GPUGrass::GPUGrass() {
    m_grassSRV = nullptr;
    m_linearSampler = nullptr;
    m_prevGrassData.padding.x = -1.0f;
} // GPUGrass   

GPUGrass::~GPUGrass() {
    m_grassSRV = nullptr;
    m_linearSampler = nullptr;
} // ~GPUGrass

bool GPUGrass::Init(const InitParams& params) {
    if (!params.device || !params.hwnd || !params.grass) {
        return false;
    }

    m_grassSRV = params.grass;
    m_linearSampler = params.linearSampler;

    if (!InitShader(params.device, params.hwnd)) {
        return false;
    }

    if (!InitBuffers(params.device)) {
        return false;
    }

    return true;
} // Init

void GPUGrass::ComputePlacement(ID3D11DeviceContext* context, const RenderParams& params) {
    if (!context) {
        return;
    }

    if (!UpdateConstantBuffer(context, m_placementBuffer.Get(), params.placementData)) {
        return;
    }
    context->CSSetConstantBuffers(PLACE_BUFFER_SLOT, 1, m_placementBuffer.GetAddressOf());

    if (!UpdateConstantBuffer(context, m_worldBuffer.Get(), XMMatrixTranspose(params.world))) {
        return;
    }
    context->CSSetConstantBuffers(WORLD_BUFFER_SLOT, 1, m_worldBuffer.GetAddressOf());

    // CS 바인딩
    context->CSSetShader(m_placementComputeShader.Get(), nullptr, 0);

    ID3D11ShaderResourceView* srvs[] = { params.heightMapSRV, params.normalMapSRV };
    context->CSSetShaderResources(TEX_HEIGHT_SLOT, 1, &params.heightMapSRV);
    context->CSSetShaderResources(TEX_NORMAL_SLOT, 1, &params.normalMapSRV);

    context->CSSetSamplers(GRASS_SAMPLER_SLOT, 1, &m_linearSampler);

    // UAV 바인딩
    // initialCount를 0으로 설정하여 버퍼를 매 프레임 비워줌
    ID3D11UnorderedAccessView* uavs[] = { m_grassInstanceUAV.Get(), m_grassFarInstanceUAV.Get(), m_treeInstanceUAV.Get() };
    UINT initialCounts[] = { 0, 0, 0 };
    context->CSSetUnorderedAccessViews(0, 3, uavs, initialCounts);

    // 지형 크기에 맞게 Dispatch
    UINT groupX = static_cast<UINT>(params.placementData.terrainWidth) / 8;
    UINT groupZ = static_cast<UINT>(params.placementData.terrainDepth) / 8;
    if (groupX == 0) groupX = 1;
    if (groupZ == 0) groupZ = 1;

    context->Dispatch(groupX, 1, groupZ);

    ID3D11ShaderResourceView* nullSRV = nullptr;
	context->CSSetShaderResources(TEX_HEIGHT_SLOT, 1, &nullSRV);
	context->CSSetShaderResources(TEX_NORMAL_SLOT, 1, &nullSRV);

    ID3D11UnorderedAccessView* nullUAVs[] = { nullptr, nullptr, nullptr };
    context->CSSetUnorderedAccessViews(0, 3, nullUAVs, nullptr);

    context->CopyStructureCount(m_grassArgsBuffer.Get(), 4, m_grassInstanceUAV.Get());
    context->CopyStructureCount(m_grassFarArgsBuffer.Get(), 4, m_grassFarInstanceUAV.Get());
    context->CopyStructureCount(m_treeArgsBuffer.Get(), 4, m_treeInstanceUAV.Get());
} // ComputePlacement

void GPUGrass::RenderNear(ID3D11DeviceContext* context) {
    if (!context) {
        return;
    }

    if (UpdateGrassBuffer(context)) {
        context->GSSetConstantBuffers(GRASS_BUFFER_SLOT, 1, m_grassBuffer.GetAddressOf());
        context->PSSetConstantBuffers(GRASS_BUFFER_SLOT, 1, m_grassBuffer.GetAddressOf());
    }

    context->IASetInputLayout(nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    context->VSSetShader(m_nearVertexShader.Get(), nullptr, 0);
    context->GSSetShader(m_nearGeometryShader.Get(), nullptr, 0);
    context->PSSetShader(m_nearPixelShader.Get(), nullptr, 0);

    context->VSSetShaderResources(1, 1, m_grassInstanceSRV.GetAddressOf());
    context->PSSetShaderResources(GRASS_TEXTURE_SLOT, 1, &m_grassSRV);
    context->PSSetSamplers(GRASS_SAMPLER_SLOT, 1, &m_linearSampler);

    context->DrawInstancedIndirect(m_grassArgsBuffer.Get(), 0);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->VSSetShaderResources(1, 1, &nullSRV);
    context->GSSetShader(nullptr, nullptr, 0);
} // RenderNear

void GPUGrass::RenderFar(ID3D11DeviceContext* context) {
    if (!context) {
        return;
    }

    if (UpdateGrassBuffer(context)) {
        context->VSSetConstantBuffers(GRASS_BUFFER_SLOT, 1, m_grassBuffer.GetAddressOf());
        context->PSSetConstantBuffers(GRASS_BUFFER_SLOT, 1, m_grassBuffer.GetAddressOf());
    }

    context->IASetInputLayout(nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    context->VSSetShader(m_farVertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_farPixelShader.Get(), nullptr, 0);

    context->VSSetShaderResources(1, 1, m_grassFarInstanceSRV.GetAddressOf());
    context->PSSetShaderResources(GRASS_TEXTURE_SLOT, 1, &m_grassSRV);
    context->PSSetSamplers(GRASS_SAMPLER_SLOT, 1, &m_linearSampler);

    context->DrawInstancedIndirect(m_grassFarArgsBuffer.Get(), 0);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->VSSetShaderResources(1, 1, &nullSRV);
    context->PSSetShaderResources(GRASS_TEXTURE_SLOT, 1, &nullSRV);
} // RenderFar

void GPUGrass::OnGui() {
    ImGui::Text("WaterComposite Parameters");
    ImGui::Separator();

    ImGui::SliderFloat("Blade Width", &m_grassData.width, 0.1f, 10.0f);
    ImGui::SliderFloat("Blade Height", &m_grassData.height, 0.1f, 20.0f);
    ImGui::SliderFloat("Wind Strength", &m_grassData.windStrength, 0.0f, 2.0f);
    ImGui::SliderFloat("Wind Speed", &m_grassData.windSpeed, 0.0f, 5.0f);

    if (ImGui::Button("Reset")) {
        m_grassData = GrassBuffer();
    }

    ImGui::Spacing();
} // OnGui

ID3D11Buffer* GPUGrass::GetTreeArgsBuffer() const {
    return m_treeArgsBuffer.Get();
} // GetTreeArgsBuffer

ID3D11ShaderResourceView* GPUGrass::GetTreeInstanceSRV() const {
    return m_treeInstanceSRV.Get();
} // GetTreeInstanceSRV

ID3D11UnorderedAccessView* GPUGrass::GetTreeInstanceUAV() const {
    return m_treeInstanceUAV.Get();
} // GetTreeInstanceUAV

bool GPUGrass::InitShader(ID3D11Device* device, HWND hwnd) {
    if (!InitComputingShader(device, hwnd, PathConstants::GRASS_PLACEMENT_CS, m_placementComputeShader.GetAddressOf())) {
        return false;
    }

    {
        Microsoft::WRL::ComPtr<ID3DBlob> blob;
        if (FAILED(D3DCompileFromFile(PathConstants::PLACE_GRASS_VS.c_str(),
            nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "main", "vs_5_0", 0, 0, blob.GetAddressOf(), nullptr))) {
            DebugPrint("GPUGrass: NearVS 컴파일 실패");
            return false;
        }
        device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(),
            nullptr, m_nearVertexShader.GetAddressOf());
    }

    if (!InitGeometryShader(device, hwnd, PathConstants::GRASS_GS, m_nearGeometryShader.GetAddressOf())) {
        return false;
    }

    if (!InitPixelShader(device, hwnd, PathConstants::GRASS_PS, m_nearPixelShader.GetAddressOf())) {
        return false;
    }

    {
        Microsoft::WRL::ComPtr<ID3DBlob> blob;
        if (FAILED(D3DCompileFromFile(PathConstants::PLACE_GRASS_FAR_VS.c_str(),
            nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "main", "vs_5_0", 0, 0, blob.GetAddressOf(), nullptr))) {
            DebugPrint("GPUGrass: FarVS 컴파일 실패");
            return false;
        }
        device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(),
            nullptr, m_farVertexShader.GetAddressOf());
    }

    if (!InitPixelShader(device, hwnd, PathConstants::GRASS_FAR_PS, m_farPixelShader.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<ConstantBuffer::PlacementBuffer>(device, m_placementBuffer.GetAddressOf())) {
        return false;
    }
    if (!InitConstantBuffer<GrassBuffer>(device, m_grassBuffer.GetAddressOf())) {
        return false;
    }

    if (!InitConstantBuffer<ConstantBuffer::WorldBuffer>(device, m_worldBuffer.GetAddressOf())) {
        return false;
    }

    return true;
} // InitShaders

bool GPUGrass::InitBuffers(ID3D11Device* device) {
    using namespace ConstantBuffer;

    // Append Structured Buffer 생성
    D3D11_BUFFER_DESC instanceDesc = {};
    instanceDesc.Usage = D3D11_USAGE_DEFAULT;
    instanceDesc.ByteWidth = sizeof(InstanceBuffer) * MAX_GRASS_INSTANCES;
    instanceDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    instanceDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    instanceDesc.StructureByteStride = sizeof(InstanceBuffer);

    // 근경 버퍼
    if (FAILED(device->CreateBuffer(&instanceDesc, nullptr, m_grassInstanceBuffer.GetAddressOf()))) {
        return false;
    }

    // 원경 버퍼
    if (FAILED(device->CreateBuffer(&instanceDesc, nullptr, m_grassFarInstanceBuffer.GetAddressOf()))) {
        return false;
    }

    if (FAILED(device->CreateBuffer(&instanceDesc, nullptr, m_treeInstanceBuffer.GetAddressOf()))) {
        return false;
    }

    // UAV 생성
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
    uavDesc.Buffer.NumElements = MAX_GRASS_INSTANCES;

    device->CreateUnorderedAccessView(m_grassInstanceBuffer.Get(), &uavDesc, m_grassInstanceUAV.GetAddressOf());
    device->CreateUnorderedAccessView(m_grassFarInstanceBuffer.Get(), &uavDesc, m_grassFarInstanceUAV.GetAddressOf());
    device->CreateUnorderedAccessView(m_treeInstanceBuffer.Get(), &uavDesc, m_treeInstanceUAV.GetAddressOf());

    // SRV 생성
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.NumElements = MAX_GRASS_INSTANCES;

    device->CreateShaderResourceView(m_grassInstanceBuffer.Get(), &srvDesc, m_grassInstanceSRV.GetAddressOf());
    device->CreateShaderResourceView(m_grassFarInstanceBuffer.Get(), &srvDesc, m_grassFarInstanceSRV.GetAddressOf());
    device->CreateShaderResourceView(m_treeInstanceBuffer.Get(), &srvDesc, m_treeInstanceSRV.GetAddressOf());

    // Indirect Args Buffer 생성
    // DrawInstancedIndirect용 파라미터 보관
    D3D11_BUFFER_DESC argsDesc = {};
    argsDesc.Usage = D3D11_USAGE_DEFAULT;
    argsDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    argsDesc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS; // 핵심 플래그

    argsDesc.ByteWidth = sizeof(DrawInstancedIndirectArgs);
    DrawInstancedIndirectArgs nearArgsInit = { QUAD_VERTEX_COUNT, 0, 0, 0 };
    D3D11_SUBRESOURCE_DATA nearData = {};
    nearData.pSysMem = &nearArgsInit;
    if (FAILED(device->CreateBuffer(&argsDesc, &nearData, m_grassArgsBuffer.GetAddressOf()))) {
        return false;
    }

    DrawInstancedIndirectArgs farArgsInit = { QUAD_VERTEX_COUNT, 0, 0, 0 };
    D3D11_SUBRESOURCE_DATA farData = {};
    farData.pSysMem = &farArgsInit;
    if (FAILED(device->CreateBuffer(&argsDesc, &farData, m_grassFarArgsBuffer.GetAddressOf()))) {
        return false;
    }

    if (!ShaderHelper::InitConstantBuffer<PlacementBuffer>(device, m_placementBuffer.GetAddressOf())) {
        return false;
    }
    return true;
} // InitBuffers

bool GPUGrass::UpdateGrassBuffer(ID3D11DeviceContext* context) {
    if (memcmp(&m_prevGrassData, &m_grassData, sizeof(GrassBuffer)) == 0) {
        return true;
    }
    if (!ShaderHelper::UpdateConstantBuffer(context, m_grassBuffer.Get(), m_grassData)) {
        return false;
    }

    m_prevGrassData = m_grassData;
    return true;
} // UpdateGrassBuffer