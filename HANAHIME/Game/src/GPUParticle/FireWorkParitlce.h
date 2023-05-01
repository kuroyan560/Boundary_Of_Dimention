#pragma once
#include"DirectX12/D3D12App.h"

class FireWorkParticle
{
public:
	FireWorkParticle(std::shared_ptr<KuroEngine::RWStructuredBuffer> particle);
	void Init(const KuroEngine::Vec3<float> &emittPos);
	void Update();

private:
	//â‘âŒç\ë¢ëÃ
	struct FireParticle
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 vel;
		int timer;
		int initFlag;
	};

	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_fireUploadBuffer;
	std::shared_ptr<KuroEngine::ConstantBuffer>m_scaleRotaBuffer;

	struct EmittreData
	{
		DirectX::XMFLOAT3 pos;
		float pad;
		DirectX::XMFLOAT3 vel;
	};
	std::shared_ptr<KuroEngine::ConstantBuffer>m_emitterBuffer;

	std::shared_ptr<KuroEngine::ComputePipeline>m_fireWorkInitPipeline;
	std::shared_ptr<KuroEngine::ComputePipeline>m_fireWorkUpdatePipeline;

	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_gpuParticleBuffer;


	KuroEngine::Vec3<float>m_emitterPos;
};