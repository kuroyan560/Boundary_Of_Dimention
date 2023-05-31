#pragma once
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"DrawExcuteIndirect.h"
#include"Render/GraphicsManager.h"
#include"Common/Singleton.h"

//演出で使われている全てのパーティクルの情報をスタックして一気に描画するクラスです
class GPUParticleRender : public KuroEngine::DesignPattern::Singleton<GPUParticleRender>
{
public:
	GPUParticleRender(int MAXNUM = 3000000);

	void InitCount();
	void Draw(KuroEngine::Camera &camera);

	std::shared_ptr<KuroEngine::RWStructuredBuffer>GetStackBuffer()const;

	struct InputData
	{
		DirectX::XMMATRIX world;
		DirectX::XMFLOAT4 color;
	};
private:

	struct OutputData
	{
		DirectX::XMMATRIX mat;
		DirectX::XMFLOAT4 color;
	};


	int dispatchNum = 50;
	int particleMaxNum = 1024 * dispatchNum;
	/*ResouceBufferHelper computeCovertWorldMatToDrawMat;
	KazGPUParticle::RESOURCE_HANDLE  worldMatHandle, outputHandle, viewProjMatHandle;

	DirectX::XMMATRIX viewProjMat;

	std::unique_ptr<DrawExcuteIndirect> excuteIndirect;
	KazGPUParticle::RESOURCE_HANDLE  vertexBufferHandle;
	KazGPUParticle::RESOURCE_HANDLE  indexBufferHandle;
	std::unique_ptr<KazGPUParticle::ID3D12ResourceWrapper> vertexBuffer, indexBuffer;

	KazGPUParticle::ID3D12ResourceWrapper gpuVertexBuffer, gpuIndexBuffer;
	KazGPUParticle::ID3D12ResourceWrapper copyBuffer;*/


	std::shared_ptr<KuroEngine::ComputePipeline> m_cPipeline;
	std::shared_ptr<KuroEngine::GraphicsPipeline> m_gPipeline;
	Microsoft::WRL::ComPtr<ID3D12RootSignature>rootsignature;

	struct SpriteVertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
	};
	std::shared_ptr<KuroEngine::VertexBuffer> m_particleVertex;
	std::shared_ptr<KuroEngine::IndexBuffer> m_particleIndex;

	//蛍情報のバッファ

	struct DrawData
	{
		DirectX::XMMATRIX matrix;
		DirectX::XMFLOAT4 color;
	};
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_fireFlyArrayBuffer;
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_fireFlyCounterBuffer;

	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_fireFlyDrawBuffer;
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_fireFlyDrawCounterBuffer;

	struct ViewProjMatData
	{
		DirectX::XMMATRIX scaleRotateBillboardMat;
		DirectX::XMMATRIX viewprojMat;
		UINT particleMaxNum;
	};
	std::shared_ptr<KuroEngine::ConstantBuffer>m_viewPorjBuffer;


	std::unique_ptr<DrawExcuteIndirect>excuteIndirect;
	void InitVerticesPos(DirectX::XMFLOAT3 *LEFTUP_POS, DirectX::XMFLOAT3 *LEFTDOWN_POS, DirectX::XMFLOAT3 *RIGHTUP_POS, DirectX::XMFLOAT3 *RIGHTDOWN_POS, const DirectX::XMFLOAT2 &ANCHOR_POINT)
	{
		*LEFTUP_POS = { (0.0f - ANCHOR_POINT.x), (0.0f - ANCHOR_POINT.y),0.0f };
		*LEFTDOWN_POS = { (0.0f - ANCHOR_POINT.x), (1.0f - ANCHOR_POINT.y),0.0f };
		*RIGHTUP_POS = { (1.0f - ANCHOR_POINT.x), (0.0f - ANCHOR_POINT.y) ,0.0f };
		*RIGHTDOWN_POS = { (1.0f - ANCHOR_POINT.x), (1.0f - ANCHOR_POINT.y) ,0.0f };
	}
	void InitUvPos(DirectX::XMFLOAT2 *UV_LEFTUP_POS, DirectX::XMFLOAT2 *UV_LEFTDOWN_POS, DirectX::XMFLOAT2 *UV_RIGHTUP_POS, DirectX::XMFLOAT2 *UV_RIGHTDOWN_POS)
	{
		*UV_LEFTUP_POS = { 0.0f,0.0f };
		*UV_LEFTDOWN_POS = { 0.0f,1.0f };
		*UV_RIGHTUP_POS = { 1.0f,0.0f };
		*UV_RIGHTDOWN_POS = { 1.0f,1.0f };
	}

};

