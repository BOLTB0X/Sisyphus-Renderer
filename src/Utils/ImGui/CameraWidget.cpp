#include "Pch.h"
#include "CameraWidget.h"
#include "ImGuiDrawer.h"
// Graphics
#include "Camera/Camera.h"

CameraWidget::CameraWidget(Camera* camera)
: m_camera(camera) {
} // CameraWidget

CameraWidget::~CameraWidget() {
    m_camera = nullptr;
} // ~CameraWidget

void CameraWidget::Frame() {
    ImGuiDrawer::DrawCamera(m_camera);
} // Frame