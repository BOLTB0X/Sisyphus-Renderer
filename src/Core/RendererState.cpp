#include "Pch.h"
#include "RendererState.h"
// Untils
#include "SharedConstants.h"

bool RendererState::FullScrren = SharedConstants::DefaultScreen::FULL_SCREEN;
bool RendererState::VsyncEnable = SharedConstants::DefaultScreen::VSYNC_ENABLED;
int RendererState::ScreenWidth = SharedConstants::DefaultScreen::WIDTH;
int RendererState::ScreenHeight = SharedConstants::DefaultScreen::HEIGHT;
float RendererState::ScreenDepth = SharedConstants::DefaultScreen::DEPTH;
float RendererState::ScreenNear = SharedConstants::DefaultScreen::NEAR_PLANE;