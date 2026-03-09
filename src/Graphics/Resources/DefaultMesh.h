#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

 class DefaultMesh {
 public:
     enum class DefaultMeshType {
         Quad,
         Cube,
     }; // DefaultMeshType

     struct BoxVertex {
         DirectX::XMFLOAT3 position;
         DirectX::XMFLOAT2 texcoord;
     }; // BoxVertex

 public:
     DefaultMesh();
     ~DefaultMesh();

     bool Init(ID3D11Device*, UINT, DefaultMeshType);
     void RenderBuffer(ID3D11DeviceContext*);
     UINT GetVertexCount() const;
     UINT GetIndexCount() const;

 private:
	 bool InitCube(ID3D11Device*, UINT);
	 bool InitQuad(ID3D11Device*, UINT);

 private:
     Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
     Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
     UINT m_vertexCount, m_indexCount;
 }; // DefaultMesh