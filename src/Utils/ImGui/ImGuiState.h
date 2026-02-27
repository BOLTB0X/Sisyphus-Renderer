#pragma once
#include <d3d11.h>

class ImGuiState {
public:
    ImGuiState();
    ImGuiState(const ImGuiState&) = delete;
    ~ImGuiState();

    bool Init(HWND, ID3D11Device*, ID3D11DeviceContext*);
    void Shutdown();

    void Begin();
    void End();

private:
    bool m_isInitialized;
}; // ImGuiState