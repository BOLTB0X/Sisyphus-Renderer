// Renderer.h
#pragma once
#include <windows.h>
#include <d3d11.h>
#include "RenderQueue.h"
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
class SceneRTManager;
class CloudMap;
class AtmosphereMap;
class CloudComposite;
class TAA;
class PostEffects;
class QuadTreeGrass;
class TransparentActor;
class MayaActor;
class SkinnedActor;
class RigidActor;
class WaterComposite;
class Terrain;
class GPUGrass;
class InstancingActor;
class FogComposite;

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
    bool Render(float);
	void UpdateModelTransform();
    void MainPass(ID3D11DeviceContext*, D3D11State*);
    void ShadowPass(ID3D11DeviceContext*, D3D11State*);
    void CouldPass(ID3D11DeviceContext*, D3D11State*);
    void WaterPass(ID3D11DeviceContext*, D3D11State*);
    void FogPass(ID3D11DeviceContext*, D3D11State*);
    void PostProcessingPass(ID3D11DeviceContext*, D3D11State*);

    void UpdateCommonShaderBuffer(ID3D11DeviceContext*, D3D11State*);
    void UpdatePlacement(ID3D11DeviceContext*);
    void DrawTerrain(ID3D11DeviceContext*, D3D11State*);
    void DrawGround(ID3D11DeviceContext*, D3D11State*);
	void DrawModel(ID3D11DeviceContext*, D3D11State*);
    void DrawSkyBox(ID3D11DeviceContext*, D3D11State*, bool isReflection = false);
	void DrawQuadtreeGrass(ID3D11DeviceContext*, D3D11State*);
	void DrawGPUGrass(ID3D11DeviceContext*, D3D11State*);
	void ComputeShaderData(ID3D11DeviceContext*, D3D11State*);

    void ApplyEffects(ID3D11DeviceContext*, D3D11State*);
    void ApplyTAA(ID3D11DeviceContext*, D3D11State*);

    void InitCommonBuffer(ID3D11Device*);
    void InitDefaultMaya(HWND, ID3D11Device*, ID3D11DeviceContext*, ID3D11SamplerState*);
    void InitTransparentMaya(HWND, ID3D11Device*, ID3D11DeviceContext*, ID3D11SamplerState*);
    void InitWidgets();

private:
    static RendererState                 m_RendererState;
    RenderQueue                          m_OpaqueQueue;
    RenderQueue                          m_TransparentQueue;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_frameBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_lightBuffer;

    std::unique_ptr<D3D11Manager>        m_D3D11Mgr;
    std::unique_ptr<Camera>              m_Camera;
	std::unique_ptr<SkyBox>              m_SkyBox;
    std::unique_ptr<Ground>              m_Ground;
    std::unique_ptr<DirectionalLight>    m_DirectionalLight;
    std::unique_ptr<VolumetricCloud>     m_VolumetricCloud;
    std::unique_ptr<ShadowMap>           m_ObjectShadowMap;
    std::unique_ptr<ShadowMap>           m_TerrainShadowMap;
    std::unique_ptr<CloudMap>            m_CloudMapLUT;
    std::unique_ptr<AtmosphereMap>       m_AtmosphereLUT;
    std::unique_ptr<CloudComposite>      m_Composite;
    std::unique_ptr<PostEffects>         m_Post;
    std::unique_ptr<TAA>                 m_TAA;
    std::unique_ptr<QuadTreeGrass>       m_QuadtreeGrass;
    std::unique_ptr<TransparentActor>    m_Tree;
	std::unique_ptr<MayaActor>           m_Stone;
	std::unique_ptr<MayaActor>           m_StonePillar;
    std::unique_ptr<MayaActor>           m_Arca;
	std::unique_ptr<SkinnedActor>        m_Rakshasa;
	std::unique_ptr<RigidActor>          m_LowpolyPlayer;
	std::unique_ptr<WaterComposite>      m_WaterComposite;
	std::unique_ptr<Terrain>             m_Terrain;
	std::unique_ptr<GPUGrass>            m_GPUGrass;
	std::unique_ptr<InstancingActor>     m_InstancingActor;
    std::unique_ptr<FogComposite>        m_FogComposite;

    std::shared_ptr<TextureManager>      m_TextureMgr;
    std::shared_ptr<ImGuiManager>        m_ImGuiMgr;
    std::unique_ptr<SceneRTManager>      m_sceneRTMgr;

    ID3D11RenderTargetView*              m_nullRTV;
    ID3D11ShaderResourceView*            m_nullSRV;
    float                                m_renderingTime;
}; // Renderer