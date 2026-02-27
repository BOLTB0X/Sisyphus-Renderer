// System.h
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <memory>
#include <windows.h>

class Input;
class Renderer;
class Window;
class ImGuiManager;
class Fps;
class Cpu;
class Timer;

class System {
public:
    System();
    System(const System&) = delete;
    ~System();

    bool Init();
    void Shutdown();
    void Run();
    bool Frame();
    LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
    void InitWidgets();

private:
    std::unique_ptr<Window>       m_Window;
    std::unique_ptr<Fps>          m_Fps;
    std::unique_ptr<Cpu>          m_Cpu;
    std::unique_ptr<Timer>        m_Timer;
    std::unique_ptr<Renderer>     m_Renderer;
    
    std::shared_ptr<Input>        m_Input;
    std::shared_ptr<ImGuiManager> m_ImGuiManager;
}; // System

static System* ApplicationHandle = nullptr;