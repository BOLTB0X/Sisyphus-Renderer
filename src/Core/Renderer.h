// Renderer.h
#pragma once
#include <windows.h>
#include <d3d11.h>
// STL
#include <memory>
// Debug
#include "Helpers/DebugHelper.h"

class RendererState;
class D3D11Manager;
class ImGuiManager;
class TextureManager;
class Camera;
class D3D11State;
class SkyBox;
class DirectionalLight;
class Ground;
class VolumetricCloud;
class ShadowMap;
class RenderTexture;
class CloudMap;
class AtmosphereMap;
class CloudComposite;
class TAA;
class PostEffects;
class Grass;
class Tree;
class DefaultMaya;

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Init(HWND, std::shared_ptr<ImGuiManager>);
    void Shutdown();
    bool Frame(float);

    void UpdateCameraRotation(float, float);
    void UpdateCameraZoom(float);
    void UpdateCameraForwardBack(float);
	void UpdateCameraLeftRight(float);
	void UpdateCameraUpDown(float);

private:
    bool Render();
	void UpdateModelTransform();
    void MainPass(ID3D11DeviceContext*, D3D11State*);
    void ShadowPass(ID3D11DeviceContext*, D3D11State*);
    void PostProcessing(ID3D11DeviceContext*, D3D11State*);

    void UpdateCommonShaderBuffer(ID3D11DeviceContext*, D3D11State*);
    void DrawGround(ID3D11DeviceContext*, D3D11State*);
	void DrawModel(ID3D11DeviceContext*, D3D11State*);
    void DrawSkyBox(ID3D11DeviceContext*, D3D11State*);
	void DrawGrass(ID3D11DeviceContext*, D3D11State*);
	void ComputeShaderData(ID3D11DeviceContext*, D3D11State*);

    void ApplyComposite(ID3D11DeviceContext*, D3D11State*);
    void ApplyEffects(ID3D11DeviceContext*, D3D11State*);
    void ApplyTAA(ID3D11DeviceContext*, D3D11State*);

    void InitDefaultMaya(HWND, ID3D11Device*, ID3D11DeviceContext*, ID3D11SamplerState*);
    void InitWidgets();

private:
    static RendererState                  m_RendererState;

    Microsoft::WRL::ComPtr<ID3D11Buffer>  m_frameBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>  m_lightBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>  m_shadowBuffer;

    std::unique_ptr<D3D11Manager>         m_D3D11Mgr;
    std::unique_ptr<Camera>               m_Camera;
	std::unique_ptr<SkyBox>               m_SkyBox;
    std::unique_ptr<Ground>               m_Ground;
    std::unique_ptr<DirectionalLight>     m_DirectionalLight;
    std::unique_ptr<VolumetricCloud>      m_VolumetricCloud;
    std::unique_ptr<ShadowMap>            m_ObjectShadowMap;
    std::unique_ptr<ShadowMap>            m_TerrainShadowMap;
    std::unique_ptr<CloudMap>             m_CloudMapLUT;
    std::unique_ptr<AtmosphereMap>        m_AtmosphereLUT;
    std::unique_ptr<CloudComposite>       m_Composite;
    std::unique_ptr<PostEffects>          m_Post;
    std::unique_ptr<TAA>                  m_TAA;
    std::unique_ptr<Grass>                m_Grass;
    std::unique_ptr<Tree>                 m_Tree;
	std::unique_ptr<DefaultMaya>          m_Stone;
	std::unique_ptr<DefaultMaya>          m_StonePillar;
    std::unique_ptr<DefaultMaya>          m_Arca;

    std::shared_ptr<TextureManager>       m_TextureMgr;
    std::shared_ptr<ImGuiManager>         m_ImGuiMgr;
    std::unique_ptr<RenderTexture>        m_sceneRT;

    ID3D11RenderTargetView*               m_nullRTV;
    ID3D11ShaderResourceView*             m_nullSRV;
    float                                 m_renderingTime;
}; // Renderer