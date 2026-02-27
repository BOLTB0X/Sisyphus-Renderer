// Fps.h
#pragma once
#pragma comment(lib, "winmm.lib")
#include <windows.h>
#include <mmsystem.h>


class Fps {
public:
	Fps();
	Fps(const Fps&);
	~Fps();

	void        Init();
	void        Frame();
	const int&  GetFps();

private:
	int m_fps;
	int m_count;
	unsigned long m_startTime;
}; // Fps