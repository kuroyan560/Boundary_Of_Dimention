#include"FireWork.h"
#include"FrameWork/UsersInput.h"
#include"../OperationConfig.h"

FireWork::FireWork(std::shared_ptr<KuroEngine::RWStructuredBuffer> particle) :m_gpuParticleBuffer(particle)
{
	m_fireUploadBuffer = KuroEngine::D3D12App::Instance()->GenerateRWStructuredBuffer(sizeof(FireParticle), FIREWORK_PARTICLE_MAX);

	{
		std::vector<KuroEngine::RootParam>rootParam =
		{
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"花火(RWStructuredBuffer)"),
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"乱数テーブル(RWStructuredBuffer)"),
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"パーティクルカラー(ShaderResourceBuffer)"),
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"エミッター(ConstBuffer)")
		};
		auto cs_init = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/FireWork.hlsl", "InitMain", "cs_6_4");

		KuroEngine::WrappedSampler smp(true, true);
		smp.m_sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		m_fireWorkInitPipeline = KuroEngine::D3D12App::Instance()->GenerateComputePipeline(cs_init, rootParam, { smp });

		std::vector<KuroEngine::RootParam>rootParam2 =
		{
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"花火(RWStructuredBuffer)"),
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"描画(RWStructuredBuffer)"),
			KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"パーティクルカラー(ShaderResourceBuffer)")
		};
		auto cs_update = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/FireWork.hlsl", "UpdateMain", "cs_6_5");

		m_fireWorkUpdatePipeline = KuroEngine::D3D12App::Instance()->GenerateComputePipeline(cs_update, rootParam2, { smp });
	}

	m_emitterBuffer = KuroEngine::D3D12App::Instance()->GenerateConstantBuffer(sizeof(EmittreData), 1);

	//乱数テーブル
	m_randomBuffer = KuroEngine::D3D12App::Instance()->GenerateRWStructuredBuffer(sizeof(UINT), 1024);
	std::vector<UINT>randomArray;
	for (int i = 0; i < 1024; ++i)
	{
		randomArray.emplace_back(KuroEngine::GetRand(10000, -10000));
	}
	m_randomBuffer->Mapping(randomArray.data());

	m_particleColor = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/Particle/check_point_particle_gradation.png");
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
		{m_randomBuffer,KuroEngine::UAV},
		{m_particleColor,KuroEngine::SRV},
		{m_emitterBuffer,KuroEngine::CBV}
	};
	KuroEngine::D3D12App::Instance()->DispathOneShot(m_fireWorkInitPipeline, { DISPATCH_NUM,1,1 }, descData);
}

void FireWork::Update()
{
	std::vector<KuroEngine::RegisterDescriptorData>descData =
	{
		{m_fireUploadBuffer,KuroEngine::UAV},
		{m_gpuParticleBuffer,KuroEngine::UAV},
		{m_particleColor,KuroEngine::SRV}
	};
	KuroEngine::D3D12App::Instance()->DispathOneShot(m_fireWorkUpdatePipeline, { DISPATCH_NUM,1,1 }, descData);
}