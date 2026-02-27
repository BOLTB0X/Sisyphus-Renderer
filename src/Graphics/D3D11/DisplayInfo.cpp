#include "Pch.h"
#include "DisplayInfo.h"
#include <dxgi1_2.h>
#include <wrl/client.h>
// Utils
#include "Helpers/DebugHelper.h"
// STL
#include <vector>

DisplayInfo::DisplayInfo()
    : m_numerator(0),
      m_denominator(1),
      m_videoCardMemory(0) {
    m_videoCardDescription[0] = '\0';
} // DisplayInfo

DisplayInfo::DisplayInfo(const DisplayInfo&)
    : m_numerator(0),
      m_denominator(1),
      m_videoCardMemory(0) {
    m_videoCardDescription[0] = '\0';
} // DisplayInfo

DisplayInfo::~DisplayInfo(){
} // ~DisplayInfo

bool DisplayInfo::Init(int screenWidth, int screenHeight)
{
    IDXGIFactory* factory;
    IDXGIAdapter* adapter;
    IDXGIOutput* adapterOutput;
    unsigned int numModes = 0;
    DXGI_ADAPTER_DESC adapterDesc;

    HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)&factory);
    if (FAILED(hr))
    {
        DebugHelper::DebugPrint("DXGI Factory 생성 실패");
        return false;
    }

    hr = factory->EnumAdapters(0, &adapter);
    // 기본 그래픽 카드(어댑터) 정보 가져오기
    if (FAILED(hr)) {
        DebugHelper::DebugPrint("기본 그래픽 어댑터 탐색 실패");
        factory->Release();
        return false;
    }

    // 기본 출력 장치(모니터) 정보 가져오기
    hr = adapter->EnumOutputs(0, &adapterOutput);
    if (FAILED(hr)) {
        DebugHelper::DebugPrint("기본 출력 장치(모니터) 탐색 실패");
        adapter->Release();
        factory->Release();
        return false;
    }

    // 지원하는 디스플레이 모드 리스트 개수 파악
    hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
    if (FAILED(hr)) {
        DebugHelper::DebugPrint("디스플레이 모드 개수 파악 실패");
        return false;
    }

    // 모드 리스트 채우기
    std::vector<DXGI_MODE_DESC> displayModeList(numModes);
    hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList.data());
    if (FAILED(hr)) {
        DebugHelper::DebugPrint("디스플레이 모드 리스트 획득 실패");
        return false;
    }
    // 새로고침 빈도(Refresh Rate) 추출
    for (unsigned int i = 0; i < numModes; i++) {
        if (displayModeList[i].Width == (unsigned int)screenWidth) {
            if (displayModeList[i].Height == (unsigned int)screenHeight) {
                m_numerator = displayModeList[i].RefreshRate.Numerator;
                m_denominator = displayModeList[i].RefreshRate.Denominator;
            }
        }
    }

    // 그래픽 카드 설명 및 메모리 저장
    hr = adapter->GetDesc(&adapterDesc);
    if (FAILED(hr)) {
        DebugHelper::DebugPrint("그래픽 카드 상세정보 획득 실패");
        return false;
    }

    m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);
    size_t stringLength;
    if (wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128) != 0) {
        return false;
    }

    adapterOutput->Release();
    adapter->Release();
    factory->Release();

    return true;
} // Init

unsigned int DisplayInfo::GetNumerator() const { return m_numerator; }
unsigned int DisplayInfo::GetDenominator() const { return m_denominator; }
int          DisplayInfo::GetVideoCardMemory() const { return m_videoCardMemory; }
const char*  DisplayInfo::GetVideoCardDescription() { return m_videoCardDescription; }