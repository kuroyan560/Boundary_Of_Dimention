#include "SignSpotFireFly.h"

SignSpotFireFly::SignSpotFireFly(std::shared_ptr<KuroEngine::RWStructuredBuffer>particle_buffer) :m_particleBuffer(particle_buffer)
{
	m_fireFlyArrayBuffer =
		KuroEngine::D3D12App::Instance()->GenerateRWStructuredBuffer(
			sizeof(DirectX::XMFLOAT3),
			1024,
			nullptr,
			"signSpotParticle - RWStructureBuffer"
		);

	m_larpFireFlyArrayBuffer =
		KuroEngine::D3D12App::Instance()->GenerateRWStructuredBuffer(
			sizeof(DirectX::XMFLOAT3),
			1024,
			nullptr,
			"larpSignSpotParticle - RWStructureBuffer"
		);


	m_commonBuffer = KuroEngine::D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(CommonData),
		1,
		nullptr,
		"CommonData - RWStructureBuffer"
	);


	{
		std::vector<KuroEngine::RootParam>rootParam =
		{
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"蛍パーティクルの情報(RWStructuredBuffer)"),
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"蛍パーティクルのラープ情報(RWStructuredBuffer)"),
		};
		auto cs_init = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/SignSpot.hlsl", "InitMain", "cs_6_4");
		m_cInitPipeline = KuroEngine::D3D12App::Instance()->GenerateComputePipeline(cs_init, rootParam, { KuroEngine::WrappedSampler(true,true) });
	}

	{
		std::vector<KuroEngine::RootParam>rootParam =
		{
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"蛍パーティクルの情報(RWStructuredBuffer)"),
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"蛍パーティクルのラープ情報(RWStructuredBuffer)"),
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"描画用の情報(RWStructuredBuffer)"),
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"描画の共通情報(RWStructuredBuffer)")
		};
		auto cs_update = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/SignSpot.hlsl", "UpdateMain", "cs_6_4");
		m_cUpdatePipeline = KuroEngine::D3D12App::Instance()->GenerateComputePipeline(cs_update, rootParam, { KuroEngine::WrappedSampler(true,true) });
	}

	std::vector<KuroEngine::RegisterDescriptorData>descData =
	{
		{m_fireFlyArrayBuffer,KuroEngine::UAV},
		{m_larpFireFlyArrayBuffer,KuroEngine::UAV}
	};
	KuroEngine::D3D12App::Instance()->DispathOneShot(m_cInitPipeline, { 1,1,1 }, descData);



	m_particleMove = std::make_shared<KuroEngine::ModelObject>("resource/user/model/", "Player.glb");

}

void SignSpotFireFly::Update()
{
	data.scaleRotaMat = DirectX::XMMatrixScaling(20.0f, 20.0f, 20.0f);
	m_commonBuffer->Mapping(&data);

	std::vector<KuroEngine::RegisterDescriptorData>descData =
	{
		{m_fireFlyArrayBuffer,KuroEngine::UAV},
		{m_larpFireFlyArrayBuffer,KuroEngine::UAV},
		{m_particleBuffer,KuroEngine::UAV},
		{m_commonBuffer,KuroEngine::CBV},
	};
	KuroEngine::D3D12App::Instance()->DispathOneShot(m_cUpdatePipeline, { 1,1,1 }, descData);



	KuroEngine::Vec3<float>pos(data.emittPos.x, data.emittPos.y, data.emittPos.z);
	KuroEngine::Vec3<float>length = pos - m_particleMove->m_transform.GetPos();
	length.Normalize();
	length *= 5.0f;

	m_vel = KuroEngine::Math::Lerp(m_vel, length, 0.01f);

	m_particlePos += m_vel;
	m_particleMove->m_transform.SetPos(m_particlePos);


	if (m_finishFlag)
	{
		data.alpha = KuroEngine::Math::Lerp(data.alpha, 0.0f, 0.1f);
	}
	else
	{
		data.alpha = KuroEngine::Math::Lerp(data.alpha, 1.0f, 0.1f);
	}
	m_finishFlag = false;
}

void SignSpotFireFly::Draw(KuroEngine::Camera &camera)
{
	KuroEngine::DrawFunc3D::DrawNonShadingModel(m_particleMove, camera);
}

void SignSpotFireFly::Finish()
{
	m_finishFlag = true;
}

void SignSpotFireFly::GoThisPos(const KuroEngine::Vec3<float> pos)
{
	data.emittPos = pos;
}
