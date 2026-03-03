#pragma once
#include "ImGuiWidget.h"

class AssimpModel;

class AssimpModelWidget : public ImGuiWidget {
public:
    AssimpModelWidget(AssimpModel* model);
    virtual ~AssimpModelWidget();

    virtual void Frame() override;
private:
    AssimpModel* m_model;
}; // AssimpModelWidget