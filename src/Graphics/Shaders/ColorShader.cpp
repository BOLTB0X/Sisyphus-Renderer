#include "Pch.h"
#include "ColorShader.h"
#include "Resources/ConstantBufferType.h"
// Utils
#include "Utils/SharedConstants.h"

using namespace SharedConstants;
using namespace DirectX;

bool ColorShader::Init(ID3D11Device* device, HWND hwnd, 
                       const std::wstring& vsPath, const std::wstring& psPath) {
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    if (!Compile(device, hwnd, vsPath, "main", "vs_5_0", vsBlob.GetAddressOf()))
        return false;
    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);

    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
    if (!Compile(device, hwnd, psPath, "main", "ps_5_0", psBlob.GetAddressOf()))
        return false;
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    device->CreateInputLayout(layoutDesc, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_layout);

    return InitBuffer(device);
} // Init

bool ColorShader::InitBuffer(ID3D11Device* device) {
    D3D11_BUFFER_DESC matrixBufferDesc = {};
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(ConstantBuffer::MatrixBuffer);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    return SUCCEEDED(device->CreateBuffer(&matrixBufferDesc, nullptr, m_matrixBuffer.GetAddressOf()));
} // InitBuffer

bool ColorShader::Render(ID3D11DeviceContext* context, int indexCount,
                         XMMATRIX world, XMMATRIX view, XMMATRIX projection) {
    if (!UpdateMatrixBuffer(context, world, view, projection))
        return false;

    context->VSSetConstantBuffers(0, 1, m_matrixBuffer.GetAddressOf());
    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    
    context->DrawIndexed(indexCount, 0, 0);
    return true;
} // Render

bool ColorShader::UpdateMatrixBuffer(ID3D11DeviceContext* context, XMMATRIX world, XMMATRIX view, XMMATRIX projection) {
    world = XMMatrixTranspose(world);
    view = XMMatrixTranspose(view);
    projection = XMMatrixTranspose(projection);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if (SUCCEEDED(context->Map(m_matrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
        ConstantBuffer::MatrixBuffer* dataPtr = (ConstantBuffer::MatrixBuffer*)mappedResource.pData;
        dataPtr->world = world;
        dataPtr->view = view;
        dataPtr->projection = projection;
        context->Unmap(m_matrixBuffer.Get(), 0);

        return true;
    }
    return false;
} // UpdateMatrixBuffer