// Cpu.cpp
#include "Pch.h"
#include "Cpu.h"

Cpu::Cpu()
: m_canReadCpu(true),
  m_queryHandle(nullptr),
  m_counterHandle(nullptr),
  m_lastSampleTime(0),
  m_cpuUsage(0) {
} // Cpu

Cpu::Cpu(const Cpu& other)
: m_canReadCpu(true),
  m_queryHandle(nullptr),
  m_counterHandle(nullptr),
  m_lastSampleTime(0),
  m_cpuUsage(0) {
} // Cpu

Cpu::~Cpu() {
} // ~Cpu

void Cpu::Init() {
    if (PdhOpenQuery(NULL, 0, &m_queryHandle) == ERROR_SUCCESS) {
        PdhAddCounter(m_queryHandle, TEXT("\\Processor(_Total)\\% Processor Time"), 0, &m_counterHandle);
        m_canReadCpu = true;
    }
    m_lastSampleTime = GetTickCount();
} // Init

void Cpu::Shutdown() {
    if (m_canReadCpu)
    {
        PdhCloseQuery(m_queryHandle);
    }
} // Shutdown

void Cpu::Frame() {
    if (m_canReadCpu && (m_lastSampleTime + 1000) < GetTickCount()) {
        m_lastSampleTime = GetTickCount();
        PdhCollectQueryData(m_queryHandle);
        
        PDH_FMT_COUNTERVALUE value;
        PdhGetFormattedCounterValue(m_counterHandle, PDH_FMT_LONG, NULL, &value);
        m_cpuUsage = value.longValue;
    }
} // Frame

const long& Cpu::GetCpuPercentage() { return m_cpuUsage; }
// GetCpuPercentage