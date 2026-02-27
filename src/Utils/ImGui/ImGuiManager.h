#pragma once
#include "ImGuiWidget.h"
// window
#include <windows.h>
// DirectX 11
#include <d3d11.h>
// STL
#include <memory>
#include <vector>
#include <string>
#include <DirectXMath.h>

class ImGuiState;

class ImGuiManager {
public:
    ImGuiManager();
    ImGuiManager(const ImGuiManager& other) = delete;
    ~ImGuiManager();

    bool Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
    void Shutdown();
    void Frame();

    void Begin();
    void End();
    bool CanControlWorld() const;
    void AddWidget(std::unique_ptr<ImGuiWidget>);
    void ToggleWidget();

    bool GetCameraLocked() const;
    void SetCameraLocked(bool);
    bool SetWorldClicked(bool) const;

public:
    template<typename T>
    T* GetWidget(const std::string& name)
    {
        for (auto& widget : m_widgets) {
            if (widget->GetName() == name)
                return dynamic_cast<T*>(widget.get());
        }
        return nullptr;
    } // GetWidget

private:
    std::unique_ptr<ImGuiState>               m_ImGuiState;
    std::vector<std::unique_ptr<ImGuiWidget>> m_widgets;
    bool                                      m_isCameraLocked;
}; // ImGuiManager