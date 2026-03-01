// Renderer.h
#pragma once
#include <windows.h>
// STL
#include <memory>

class RendererState;
class D3D11Manager;
class ImGuiManager;
class Triangle;
class Camera;
class Stone;

class Renderer {
public:
    Renderer();
    Renderer(const Renderer&);
    ~Renderer();

    bool Init(HWND, std::shared_ptr<ImGuiManager>);
    void Shutdown();
    bool Frame();
    
private:
    bool Render();

private:
    static RendererState          m_RendererState;
    std::unique_ptr<D3D11Manager> m_D3D11Manager;
    std::unique_ptr<Triangle>     m_Triangle;
    std::unique_ptr<Stone>        m_Stone;
    std::shared_ptr<ImGuiManager> m_ImGuiManager;
    std::unique_ptr<Camera>       m_Camera;
}; // Renderer