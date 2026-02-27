#pragma once

// Windows
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// DirectX
#include <d3d11.h>
#include <directxmath.h>
#include <d3dcompiler.h>
#include <directxcolors.h>
#include <wrl/client.h>

// DirectXTK
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <CommonStates.h>
#include <Effects.h>
#include <DDSTextureLoader.h>

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Third party
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

// STL
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <map>
#include <unordered_map>
#include <algorithm>