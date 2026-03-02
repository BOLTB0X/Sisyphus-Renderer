#pragma once
#include "ImGuiWidget.h"

class Camera;

class CameraWidget : public ImGuiWidget {
public:
    CameraWidget(Camera* camera);
    virtual ~CameraWidget();

    virtual void Frame() override;

private:
    Camera* m_camera;
}; // ImGuiWidget