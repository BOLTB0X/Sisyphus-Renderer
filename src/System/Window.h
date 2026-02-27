#pragma once
#include <windows.h>
#include <string>

class Window {
public:
    Window();
    Window(const Window& other);
    ~Window();

    bool Init(WNDPROC wndProc, LPCWSTR appName);
    void Shutdown();
    
    HWND      GetHwnd()      const;
    HINSTANCE GetHinstance() const;

private:
    HWND      m_hwnd;
    HINSTANCE m_hinstance;
    LPCWSTR   m_engineName;
}; // Window