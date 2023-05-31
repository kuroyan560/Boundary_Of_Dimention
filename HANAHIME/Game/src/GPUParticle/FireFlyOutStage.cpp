#include "FireFlyOutStage.h"

FireFlyOutStage::FireFlyOutStage(std::shared_ptr<KuroEngine::RWStructuredBuffer> particle_buffer) :m_particleData(particle_buffer)
{
	std::vector<KuroEngine::RootParam>rootParam =
	{
		KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"蛍パーティクルの情報(RWStructuredBuffer)"),
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"プレイヤーの情報(ConstantBuffer)")
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
		KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"プレイヤーの情報(ConstantBuffer)")
	};
	auto cs_update = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/FireFlyOutStage.hlsl", "UpdateMain", "cs_6_4");
	m_updatePipeline = KuroEngine::D3D12App::Instance()->GenerateComputePipeline(cs_update, updateRootParam, { KuroEngine::WrappedSampler(true,true) });


	m_scaleRotateData = KuroEngine::D3D12App::Instance()->GenerateConstantBuffer(sizeof(DirectX::XMMATRIX), 1);
	DirectX::XMMATRIX mat = DirectX::XMMatrixIdentity();
	m_scaleRotateData->Mapping(&mat);


	m_playerData = KuroEngine::D3D12App::Instance()->GenerateConstantBuffer(sizeof(DirectX::XMFLOAT3), 1);
}

void FireFlyOutStage::ComputeInit(const KuroEngine::Vec3<float> &arg_pos)
{
	DirectX::XMFLOAT3 pos = { arg_pos.x,arg_pos.y,arg_pos.z };
	m_playerData->Mapping(&pos);

	std::vector<KuroEngine::RegisterDescriptorData>descData =
	{
		{m_fireFlyParticleData,KuroEngine::UAV},
		{m_playerData,KuroEngine::CBV}
	};
	KuroEngine::D3D12App::Instance()->DispathOneShot(m_initPipeline, { DISPATCH_MAX,1,1 }, descData);
}

void FireFlyOutStage::ComputeUpdate(const KuroEngine::Vec3<float> &arg_pos)
{
	DirectX::XMFLOAT3 pos = { arg_pos.x,arg_pos.y,arg_pos.z };
	m_playerData->Mapping(&pos);

	std::vector<KuroEngine::RegisterDescriptorData>descData =
	{
		{m_fireFlyParticleData,KuroEngine::UAV},
		{m_particleData,KuroEngine::UAV},
		{m_scaleRotateData,KuroEngine::CBV},
		{m_playerData,KuroEngine::CBV}
	};
	KuroEngine::D3D12App::Instance()->DispathOneShot(m_updatePipeline, { DISPATCH_MAX,1,1 }, descData);
}
