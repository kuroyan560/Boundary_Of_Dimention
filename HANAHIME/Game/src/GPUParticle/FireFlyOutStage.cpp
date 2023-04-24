#include "FireFlyOutStage.h"

FireFlyOutStage::FireFlyOutStage(std::shared_ptr<KuroEngine::RWStructuredBuffer> particle_buffer):m_particleData(particle_buffer)
{
	std::vector<KuroEngine::RootParam>rootParam =
	{
		KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"蛍パーティクルの情報(RWStructuredBuffer)"),
	};
	auto cs_init = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/FireFlyOutStage.hlsl", "InitMain", "cs_6_4");
	m_initPipeline = KuroEngine::D3D12App::Instance()->GenerateComputePipeline(cs_init, rootParam, { KuroEngine::WrappedSampler(true,true) });


	KuroEngine::D3D12App::Instance()->GenerateRWStructuredBuffer(
		&m_fireFlyParticleData, &m_fireFlyParticleCounterData,
		sizeof(FireFlyData),
		FIRE_FLY_MAX,
		nullptr,
		"FireFlyParticleData - RWStructuredBuffer");

	std::vector<KuroEngine::RootParam> updateRootParam =
	{
		KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"蛍パーティクルの情報(RWStructuredBuffer)"),
		KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"パーティクル情報(RWStructuredBuffer)"),
		KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"スケールと角度の情報(ConstantBuffer)"),
	};
	auto cs_update = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/FireFlyOutStage.hlsl", "UpdateMain", "cs_6_4");
	m_updatePipeline = KuroEngine::D3D12App::Instance()->GenerateComputePipeline(cs_update, updateRootParam, { KuroEngine::WrappedSampler(true,true) });


	m_scaleRotateData = KuroEngine::D3D12App::Instance()->GenerateConstantBuffer(sizeof(DirectX::XMMATRIX), 1);
	DirectX::XMMATRIX mat = DirectX::XMMatrixIdentity();
	mat *= DirectX::XMMatrixScaling(10.0f, 10.0f, 10.0f);
	m_scaleRotateData->Mapping(&mat);

	ComputeInit();
}

void FireFlyOutStage::ComputeInit()
{
	std::vector<KuroEngine::RegisterDescriptorData>descData =
	{
		{m_fireFlyParticleData,KuroEngine::UAV},
	};
	KuroEngine::D3D12App::Instance()->DispathOneShot(m_initPipeline, { 1,1,1 }, descData);
}

void FireFlyOutStage::ComputeUpdate()
{
	std::vector<KuroEngine::RegisterDescriptorData>descData =
	{
		{m_fireFlyParticleData,KuroEngine::UAV},
		{m_particleData,KuroEngine::UAV},
		{m_scaleRotateData,KuroEngine::CBV}
	};
	KuroEngine::D3D12App::Instance()->DispathOneShot(m_updatePipeline, { 1,1,1 }, descData);
}
