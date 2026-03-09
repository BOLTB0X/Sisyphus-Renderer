// Renderer.h
#pragma once
#include <windows.h>
#include <d3d11.h>
// STL
#include <memory>

class RendererState;
class D3D11Manager;
class ImGuiManager;
class TextureManager;
class Triangle;
class Camera;
class Stone;
class D3D11State;
class VolumeTexture;
class NoiseGenerator;
class Atmosphere;

class Renderer {
public:
    Renderer();
    Renderer(const Renderer&);
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
    void InitWidgets();
    void GenerateCloudNoise(ID3D11DeviceContext*);
    
    void DrawTriangle(ID3D11DeviceContext*, D3D11State*);
	void DrawStone(ID3D11DeviceContext*, D3D11State*);

private:
    static RendererState            m_RendererState;
    std::unique_ptr<D3D11Manager>   m_D3D11Mgr;
    std::unique_ptr<Triangle>       m_Triangle;
    std::unique_ptr<Stone>          m_Stone;
    std::unique_ptr<Camera>         m_Camera;
    std::unique_ptr<VolumeTexture>  m_VolumeTexture;
    std::unique_ptr<NoiseGenerator> m_NoiseGenerator;
	std::unique_ptr<Atmosphere>     m_Atmosphere;

    std::shared_ptr<TextureManager> m_TextureMgr;
    std::shared_ptr<ImGuiManager>   m_ImGuiMgr;
}; // Renderer