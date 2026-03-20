#pragma once
#include "ImGuiWidget.h"
#include <functional>
#include "imgui.h"

class FunctionWidget : public ImGuiWidget {
public:
    FunctionWidget(std::string, std::function<void()>);

    void Frame() override;

private:
    std::string m_name;
    std::function<void()> m_guiFunc;
}; // FunctionWidget