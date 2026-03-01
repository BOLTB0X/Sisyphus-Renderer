#include "Pch.h"
#include "StoneShader.h"
#include "Resources/ConstantBufferType.h"

using namespace DirectX;

StoneShader::StoneShader(): Shader() {
    m_type = ShaderType::Stone;
    m_sampleState = nullptr;
} // StoneShader

StoneShader::~StoneShader() {
    m_sampleState = nullptr;
} // ~StoneShader

bool StoneShader::Init(ID3D11Device* device, HWND hwnd, const std::wstring& vsPath, const std::wstring& psPath) {
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;

    if (!Compile(device, hwnd, vsPath, "main", "vs_5_0", vsBlob.GetAddressOf()))
        return false;
    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);

    if (!Compile(device, hwnd, psPath, "main", "ps_5_0", psBlob.GetAddressOf()))
        return false;
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    device->CreateInputLayout(layoutDesc, 5, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_layout);
    return InitBuffer(device);
} // Init

bool StoneShader::InitBuffer(ID3D11Device* device) {
    D3D11_BUFFER_DESC cameraBufferDesc = {};
    cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    cameraBufferDesc.ByteWidth = sizeof(ConstantBuffer::CameraBuffer);
    cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(device->CreateBuffer(&cameraBufferDesc, nullptr, m_cameraBuffer.GetAddressOf()))) {
        return false;
    }

    D3D11_BUFFER_DESC lightBufferDesc = {};
    lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    lightBufferDesc.ByteWidth = sizeof(ConstantBuffer::LightBuffer);
    lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    return SUCCEEDED(device->CreateBuffer(&lightBufferDesc, nullptr, m_lightBuffer.GetAddressOf()));
} // InitBuffer

bool StoneShader::Render(ID3D11DeviceContext* context,const RenderParams& params) {
    if (!UpdateCameraBuffer(context, params.world, params.view, params.projection, params.camPos))
        return false;
    if (!UpdateLightBuffer(context, params.diffuse, params.lightDir))
        return false;
    
    context->VSSetConstantBuffers(0, 1, m_cameraBuffer.GetAddressOf());
    context->PSSetConstantBuffers(1, 1, m_lightBuffer.GetAddressOf());
    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    
    if (m_sampleState) {
        context->PSSetSamplers(0, 1, &m_sampleState);
    }
    return true;
} // Render

void  StoneShader::SetSampler(ID3D11SamplerState* sampler) { m_sampleState = sampler; }

bool StoneShader::UpdateCameraBuffer(ID3D11DeviceContext* context, XMMATRIX world, XMMATRIX view, XMMATRIX projection, XMFLOAT3 camPos) {
    world = XMMatrixTranspose(world);
    view = XMMatrixTranspose(view);
    projection = XMMatrixTranspose(projection);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if (SUCCEEDED(context->Map(m_cameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
        ConstantBuffer::CameraBuffer* dataPtr = (ConstantBuffer::CameraBuffer*)mappedResource.pData;
        dataPtr->world = world;
        dataPtr->view = view;
        dataPtr->projection = projection;
        dataPtr->cameraPosition = camPos;
        dataPtr->padding = 0.0f;
        context->Unmap(m_cameraBuffer.Get(), 0);

        return true;
    }
    return false;
} // UpdateCameraBuffer

bool StoneShader::UpdateLightBuffer(ID3D11DeviceContext* context, XMFLOAT4 diffuse, XMFLOAT3 dir) {
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if (SUCCEEDED(context->Map(m_lightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
        auto* dataPtr = (ConstantBuffer::LightBuffer*)mappedResource.pData;
        dataPtr->diffuseColor = diffuse;
        dataPtr->lightDirection = dir;
        dataPtr->padding = 0.0f;
        context->Unmap(m_lightBuffer.Get(), 0);
        return true;
    }
    return false;
} // UpdateLightBuffer