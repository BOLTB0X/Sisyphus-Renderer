#include "Pch.h"
#include "ImGuiManager.h"
#include "ImGuiWidget.h"
#include "ImGuiState.h"
#include "ImGuiDrawer.h"
// imgui
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

ImGuiManager::ImGuiManager()
    : m_isCameraLocked(false) {
    m_ImGuiState = std::make_unique<ImGuiState>();
} // ImGuiManager

ImGuiManager::~ImGuiManager()  {
} // ~ImGuiManager

bool ImGuiManager::Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
    return m_ImGuiState->Init(hwnd, device, deviceContext);
} // Init

void ImGuiManager::Shutdown()
{
    m_widgets.clear();

    if (m_ImGuiState) {
        m_ImGuiState->Shutdown();
    }
} // Shutdown

void ImGuiManager::Frame() {
    m_ImGuiState->Begin();

    for (auto& widget : m_widgets)
    {
        if (widget->IsVisible())
        {
            widget->Frame();
        }
    }

    m_ImGuiState->End();
    return;
} // Render

void ImGuiManager::Begin()
{
    if (m_ImGuiState)
    {
        m_ImGuiState->Begin();
    }
} // Begin

void ImGuiManager::End() {
    if (m_ImGuiState) {
        m_ImGuiState->End();
    }
} // End

bool ImGuiManager::CanControlWorld() const {
    if (m_ImGuiState == nullptr) {
        return false;
    }
    ImGuiIO& io = ImGui::GetIO();
    return !(io.WantCaptureMouse || io.WantCaptureKeyboard);
} // CanControlWorld

void ImGuiManager::AddWidget(std::unique_ptr<ImGuiWidget> widget) {
    m_widgets.push_back(std::move(widget));
    return;
} // AddWidget

void ImGuiManager::ToggleWidget() {
    for (auto& widget : m_widgets) {
        widget->SetVisible(!widget->IsVisible());
    }
} // ToggleWidget

bool ImGuiManager::GetCameraLocked() const { return m_isCameraLocked;}
// GetCameraLocked
void ImGuiManager::SetCameraLocked(bool lock) { m_isCameraLocked = lock; }
// SetCameraLocked

bool ImGuiManager::SetWorldClicked(bool mousePressed) const {
    if (m_ImGuiState == nullptr) {
        return false;
    }
    return mousePressed && !ImGui::GetIO().WantCaptureMouse;
} // SetWorldClicked