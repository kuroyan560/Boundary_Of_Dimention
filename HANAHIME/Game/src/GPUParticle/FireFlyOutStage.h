#pragma once
#include"GPUParticleRender.h"

/// <summary>
/// ステージ外での蛍の生成
/// </summary>
class FireFlyOutStage
{
public:
	FireFlyOutStage(std::shared_ptr<KuroEngine::RWStructuredBuffer> particle_buffer);

	void ComputeInit();
	void ComputeUpdate();

private:
	static const int FIRE_FLY_MAX = 1024;
	std::shared_ptr<KuroEngine::ComputePipeline> m_initPipeline;
	std::shared_ptr<KuroEngine::ComputePipeline> m_updatePipeline;

	struct FireFlyData
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 color;
		DirectX::XMUINT2 timer;
	};
	std::shared_ptr<KuroEngine::RWStructuredBuffer> m_fireFlyParticleData;
	std::shared_ptr<KuroEngine::RWStructuredBuffer> m_fireFlyParticleCounterData;

	std::shared_ptr<KuroEngine::ConstantBuffer> m_scaleRotateData;

	std::shared_ptr<KuroEngine::RWStructuredBuffer> m_particleData;
};

