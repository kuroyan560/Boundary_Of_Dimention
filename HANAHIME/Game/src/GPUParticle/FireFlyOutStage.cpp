#include "FireFlyOutStage.h"

FireFlyOutStage::FireFlyOutStage(std::shared_ptr<KuroEngine::RWStructuredBuffer> particle_buffer):m_particleData(particle_buffer)
{
	std::vector<KuroEngine::RootParam>rootParam =
	{
		KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"蛍パーティクルの情報(RWStructuredBuffer)"),
	};
	auto cs_init = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/FireFlyOutStage.hlsl", "InitMain", "cs_6_4");
	m_cPipeline = KuroEngine::D3D12App::Instance()->GenerateComputePipeline(cs_init, rootParam, { KuroEngine::WrappedSampler(true,true) });

	Compute();
}

void FireFlyOutStage::Compute()
{
	std::vector<KuroEngine::RegisterDescriptorData>descData =
	{
		{m_particleData,KuroEngine::UAV},
	};
	KuroEngine::D3D12App::Instance()->DispathOneShot(m_cPipeline, { 1,1,1 }, descData);
}
