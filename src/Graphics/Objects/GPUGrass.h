#pragma once
#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
// Utils
#include "SharedConstants/BuffersConstants.h"
// Resources
#include "Resources/ConstantBuffer.h"

class Frustum;

class GPUGrass {
public:
    struct InitParams {
        ID3D11Device*             device;
        HWND                      hwnd;
        ID3D11ShaderResourceView* grass;
        ID3D11SamplerState*       linearSampler;

        InitParams() : device(nullptr), hwnd(0),
            grass(nullptr), linearSampler(nullptr) {
        }
    }; // InitParams

    struct RenderParams {
        ConstantBuffer::PlacementBuffer placementData;
		ID3D11ShaderResourceView*       heightMapSRV;
		ID3D11ShaderResourceView*       normalMapSRV;
        DirectX::XMMATRIX               world;

        RenderParams() : heightMapSRV(nullptr), normalMapSRV(nullptr) {
            placementData = ConstantBuffer::PlacementBuffer();
			world = DirectX::XMMatrixIdentity();
		}
    }; // RenderParams

public:
    GPUGrass();
    ~GPUGrass();

    bool Init(const InitParams&);
    void ComputePlacement(ID3D11DeviceContext*, const RenderParams&);

    void RenderNear(ID3D11DeviceContext*);
    void RenderFar(ID3D11DeviceContext*);
    void OnGui();

    ID3D11Buffer*              GetTreeArgsBuffer() const;
    ID3D11ShaderResourceView*  GetTreeInstanceSRV() const;
    ID3D11UnorderedAccessView* GetTreeInstanceUAV() const;

private:
    struct GrassBuffer {
        float             width;
        float             height;
        float             windStrength;
        float             windSpeed;

        float             dist;
        float             alphaCut;
        DirectX::XMFLOAT2 padding;

        GrassBuffer() {
            using namespace SharedConstants;
            width = BuffersConstants::GRASS_BLADE_WIDTH;
            height = BuffersConstants::GRASS_BLADE_HEIGHT;
            windStrength = BuffersConstants::WIND_STRENGTH;
            windSpeed = BuffersConstants::WIND_SPEED;
            dist = BuffersConstants::DIST;
            alphaCut = BuffersConstants::ALPHA_CUT;
            padding = DirectX::XMFLOAT2(0.0f, 0.0f);
        }
    }; // GrassBuffer

    struct InstanceBuffer {
        DirectX::XMFLOAT3 position;
        float             scale;
        DirectX::XMFLOAT3 normal;
        float             type; // 0: Grass, 1: GrassFar, 2: Tree 
    }; // InstanceBuffer

    struct IndirectArgs {
        UINT VertexCountPerInstance; // 모델의 버텍스 개수
        UINT InstanceCount;
        UINT StartVertexLocation;    // 0
        UINT StartInstanceLocation;  // 0
    }; // IndirectArgs

private:
    bool InitShader(ID3D11Device*, HWND);
    bool InitBuffers(ID3D11Device*);
    bool UpdateGrassBuffer(ID3D11DeviceContext*);

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader>        m_nearVertexShader;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader>      m_nearGeometryShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>         m_nearPixelShader;

    Microsoft::WRL::ComPtr<ID3D11VertexShader>        m_farVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>         m_farPixelShader;

    Microsoft::WRL::ComPtr<ID3D11ComputeShader>       m_placementComputeShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>              m_placementBuffer;

    Microsoft::WRL::ComPtr<ID3D11Buffer>              m_treeInstanceBuffer;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_treeInstanceUAV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>  m_treeInstanceSRV;
    Microsoft::WRL::ComPtr<ID3D11Buffer>              m_treeArgsBuffer;

    Microsoft::WRL::ComPtr<ID3D11Buffer>              m_grassInstanceBuffer;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_grassInstanceUAV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>  m_grassInstanceSRV;
    Microsoft::WRL::ComPtr<ID3D11Buffer>              m_grassArgsBuffer;

    Microsoft::WRL::ComPtr<ID3D11Buffer>              m_grassFarInstanceBuffer;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_grassFarInstanceUAV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>  m_grassFarInstanceSRV;
    Microsoft::WRL::ComPtr<ID3D11Buffer>              m_grassFarArgsBuffer;

    Microsoft::WRL::ComPtr<ID3D11Buffer>              m_worldBuffer;

    Microsoft::WRL::ComPtr<ID3D11Buffer>              m_grassBuffer;
    ID3D11ShaderResourceView*                         m_grassSRV;
    ID3D11SamplerState*                               m_linearSampler;

    GrassBuffer                                       m_grassData;
    GrassBuffer                                       m_prevGrassData;
}; // GPUGrass