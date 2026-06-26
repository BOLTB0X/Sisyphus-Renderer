#include "Pch.h"
#include "RenderQueue.h"
// STL
#include <algorithm>
#include <DirectXMath.h>

RenderQueue::RenderQueue() {
} // RenderQueue

RenderQueue::~RenderQueue() {
} // ~RenderQueue

void RenderQueue::Submit(const DrawCommand& cmd) {
	m_commands.push_back(cmd);
} // Submit

void RenderQueue::SortOpaque() {
	std::sort(m_commands.begin(), m_commands.end(), [](const DrawCommand& a, const DrawCommand& b) {
		return a.sortKey < b.sortKey;
	});
} // SortOpaque

void RenderQueue::SortTransparent() {
	std::sort(m_commands.begin(), m_commands.end(), [](const DrawCommand& a, const DrawCommand& b) {
		return a.sortKey > b.sortKey;
	});
} // SortTransparent

void RenderQueue::Execute(ID3D11DeviceContext* context) {
    ID3D11VertexShader* lastVS = nullptr;
    ID3D11PixelShader* lastPS = nullptr;

    for (auto& cmd : m_commands) {
        if (cmd.vs != lastVS) {
            context->VSSetShader(cmd.vs, nullptr, 0);
            lastVS = cmd.vs;
        }
        if (cmd.ps != lastPS) {
            context->PSSetShader(cmd.ps, nullptr, 0);
            lastPS = cmd.ps;
        }

        if (cmd.execute) {
            cmd.execute(context);
        }
    }
} // Execute

void RenderQueue::Clear() {
    m_commands.clear();
} // Clear