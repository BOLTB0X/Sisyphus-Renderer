// Cpu.h
#pragma once
#pragma comment(lib, "pdh.lib")
#include <pdh.h>

class Cpu {
public:
    Cpu();
    Cpu(const Cpu& other);
    ~Cpu();

    void       Init();
    void       Shutdown();
    void       Frame();

    const long& GetCpuPercentage();

private:
    bool          m_canReadCpu;
    HQUERY        m_queryHandle;
    HCOUNTER      m_counterHandle;
    unsigned long m_lastSampleTime;
    long          m_cpuUsage;
}; // Cpu