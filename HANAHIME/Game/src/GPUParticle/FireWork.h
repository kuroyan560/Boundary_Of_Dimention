#pragma once
#include"DirectX12/D3D12App.h"

class FireWork
{
public:
	FireWork(std::shared_ptr<KuroEngine::RWStructuredBuffer> particle);
	void Init(const KuroEngine::Vec3<float> &emittPos);
	void Update();

private:

	static const int DISPATCH_NUM = 1;
	static const int FIREWORK_PARTICLE_MAX = 1024 * DISPATCH_NUM;

	//â‘âŒç\ë¢ëÃ
	struct FireParticle
	{
		DirectX::XMFLOAT3 startPos;
		DirectX::XMFLOAT3 endPos;
		DirectX::XMFLOAT2 uv;
		int timer;
		int isAliveFlag;
	};

	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_fireUploadBuffer;
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_randomBuffer;

	struct EmittreData
	{
		DirectX::XMFLOAT3 pos;
	};
	std::shared_ptr<KuroEngine::ConstantBuffer>m_emitterBuffer;

	std::shared_ptr<KuroEngine::ComputePipeline>m_fireWorkInitPipeline;
	std::shared_ptr<KuroEngine::ComputePipeline>m_fireWorkUpdatePipeline;

	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_gpuParticleBuffer;

	std::shared_ptr<KuroEngine::TextureBuffer>m_particleColor;

	KuroEngine::Vec3<float>m_emitterPos;
};