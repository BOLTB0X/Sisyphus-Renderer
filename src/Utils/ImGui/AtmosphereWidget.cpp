#include "Pch.h"
#include "AtmosphereWidget.h"
#include "ImGuiDrawer.h"
#include "Objects/Atmosphere.h"
#include "Objects/SkyBox.h"

AtmosphereWidget::AtmosphereWidget(Atmosphere* atmosphere, SkyBox* sky) {
	m_Atmosphere = atmosphere;
	m_SkyBox = sky;
} // AtmosphereWidget

AtmosphereWidget::~AtmosphereWidget() {
	m_Atmosphere = nullptr;
} // ~

void AtmosphereWidget::Frame() {
	if (ImGuiDrawer::DrawAtmosphere(&m_Atmosphere->GetAtmosphereBuffer())) {
		m_SkyBox->ForceBakeAtmosphere();
	}
} // Frame