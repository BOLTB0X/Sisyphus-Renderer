#include "Pch.h"
#include "ImGuiState.h"
#include "Helpers/DebugHelper.h"
// imgui
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

ImGuiState::ImGuiState()
: m_isInitialized(false) {
} // ImGui

ImGuiState::~ImGuiState() {
} // ~ImGui

bool ImGuiState::Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext) {
    if (m_isInitialized)
        return true;

    IMGUI_CHECKVERSION();
    if (ImGui::CreateContext() == false) {
        return false;
    }

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    if (!ImGui_ImplWin32_Init(hwnd)) {
        DebugHelper::DebugPrint("ImGui Win32 백엔드 초기화 실패");
        return false;
    }

    if (!ImGui_ImplDX11_Init(device, deviceContext)) {
        DebugHelper::DebugPrint("ImGui DX11 백엔드 초기화 실패");
        return false;
    }

    ImGui::StyleColorsDark();

    m_isInitialized = true;
    return true;
} // Init

void ImGuiState::Shutdown() {
    if (m_isInitialized == false) {
        return;
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    m_isInitialized = false;
} // Shutdown

void ImGuiState::Begin() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
} // Begin

void ImGuiState::End() {
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
} // End