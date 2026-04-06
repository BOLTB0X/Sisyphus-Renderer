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
class Stone;
class D3D11State;
class SkyBox;
class DirectionalLight;
class Ground;
class ShadowShader;
class DepthRecorder;
class RenderTexture;

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
    void MainPass(ID3D11DeviceContext*, D3D11State*);
    void DepthPass(ID3D11DeviceContext*, D3D11State*);
	void DrawStone(ID3D11DeviceContext*, D3D11State*);
    void DrawSkyBox(ID3D11DeviceContext*, D3D11State*);
    void DrawGround(ID3D11DeviceContext*, D3D11State*);

    //void DebugVolume(ID3D11DeviceContext*);
    void InitWidgets();

private:
    static RendererState              m_RendererState;
    std::unique_ptr<D3D11Manager>     m_D3D11Mgr;
    std::unique_ptr<Stone>            m_Stone;
    std::unique_ptr<Camera>           m_Camera;
	std::unique_ptr<SkyBox>           m_SkyBox;
    std::unique_ptr<Ground>           m_Ground;
    std::unique_ptr<DirectionalLight> m_DirectionalLight;
    std::unique_ptr<DepthRecorder>    m_DepthRecorder;
    std::shared_ptr<TextureManager>   m_TextureMgr;
    std::shared_ptr<ImGuiManager>     m_ImGuiMgr;
    std::unique_ptr<RenderTexture>    m_shadowMapTexture;

    ID3D11RenderTargetView*           m_nullRTV;
    ID3D11ShaderResourceView*         m_nullSRV;
    float                             m_renderingTime;
    float                             m_blendFactor[4];
    // 디버깅
    //std::unique_ptr<DebugHelper::VolumeSlicer> m_VolumeSlicer;
}; // Renderer