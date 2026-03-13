#pragma once
#include "ImGuiWidget.h"

class Atmosphere;
class SkyBox;

class AtmosphereWidget : public ImGuiWidget {
public:
    AtmosphereWidget(Atmosphere* atmosphere, SkyBox* sky);
    virtual ~AtmosphereWidget();

    virtual void Frame() override;

private:
    Atmosphere* m_Atmosphere;
    SkyBox*     m_SkyBox;
}; // ImGuiWidget