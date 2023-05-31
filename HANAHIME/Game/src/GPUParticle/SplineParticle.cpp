#include "SplineParticle.h"
#include"FrameWork/UsersInput.h"
#include"../OperationConfig.h"

SplineParticle::SplineParticle(std::shared_ptr<KuroEngine::RWStructuredBuffer> particle) :
	m_gpuParticleBuffer(particle), m_splineTimer(10)
{
	m_particleBuffer = KuroEngine::D3D12App::Instance()->GenerateRWStructuredBuffer(sizeof(SplineData), 1024);
	m_limitIndexPosBuffer = KuroEngine::D3D12App::Instance()->GenerateRWStructuredBuffer(sizeof(DirectX::XMFLOAT3), LIMIT_POS_MAX);

	m_scaleRotaBuffer = KuroEngine::D3D12App::Instance()->GenerateConstantBuffer(sizeof(ConstData), 1);
	m_limitIndexBuffer = KuroEngine::D3D12App::Instance()->GenerateConstantBuffer(sizeof(UINT), 1);


	std::vector<KuroEngine::RootParam>rootParam =
	{
		KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"スプラインパーティクルの情報(RWStructuredBuffer)"),
		KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"制御点の座標(RWStructuredBuffer)"),
		KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"制御点の最大値(RWStructuredBuffer)")
	};
	auto cs_init = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/SplineParticle.hlsl", "SplineInitMain", "cs_6_4");
	m_initLoucusPipeline = KuroEngine::D3D12App::Instance()->GenerateComputePipeline(cs_init, rootParam, { KuroEngine::WrappedSampler(true,true) });

	auto cs_update = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/SplineParticle.hlsl", "SplineUpdateMain", "cs_6_4");
	m_updateLoucusPipeline = KuroEngine::D3D12App::Instance()->GenerateComputePipeline(cs_update, rootParam, { KuroEngine::WrappedSampler(true,true) });

	m_initFlag = false;
}

void SplineParticle::Init(std::vector<KuroEngine::Vec3<float>>posArray)
{
	m_posArray.clear();
	m_posArray.shrink_to_fit();
	for (int i = 0; i < posArray.size(); ++i)
	{
		m_posArray.emplace_back(posArray[i].x, posArray[i].y, posArray[i].z);
	}
	UINT num = static_cast<UINT>(m_posArray.size());
	m_limitIndexBuffer->Mapping(&num);
	m_limitIndexPosBuffer->Mapping(m_posArray.data());


	std::vector<KuroEngine::RegisterDescriptorData>descData =
	{
		{m_particleBuffer,KuroEngine::UAV},
		{m_limitIndexPosBuffer,KuroEngine::UAV},
		{m_limitIndexBuffer,KuroEngine::CBV},
	};
	KuroEngine::D3D12App::Instance()->DispathOneShot(m_initLoucusPipeline, { 1,1,1 }, descData);

	cd.startIndex = 0;
	cd.rate = 0.0f;
	m_splineTimer.Reset();
	m_finishFlag = false;
	m_initFlag = true;
}

void SplineParticle::Update()
{
	if (!m_initFlag)
	{
		return;
	}

	if (OperationConfig::Instance()->DebugKeyInputOnTrigger(DIK_SPACE))
	{
		std::vector<KuroEngine::RegisterDescriptorData>descData =
		{
			{m_particleBuffer,KuroEngine::UAV},
			{m_limitIndexPosBuffer,KuroEngine::UAV},
			{m_limitIndexBuffer,KuroEngine::CBV},
		};
		KuroEngine::D3D12App::Instance()->DispathOneShot(m_initLoucusPipeline, { 1,1,1 }, descData);
		m_finishFlag = false;
	}

	if (!m_finishFlag)
	{
		cd.scaleRotate = DirectX::XMMatrixScaling(0.3f, 0.3f, 0.3f);
		if (m_splineTimer.IsTimeUp())
		{
			++cd.startIndex;
			m_splineTimer.Reset();
		}
		if (m_posArray.size() <= cd.startIndex)
		{
			cd.startIndex = 0;
			m_finishFlag = true;
			m_initFlag = false;
		}
		cd.rate = m_splineTimer.GetTimeRate();
		m_splineTimer.UpdateTimer();
		m_scaleRotaBuffer->Mapping(&cd);
	}


	std::vector<KuroEngine::RegisterDescriptorData>descData =
	{
		{m_particleBuffer,KuroEngine::UAV},
		{m_gpuParticleBuffer,KuroEngine::UAV},
		{m_scaleRotaBuffer,KuroEngine::CBV},
	};
	KuroEngine::D3D12App::Instance()->DispathOneShot(m_updateLoucusPipeline, { 1,1,1 }, descData);
}
