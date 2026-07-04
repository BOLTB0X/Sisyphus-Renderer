#pragma once
#include <d3d11.h>
#include <vector>
#include <wrl/client.h>

class PBRMesh {
public:
    struct FBRVertex {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT2 texture;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT3 tangent;
        DirectX::XMFLOAT3 binormal;

        unsigned int boneIDs[4] = { 0, 0, 0, 0 };
        float        boneWeights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

        FBRVertex() : position(0.0f, 0.0f, 0.0f), texture(0.0f, 0.0f), normal(0.0f, 0.0f, 0.0f),
                      tangent(0.0f, 0.0f, 0.0f), binormal(0.0f, 0.0f, 0.0f) {
		} // FBRVertex
    }; // FBRVertex

public:
    PBRMesh();
    ~PBRMesh();

    bool         Init(ID3D11Device*, const std::vector<FBRVertex>&, const std::vector<unsigned int>&, unsigned int);
    void         BindBuffers(ID3D11DeviceContext*);
    void         RenderBuffer(ID3D11DeviceContext*);
    void         RenderInstancedBuffer(ID3D11DeviceContext*, ID3D11Buffer*);
    unsigned int GetMaterialIndex() const;
    UINT         GetIndexCount() const;

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    
    UINT m_indexCount;
    UINT m_materialIndex;
    UINT m_stride;
    UINT m_offset;
}; // PBRMesh