#include "Pch.h"
#include "D3D11State.h"
#include "Helpers/DebugHelper.h"

D3D11State::D3D11State() {    
} // D3D11State

D3D11State::D3D11State(const D3D11State& other){
} // D3D11State

D3D11State::~D3D11State() {
} // ~D3D11State

bool D3D11State::Init(ID3D11Device* device) {
    if (!InitRasterizer(device))
        return false;
    if (!InitWireframe(device))
        return false;
    if (!InitDepthStencil(device))
        return false;
    if (!InitSampler(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, 
                     D3D11_TEXTURE_ADDRESS_WRAP, m_linearSamplerState.GetAddressOf()))
        return false;
    if (!InitBlendState(device)) return false;
    return true;
} // Init

ID3D11RasterizerState*   D3D11State::GetRasterizerState() const { return m_rasterizerState.Get(); }
ID3D11DepthStencilState* D3D11State::GetDepthStencilState() const { return m_depthStencilState.Get(); }
ID3D11SamplerState*      D3D11State::GetLinearSamplerState() const { return m_linearSamplerState.Get(); }
ID3D11BlendState*        D3D11State::GetBlendState() const { return m_blendState.Get(); }

bool D3D11State::InitRasterizer(ID3D11Device* device) {
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.AntialiasedLineEnable = false;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.DepthBias = 0;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.MultisampleEnable = false;
    rasterDesc.ScissorEnable = false;
    rasterDesc.SlopeScaledDepthBias = 0.0f;

    return SUCCEEDED(device->CreateRasterizerState(&rasterDesc, &m_rasterizerState));
} // InitRasterizer

bool D3D11State::InitWireframe(ID3D11Device* device) {
    D3D11_RASTERIZER_DESC wireDesc = {};
    wireDesc.CullMode = D3D11_CULL_BACK;
    wireDesc.FillMode = D3D11_FILL_WIREFRAME;
    wireDesc.DepthClipEnable = true;

    return SUCCEEDED(device->CreateRasterizerState(&wireDesc, &m_wireframeState));
} // InitWireframe

bool D3D11State::InitDepthStencil(ID3D11Device* device) {
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = false;

    return SUCCEEDED(device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState));
} // InitDepthStencil

bool D3D11State::InitSampler(ID3D11Device* device, D3D11_FILTER filter, 
                             D3D11_TEXTURE_ADDRESS_MODE addressMode, 
                             ID3D11SamplerState** targetSampler) {
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = filter;
    samplerDesc.AddressU = addressMode;
    samplerDesc.AddressV = addressMode;
    samplerDesc.AddressW = addressMode;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    return SUCCEEDED(device->CreateSamplerState(&samplerDesc, targetSampler));
} // InitSampler

bool D3D11State::InitBlendState(ID3D11Device* device) {
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    return SUCCEEDED(device->CreateBlendState(&blendDesc, &m_blendState));
} // InitBlendState