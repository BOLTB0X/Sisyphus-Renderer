#include "Pch.h"
#include "System.h"
#include "Window.h"
#include "Input.h"
// Core
#include "Renderer.h"
#include "Fps.h"
#include "Timer.h"
#include "Cpu.h"
// Utils
#include "ImGui/ImGuiManager.h"
#include "ImGui/PerformanceWidget.h"
// imgui
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

// Imgui
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win API
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, umessage, wparam, lparam))
        return true;

    switch (umessage) {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }

    case WM_CLOSE:
    {
        PostQuitMessage(0);
        return 0;
    }

    default:
    {
        return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
    }
    }
} // WndProc

LRESULT CALLBACK System::MessageHandler(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
    return DefWindowProc(hwnd, umessage, wparam, lparam);
} // MessageHandler

System::System() {
    m_Window = std::make_unique<Window>();
    m_Input = std::make_shared<Input>();
    m_Fps = std::make_unique<Fps>();
    m_Cpu = std::make_unique<Cpu>();
    m_Timer = std::make_unique<Timer>();
    m_Renderer = std::make_unique<Renderer>();
    m_ImGuiManager = std::make_shared<ImGuiManager>();
    ApplicationHandle = this;
} // System

System::~System() {
} // ~System

bool System::Init() {
    if (!m_Window->Init(WndProc, L"Sisyphus Engine"))
        return false;

    if (!m_Input->Init(m_Window->GetHinstance(), m_Window->GetHwnd()))
        return false;
    
    m_Fps->Init();
    m_Cpu->Init();
    if (!m_Timer->Init())
        return false;

    if (!m_Renderer->Init(m_Window->GetHwnd(), m_ImGuiManager))
        return false;
    
    InitWidgets();
    return true;
} // Init

void System::Shutdown()
{
    if (m_Renderer) {
        m_Renderer->Shutdown();
    }
    if (m_Timer) {
        m_Timer.reset();
    }
    if (m_Cpu) {
        m_Cpu->Shutdown();
    }
    if (m_Fps) {
        m_Fps.reset();
    }
    if (m_Input) {
        m_Input.reset();
    }
    if (m_Window) {
        m_Window.reset();
    }
    spdlog::info("System Fully Shutdown.");
} // Shutdown

void System::Run() {
    MSG msg = {};
    while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            if (Frame() == false)
                break;
        }
    }
} // Run

bool System::Frame() {
    if (!m_Input->Frame())
        return false;

    if (m_Input->IsEscapePressed())
        return false;

    m_Fps->Frame();
    m_Cpu->Frame();
    m_Timer->Frame();

    if (!m_Renderer->Frame())
        return false;

    return true;
} // Frame


void System::InitWidgets() {
    auto perfWidget = std::make_unique<PerformanceWidget>(
        m_Fps->GetFps(),
        m_Cpu->GetCpuPercentage(),
        m_Timer->GetTotalTime()
    );

    m_ImGuiManager->AddWidget(std::move(perfWidget));
} // InitWidgets