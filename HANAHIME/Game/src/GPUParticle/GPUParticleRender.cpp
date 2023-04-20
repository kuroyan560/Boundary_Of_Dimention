#include "GPUParticleRender.h"
#include"DirectX12/D3D12App.h"
#include"DirectX12/D3D12Data.h"
#include<Render/RenderObject/SpriteMesh.h>
#include"Render/GraphicsManager.h"
#include"../Graphics/BasicDraw.h"

GPUParticleRender::GPUParticleRender(int MAXNUM)
{
	//�`����̕ϊ�����----------------------------------------

	//�u�̔z����
	KuroEngine::D3D12App::Instance()->GenerateRWStructuredBuffer(
		&m_fireFlyArrayBuffer, &m_fireFlyCounterBuffer,
		sizeof(FireFlyData),
		particleMaxNum,
		nullptr,
		"FireFlyParticleData - RWStructuredBuffer");

	//�u�̕`����
	KuroEngine::D3D12App::Instance()->GenerateRWStructuredBuffer(
		&m_fireFlyDrawBuffer, &m_fireFlyDrawCounterBuffer,
		sizeof(DrawData),
		particleMaxNum,
		nullptr,
		"FireFlyDrawData - RWStructuredBuffer");

	//�r���[�v���W�F�N�V�����s��
	m_viewPorjBuffer = KuroEngine::D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(ViewProjMatData),
		1,
		nullptr,
		"viewProjBuffer - ConstBuffer"
	);

	std::vector<KuroEngine::RootParam>rootParam =
	{
		KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"�u�p�[�e�B�N���̏��(RWStructuredBuffer)"),
		KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"�`��p�̏��(RWStructuredBuffer)"),
		KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"�`��̋��ʏ��(RWStructuredBuffer)")
	};
	auto cs_init = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/CurlingParticle.hlsl", "CSmain", "cs_6_4");
	m_cPipeline = KuroEngine::D3D12App::Instance()->GenerateComputePipeline(cs_init, rootParam, { KuroEngine::WrappedSampler(true,true) });

	//�`����̕ϊ�����----------------------------------------

	//�p�C�v���C���̐���----------------------------------------
	KuroEngine::PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	PIPELINE_OPTION.m_depthTest = true;
	PIPELINE_OPTION.m_depthWriteMask = false;
	PIPELINE_OPTION.m_calling = D3D12_CULL_MODE_NONE;

	std::vector<KuroEngine::RootParam> graphicRootParam =
	{
		KuroEngine::RootParam(KuroEngine::UAV,"�u�p�[�e�B�N���̕`����(RWStructuredBuffer)")
		,KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"�p�[�e�B�N���̃e�N�X�`��")
	};
	std::vector<D3D12_STATIC_SAMPLER_DESC>sampler;
	sampler.emplace_back(KuroEngine::WrappedSampler(true, false));

	KuroEngine::Shaders SHADERS;
	SHADERS.m_vs = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/DrawInstancing.hlsl", "VSmain", "vs_6_4");
	SHADERS.m_ps = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/DrawInstancing.hlsl", "PSmain", "ps_6_4");

	DXGI_FORMAT renderTargetFormat = BasicDraw::Instance()->GetRenderTarget(BasicDraw::MAIN)->GetDesc().Format;
	//DXGI_FORMAT renderTargetFormat = KuroEngine::D3D12App::Instance()->GetBackBuffFormat();
	
	std::vector<KuroEngine::RenderTargetInfo>RENDER_TARGET_INFO;
	RENDER_TARGET_INFO.emplace_back(renderTargetFormat, KuroEngine::AlphaBlendMode_Trans);


	static std::vector<KuroEngine::InputLayoutParam>INPUT_LAYOUT =
	{
		KuroEngine::InputLayoutParam("POSITION",DXGI_FORMAT_R32G32B32_FLOAT),
		KuroEngine::InputLayoutParam("TEXCOORD",DXGI_FORMAT_R32G32_FLOAT)
	};
	m_gPipeline = KuroEngine::D3D12App::Instance()->GenerateGraphicsPipeline
	(
		PIPELINE_OPTION,
		SHADERS,
		INPUT_LAYOUT,
		graphicRootParam,
		RENDER_TARGET_INFO,
		{ KuroEngine::WrappedSampler(true, false) }
	);
	//�p�C�v���C���̐���----------------------------------------




	//ExcuteIndirect----------------------------------------
	std::array<SpriteVertex, 4>verticesArray;
	InitVerticesPos(&verticesArray[0].pos, &verticesArray[1].pos, &verticesArray[2].pos, &verticesArray[3].pos, { 0.5f,0.5f });
	InitUvPos(&verticesArray[0].uv, &verticesArray[1].uv, &verticesArray[2].uv, &verticesArray[3].uv);
	m_particleVertex = KuroEngine::D3D12App::Instance()->GenerateVertexBuffer(sizeof(SpriteVertex), static_cast<int>(verticesArray.size()), verticesArray.data());

	std::array<UINT, 6>result;
	result[0] = 0;
	result[1] = 1;
	result[2] = 2;
	result[3] = 2;
	result[4] = 1;
	result[5] = 3;
	m_particleIndex = KuroEngine::D3D12App::Instance()->GenerateIndexBuffer(static_cast<int>(result.size()), result.data());

	InitDrawIndexedExcuteIndirect lInitData;
	lInitData.particleVertex = m_particleVertex;
	lInitData.particleIndex = m_particleIndex;
	lInitData.indexNum = m_particleIndex->m_indexNum;
	lInitData.elementNum = particleMaxNum;
	lInitData.updateView = m_fireFlyDrawBuffer->GetResource()->GetBuff()->GetGPUVirtualAddress();
	lInitData.rootsignature = m_gPipeline->m_rootSignature;

	std::array<D3D12_INDIRECT_ARGUMENT_DESC, 2> args{};
	args[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW;
	args[0].UnorderedAccessView.RootParameterIndex = 0;
	args[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

	for (int i = 0; i < args.size(); ++i)
	{
		lInitData.argument.push_back(args[i]);
	}
	excuteIndirect = std::make_unique<DrawExcuteIndirect>(lInitData);
	//ExcuteIndirect----------------------------------------
}

void GPUParticleRender::InitCount()
{
	int num = 0;
	m_fireFlyCounterBuffer->Mapping(&num);
	m_fireFlyDrawCounterBuffer->Mapping(&num);
}

void GPUParticleRender::Draw(KuroEngine::Camera &camera)
{
	ViewProjMatData mat;
	mat.viewprojMat = camera.GetViewMat() * camera.GetProjectionMat();
	mat.scaleRotateBillboardMat = DirectX::XMMatrixScaling(10.0f, 10.0f, 10.0f) * camera.GetBillBoardMat();
	m_viewPorjBuffer->Mapping(&mat);

	std::vector<KuroEngine::RegisterDescriptorData>descData =
	{
		{m_fireFlyArrayBuffer,KuroEngine::UAV},
		{m_fireFlyDrawBuffer,KuroEngine::UAV},
		{m_viewPorjBuffer,KuroEngine::CBV},
	};
	KuroEngine::D3D12App::Instance()->DispathOneShot(m_cPipeline, { 1,1,1 }, descData);

	excuteIndirect->Draw(m_gPipeline, nullptr);
}

std::shared_ptr<KuroEngine::RWStructuredBuffer>GPUParticleRender::GetStackBuffer()const
{
	return m_fireFlyArrayBuffer;
}