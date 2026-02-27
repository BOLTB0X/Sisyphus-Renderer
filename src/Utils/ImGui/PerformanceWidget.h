#pragma once
#include "ImGuiWidget.h"

class PerformanceWidget : public ImGuiWidget {
public:
    PerformanceWidget(const int&, const long&, const float&);
    PerformanceWidget(const PerformanceWidget& other) = delete;

    virtual      ~PerformanceWidget() = default;
    virtual void Frame() override;
    
private:
    const int&   m_fps;
    const long&   m_cpu;
    const float& m_frameTime;
}; // PerformanceWidget