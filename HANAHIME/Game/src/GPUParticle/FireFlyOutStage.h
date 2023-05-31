#pragma once
#include"GPUParticleRender.h"

/// <summary>
/// ステージ外での蛍の生成
/// </summary>
class FireFlyOutStage
{
public:
	FireFlyOutStage(std::shared_ptr<KuroEngine::RWStructuredBuffer> particle_buffer);

	void ComputeInit(const KuroEngine::Vec3<float> &arg_pos);
	void ComputeUpdate(const KuroEngine::Vec3<float> &arg_pos);

private:
	static const int DISPATCH_MAX = 5;
	static const int FIRE_FLY_MAX = 1024 * DISPATCH_MAX;

	std::shared_ptr<KuroEngine::ComputePipeline> m_initPipeline;
	std::shared_ptr<KuroEngine::ComputePipeline> m_updatePipeline;

	struct FireFlyData
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 vel;
		DirectX::XMFLOAT2 scale;
		DirectX::XMFLOAT4 color;
		DirectX::XMUINT3 timer;
		DirectX::XMFLOAT2 angle;
		DirectX::XMFLOAT3 basePos;
	};
	std::shared_ptr<KuroEngine::RWStructuredBuffer> m_fireFlyParticleData;
	std::shared_ptr<KuroEngine::RWStructuredBuffer> m_fireFlyParticleCounterData;

	std::shared_ptr<KuroEngine::ConstantBuffer> m_scaleRotateData;
	std::shared_ptr<KuroEngine::ConstantBuffer> m_playerData;

	std::shared_ptr<KuroEngine::RWStructuredBuffer> m_particleData;
};

