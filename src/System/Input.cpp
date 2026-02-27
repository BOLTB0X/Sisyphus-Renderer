#include "Pch.h"
#include "Input.h"
// Core
#include "RendererState.h"


Input::Input()
	: m_directInput(nullptr), m_keyboard(nullptr), m_mouse(nullptr),
	m_mouseX(0), m_mouseY(0),
	m_F1_released(0), m_prevMouseL(false) {
	memset(m_keyboardState, 0, sizeof(m_keyboardState));
	m_mouseState = {};
} // Input


Input::~Input() {} // ~Input


bool Input::Init(HINSTANCE hinstance, HWND hwnd) {
	HRESULT result;

	m_mouseX = 0;
	m_mouseY = 0;

	// DirectInput8 메인 객체 생성
	result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
	if (FAILED(result)) {
        spdlog::error("Input::Init->DirectInput8Create Failed.");
        return false;
    }

	// 키보드 디바이스 생성
	result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
	// 키보드 데이터 형식 설정
	result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
	// 키보드 협업 수준 설정
	result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	//  키보드 권한 획득
	result = m_keyboard->Acquire();
	// 마우스 디바이스 생성
	result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
	// 마우스 데이터 형식 설정
	result = m_mouse->SetDataFormat(&c_dfDIMouse);
	// 마우스 협업 수준 설정
	result = m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	// 마우스 권한 획득
	result = m_mouse->Acquire();
	return true;
} // Init


void Input::Shutdown() {
	if (m_mouse) {
		m_mouse->Unacquire();
		m_mouse->Release();
		m_mouse = 0;
	}

	if (m_keyboard) {
		m_keyboard->Unacquire();
		m_keyboard->Release();
		m_keyboard = 0;
	}

	if (m_directInput) {
		m_directInput->Release();
		m_directInput = 0;
	}

	return;
} // Shutdown


bool Input::Frame() {
	if (ReadKeyboard() == false)
        return false;

	if (ReadMouse() == false)
        return false;

	ProcessInput();

	m_prevMouseL = IsMouseLPressed() != 0;
	return true;
} // Frame


// 마우스 상태
void Input::GetMouseLocation(int& mouseX, int& mouseY) { mouseX = m_mouseX; mouseY = m_mouseY; }
void Input::GetMouseDelta(int& x, int& y) { x = m_mouseState.lX; y = m_mouseState.lY; }
int Input::GetMouseWheelDelta() { return m_mouseState.lZ; }
bool Input::IsMouseLPressed() { return (m_mouseState.rgbButtons[0] & 0x80) != 0; }
bool Input::IsLeftMouseDown()  { return (m_mouseState.rgbButtons[0] & 0x80) != 0; }
bool Input::IsRightMouseDown() { return (m_mouseState.rgbButtons[1] & 0x80) != 0; }


// 키보드 상태
bool Input::IsEscapePressed()  { return (m_keyboardState[DIK_ESCAPE] & 0x80) != 0; }
bool Input::IsLeftPressed()    { return (m_keyboardState[DIK_LEFT]   & 0x80) != 0; }
bool Input::IsRightPressed()   { return (m_keyboardState[DIK_RIGHT]  & 0x80) != 0; }
bool Input::IsUpPressed()      { return (m_keyboardState[DIK_UP]     & 0x80) != 0; }
bool Input::IsDownPressed()    { return (m_keyboardState[DIK_DOWN]   & 0x80) != 0; }
bool Input::IsZPressed()       { return (m_keyboardState[DIK_Z]      & 0x80) != 0; }
bool Input::IsXPressed()       { return (m_keyboardState[DIK_X]      & 0x80) != 0; }
bool Input::IsPgUpPressed()    { return (m_keyboardState[DIK_PGUP]   & 0x80) != 0; }
bool Input::IsPgDownPressed()  { return (m_keyboardState[DIK_PGDN]   & 0x80) != 0; }


bool Input::IsF1Toggled() {
	if (m_keyboardState[DIK_F1] & 0x80) {
		if (m_F1_released) {
			m_F1_released = false;
			return true;
		}
	} else {
        m_F1_released = true;
    }
	
	return false;
} // IsF1Toggled


bool Input::ReadKeyboard() {
	HRESULT result;

	result = m_keyboard->GetDeviceState(sizeof(m_keyboardState), (LPVOID)&m_keyboardState);
	if (FAILED(result))
	{
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
			m_keyboard->Acquire();
		else return false;
	}

	return true;
} // ReadKeyboard


bool Input::ReadMouse() {
	HRESULT result;

	result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
	if (FAILED(result)) {
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
			m_mouse->Acquire();
		else return false;
	}

	return true;
} // ReadMouse


void Input::ProcessInput() {
	m_mouseX += m_mouseState.lX;
	m_mouseY += m_mouseState.lY;

	if (m_mouseX < 0) { m_mouseX = 0; }
	if (m_mouseY < 0) { m_mouseY = 0; }

	if (m_mouseX > RendererState::ScreenWidth) {
        m_mouseX = RendererState::ScreenWidth;
    }
	if (m_mouseY > RendererState::ScreenHeight) { 
        m_mouseY = RendererState::ScreenHeight;
    }
} // ProcessInput
