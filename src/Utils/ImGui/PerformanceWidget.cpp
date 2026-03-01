#include "Pch.h"
#include "PerformanceWidget.h"
#include "ImGuiDrawer.h"
#define NAME "Performance"

PerformanceWidget::PerformanceWidget(
    const int& fps, const long& cpu, const float& frameTime)
: ImGuiWidget(NAME),
  m_fps(fps), m_cpu(cpu), m_frameTime(frameTime) {
} //PerformanceWidget

void PerformanceWidget::Frame() {
    ImGuiDrawer::DrawPerformance(m_fps, m_cpu, m_frameTime);
} // Frame