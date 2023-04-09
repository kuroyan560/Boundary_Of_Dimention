#pragma once
#include"GPUParticleRender.h"

/// <summary>
/// ステージ外での蛍の生成
/// </summary>
class FireFlyOutStage
{
public:
	FireFlyOutStage(std::shared_ptr<KuroEngine::RWStructuredBuffer> particle_buffer);

	void Compute();
private:
	std::shared_ptr<KuroEngine::ComputePipeline> m_cPipeline;
	std::shared_ptr<KuroEngine::RWStructuredBuffer> m_particleData;
};

