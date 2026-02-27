// Input.h
#pragma once
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>


class Input {
public:
	Input();
	Input(const Input&) = delete;
	~Input();

	bool Init(HINSTANCE, HWND);
	void Shutdown();
	bool Frame();

public: // 마우스
	void GetMouseLocation(int&, int&);
	void GetMouseDelta(int&, int&);
	int  GetMouseWheelDelta();

	bool IsMouseLPressed();
	bool IsLeftMouseDown();
	bool IsRightMouseDown();

public: // 키보드
	bool IsEscapePressed();
	bool IsLeftPressed();
	bool IsRightPressed();
	bool IsUpPressed();
	bool IsDownPressed();
	bool IsZPressed();
	bool IsXPressed();
	bool IsF1Toggled();
    
	bool IsPgUpPressed();
	bool IsPgDownPressed();

private:
	bool ReadKeyboard();
	bool ReadMouse();
	void ProcessInput();

private:
	IDirectInput8*       m_directInput;
	IDirectInputDevice8* m_keyboard;
	IDirectInputDevice8* m_mouse;

	unsigned char m_keyboardState[256];
	DIMOUSESTATE  m_mouseState;

	int  m_mouseX, m_mouseY;
	bool m_F1_released;
	bool m_prevMouseL;
}; // Input