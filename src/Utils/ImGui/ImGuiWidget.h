#pragma once
#include <string>

class ImGuiWidget {
public:
    ImGuiWidget();
    ImGuiWidget(const std::string&);
	ImGuiWidget(const ImGuiWidget&) = delete;

    virtual      ~ImGuiWidget() = default;
    virtual void Frame() = 0;
    
public:
    void               SetVisible(bool);
    bool               IsVisible() const;
    const std::string& GetName() const;

private:
    std::string m_name;
    bool        m_isVisible;
}; // ImGuiWidget
