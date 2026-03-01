#pragma once
#include <d3d11.h>
#include <vector>
#include <wrl/client.h>
#include "VertexTypes.h"

class PBRMesh {
public:
    PBRMesh();
    ~PBRMesh();

    bool         Init(ID3D11Device*, const std::vector<VertexTypes::FBRVertex>&, const std::vector<unsigned int>&, unsigned int);
    void         RenderBuffer(ID3D11DeviceContext*);
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