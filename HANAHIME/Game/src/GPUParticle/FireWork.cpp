#include"FireWork.h"
#include"FrameWork/UsersInput.h"
#include"../OperationConfig.h"

FireWork::FireWork(std::shared_ptr<KuroEngine::RWStructuredBuffer> particle) :m_gpuParticleBuffer(particle)
{
	const int FIREWORK_PARTICLE_MAX = 1024;
	m_fireUploadBuffer = KuroEngine::D3D12App::Instance()->GenerateRWStructuredBuffer(sizeof(FireParticle), FIREWORK_PARTICLE_MAX);

	{
		std::vector<KuroEngine::RootParam>rootParam =
		{
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"花火(RWStructuredBuffer)"),
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"エミッター(ConstBuffer)"),
		};
		auto cs_init = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/FireWork.hlsl", "InitMain", "cs_6_4");
		m_fireWorkInitPipeline = KuroEngine::D3D12App::Instance()->GenerateComputePipeline(cs_init, rootParam, { KuroEngine::WrappedSampler(true,true) });

		std::vector<KuroEngine::RootParam>rootParam2 =
		{
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"花火(RWStructuredBuffer)"),
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"描画(RWStructuredBuffer)")
		};
		auto cs_update = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/FireWork.hlsl", "UpdateMain", "cs_6_4");
		m_fireWorkUpdatePipeline = KuroEngine::D3D12App::Instance()->GenerateComputePipeline(cs_update, rootParam2, { KuroEngine::WrappedSampler(true,true) });
	}

	m_emitterBuffer = KuroEngine::D3D12App::Instance()->GenerateConstantBuffer(sizeof(EmittreData), 1);

}

void FireWork::Init(const KuroEngine::Vec3<float> &emittPos)
{
	m_emitterPos = emittPos;
	EmittreData data;
	data.pos = { emittPos.x,emittPos.y,emittPos.z };
	m_emitterBuffer->Mapping(&data);

	std::vector<KuroEngine::RegisterDescriptorData>descData =
	{
		{m_fireUploadBuffer,KuroEngine::UAV},
		{m_emitterBuffer,KuroEngine::CBV},
	};
	KuroEngine::D3D12App::Instance()->DispathOneShot(m_fireWorkInitPipeline, { 1,1,1 }, descData);
}

void FireWork::Update()
{
	{
		std::vector<KuroEngine::RegisterDescriptorData>descData =
		{
			{m_fireUploadBuffer,KuroEngine::UAV},
			{m_gpuParticleBuffer,KuroEngine::UAV}
		};
		KuroEngine::D3D12App::Instance()->DispathOneShot(m_fireWorkUpdatePipeline, { 1,1,1 }, descData);
	}
}