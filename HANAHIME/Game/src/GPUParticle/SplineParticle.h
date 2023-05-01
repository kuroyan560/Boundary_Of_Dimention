#pragma once
#include"DirectX12/D3D12App.h"
#include"ForUser/Object/Object.h"
#include"ForUser/Timer.h"

class SplineParticle
{
public:
	SplineParticle(std::shared_ptr<KuroEngine::RWStructuredBuffer> particle);
	void Init(std::vector<KuroEngine::Vec3<float>>posArray);
	void Update();

	bool IsHalf()
	{
		return m_posArray.size() / 2 <= cd.startIndex && m_splineTimer.IsTimeUp();
	}
	bool IsFinish()
	{
		return m_finishFlag;
	}
private:

	std::vector<DirectX::XMFLOAT3>m_posArray;

	bool m_initFlag;
	bool m_finishFlag;
	//ãOê’----------------------------------------

	struct SplineData
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 vel;
		DirectX::XMFLOAT4 color;
		int startIndex;
		float rate;
	};

	struct ConstData
	{
		DirectX::XMMATRIX scaleRotate;
		UINT startIndex;
		float rate;

		ConstData()
		{
			startIndex = 0;
			rate = 0.0f;
		};
	};

	std::shared_ptr<KuroEngine::RWStructuredBuffer> m_particleBuffer;
	std::shared_ptr<KuroEngine::RWStructuredBuffer> m_limitIndexPosBuffer;
	std::shared_ptr<KuroEngine::RWStructuredBuffer> m_gpuParticleBuffer;

	std::shared_ptr<KuroEngine::ConstantBuffer> m_scaleRotaBuffer;
	std::shared_ptr<KuroEngine::ConstantBuffer> m_limitIndexBuffer;

	std::shared_ptr<KuroEngine::ComputePipeline> m_initLoucusPipeline;
	std::shared_ptr<KuroEngine::ComputePipeline> m_updateLoucusPipeline;

	ConstData cd;
	KuroEngine::Timer m_splineTimer;

	static const int LIMIT_POS_MAX = 10;
	std::array<std::shared_ptr<KuroEngine::ModelObject>, LIMIT_POS_MAX>limitPosArray;
};

