#include "Pch.h"
#include "AssimpModelWidget.h"
#include "ImGuiDrawer.h"
// Graphics
#include "Resources/AssimpModel.h"
#define NAME "AssimpModel"

AssimpModelWidget::AssimpModelWidget(AssimpModel* model)
    :ImGuiWidget(NAME), m_model(model) {
} // AssimpModelWidget

AssimpModelWidget::~AssimpModelWidget() {
    m_model = nullptr;
} // ~AssimpModelWidget

void AssimpModelWidget::Frame() {
    ImGuiDrawer::DrawAssimpModel(m_model);
} // Frame