#pragma once
#include <d3d11.h>
#include <vector>
#include <functional>

enum class ShaderID : uint16_t {
    Stone = 1,
    StonePillar = 2,
    Arca = 3,
    Tree = 4,
    Rakshasa = 5
}; // ShaderID

inline uint64_t GenerateSortKey(uint16_t shaderID, uint16_t materialID, float distance) {
    uint32_t depth = static_cast<uint32_t>(distance * 1000.0f);

    uint64_t key = 0;
    key |= (static_cast<uint64_t>(shaderID) << 48);
    key |= (static_cast<uint64_t>(materialID) << 32);
    key |= static_cast<uint64_t>(depth);
    return key;
} // GenerateSortKey

class RenderQueue {
public:
    struct DrawCommand {
        uint64_t            sortKey;
        ID3D11VertexShader* vs;
        ID3D11PixelShader*  ps;

        std::function<void(ID3D11DeviceContext*)> execute;

        DrawCommand() : sortKey(0), vs(nullptr), ps(nullptr) {
        }
    }; // DrawCommand

public:
    RenderQueue();
    ~RenderQueue();

    void Submit(const DrawCommand& cmd);
    void SortOpaque();
    void SortTransparent();
    void Execute(ID3D11DeviceContext* context);
    void Clear();

private:
    std::vector<DrawCommand> m_commands;
}; // RenderQueue