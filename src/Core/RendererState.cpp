#include "Pch.h"
#include "RendererState.h"
// Untils
#include "SharedConstants/ScreenConstants.h"

using namespace SharedConstants;

bool RendererState::FullScrren = ScreenConstants::FULL_SCREEN;
bool RendererState::VsyncEnable = ScreenConstants::VSYNC_ENABLED;
int RendererState::ScreenWidth = ScreenConstants::WIDTH;
int RendererState::ScreenHeight = ScreenConstants::HEIGHT;
float RendererState::ScreenDepth = ScreenConstants::DEPTH;
float RendererState::ScreenNear = ScreenConstants::NEAR_PLANE;