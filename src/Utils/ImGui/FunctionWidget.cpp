#include "Pch.h"
#include "FunctionWidget.h"
//
#include "imgui.h"

FunctionWidget::FunctionWidget(std::string name, std::function<void()> guiFunc)
    : m_name(name), m_guiFunc(guiFunc) {
} // FunctionWidget

void FunctionWidget::Frame() {
    ImGui::Begin(m_name.c_str());
    if (m_guiFunc) m_guiFunc();
    ImGui::End();
} // Frame