#pragma once
#include <dxgi.h>
#include <string>

// 그래픽 카드 및 모니터 정보 조사
class DisplayInfo {
public:
    DisplayInfo();
	DisplayInfo(const DisplayInfo&);
    ~DisplayInfo();

    bool Init(int, int);

    unsigned int GetNumerator() const;
    unsigned int GetDenominator() const;
    int          GetVideoCardMemory() const;
    const char*  GetVideoCardDescription();

private:
    unsigned int m_numerator;
    unsigned int m_denominator;
    int          m_videoCardMemory;
    char         m_videoCardDescription[128];
}; // DisplayInfo