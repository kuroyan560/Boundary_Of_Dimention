#include "GPUParticleRender.h"
#include"DirectX12/D3D12App.h"
#include"DirectX12/D3D12Data.h"
#include<Render/RenderObject/SpriteMesh.h>
#include"Render/GraphicsManager.h"

GPUParticleRender::GPUParticleRender(int MAXNUM)
{

	particleMaxNum = MAXNUM;

	//worldMatHandle = computeCovertWorldMatToDrawMat.CreateBuffer(
	//	KazBufferHelper::SetOnlyReadStructuredBuffer(sizeof(InputData) * particleMaxNum),
	//	GRAPHICS_RANGE_TYPE_UAV_DESC,
	//	GRAPHICS_PRAMTYPE_DATA,
	//	sizeof(InputData),
	//	particleMaxNum,
	//	true);

	//outputHandle = computeCovertWorldMatToDrawMat.CreateBuffer(
	//	KazBufferHelper::SetOnlyReadStructuredBuffer(sizeof(OutputData) * particleMaxNum),
	//	GRAPHICS_RANGE_TYPE_UAV_DESC,
	//	GRAPHICS_PRAMTYPE_DATA2,
	//	sizeof(OutputData),
	//	particleMaxNum,
	//	true);

	//viewProjMatHandle = computeCovertWorldMatToDrawMat.CreateBuffer(
	//	sizeof(DirectX::XMMATRIX),
	//	GRAPHICS_RANGE_TYPE_CBV_VIEW,
	//	GRAPHICS_PRAMTYPE_DATA3,
	//	1,
	//	false);






	//std::array<Vertex, 4>lVerticesArray;
	//std::array<USHORT, 6> lIndicesArray;
	//lIndicesArray = KazRenderHelper::InitIndciesForPlanePolygon();
	//KazRenderHelper::InitVerticesPos(&lVerticesArray[0].pos, &lVerticesArray[1].pos, &lVerticesArray[2].pos, &lVerticesArray[3].pos, { 0.5f,0.5f });
	//KazRenderHelper::InitUvPos(&lVerticesArray[0].uv, &lVerticesArray[1].uv, &lVerticesArray[2].uv, &lVerticesArray[3].uv);

	//KazGPUParticle::BUFFER_SIZE lVertBuffSize = KazBufferHelper::GetBufferSize<KazGPUParticle::BUFFER_SIZE>(lVerticesArray.size(), sizeof(Vertex));
	//KazGPUParticle::BUFFER_SIZE lIndexBuffSize = KazBufferHelper::GetBufferSize<KazGPUParticle::BUFFER_SIZE>(lIndicesArray.size(), sizeof(UINT));


	//vertexBuffer = std::make_unique<KazRenderHelper::ID3D12ResourceWrapper>();
	//indexBuffer = std::make_unique<KazRenderHelper::ID3D12ResourceWrapper>();


	//vertexBuffer->CreateBuffer(KazBufferHelper::SetVertexBufferData(lVertBuffSize));
	//indexBuffer->CreateBuffer(KazBufferHelper::SetIndexBufferData(lIndexBuffSize));

	//gpuVertexBuffer.CreateBuffer(KazBufferHelper::SetGPUBufferData(lVertBuffSize));
	//gpuIndexBuffer.CreateBuffer(KazBufferHelper::SetGPUBufferData(lIndexBuffSize));

	//vertexBuffer->TransData(lVerticesArray.data(), lVertBuffSize);
	//indexBuffer->TransData(lIndicesArray.data(), lIndexBuffSize);

	//gpuVertexBuffer.CopyBuffer(vertexBuffer->GetBuffer(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);
	//gpuIndexBuffer.CopyBuffer(indexBuffer->GetBuffer(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);


/*
	UINT lNum = 0;
	KazBufferHelper::BufferResourceData lBufferData
	(
		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		"CopyCounterBuffer"
	);

	copyBuffer.CreateBuffer(lBufferData);
	copyBuffer.TransData(&lNum, sizeof(UINT))*/;


	//std::shared_ptr<KuroEngine::IndirectCommandBuffer> indirectBuff;
	//std::shared_ptr<KuroEngine::IndirectDevice> indirectDivece;
	//std::shared_ptr<KuroEngine::IndexBuffer> indexBuffer;
	//KuroEngine::SpriteMesh meshData;
	//KuroEngine::GraphicsManager::ExcuteIndirectCommand excuteIndrect(
	//	{}, indirectDivece,
	//	0,
	//	meshData, indexBuffer);



	//描画情報の変換処理----------------------------------------

	//蛍の配列情報
	KuroEngine::D3D12App::Instance()->GenerateRWStructuredBuffer(
		&m_fireFlyArrayBuffer, &m_fireFlyCounterBuffer,
		sizeof(FireFlyData),
		particleMaxNum,
		nullptr,
		"FireFlyParticleData - RWStructuredBuffer");

	//蛍の描画情報
	KuroEngine::D3D12App::Instance()->GenerateRWStructuredBuffer(
		&m_fireFlyDrawBuffer, &m_fireFlyDrawCounterBuffer,
		sizeof(DrawData),
		particleMaxNum,
		nullptr,
		"FireFlyDrawData - RWStructuredBuffer");

	//ビュープロジェクション行列
	m_viewPorjBuffer = KuroEngine::D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(DirectX::XMMATRIX),
		1,
		nullptr,
		"viewProjBuffer - ConstBuffer");

	std::vector<KuroEngine::RootParam>rootParam =
	{
		KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"蛍パーティクルの情報(RWStructuredBuffer)"),
		KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"描画用の情報(RWStructuredBuffer)"),
		KuroEngine::RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"描画の共通情報(RWStructuredBuffer)")
	};
	auto cs_init = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/CurlingParticle.hlsl", "CSmain", "cs_6_4");
	m_cPipeline = KuroEngine::D3D12App::Instance()->GenerateComputePipeline(cs_init, rootParam, { KuroEngine::WrappedSampler(true,true) });

	//描画情報の変換処理----------------------------------------

	//パイプラインの生成----------------------------------------
	KuroEngine::PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	PIPELINE_OPTION.m_depthTest = false;

	std::vector<KuroEngine::RootParam> graphicRootParam =
	{
		KuroEngine::RootParam(KuroEngine::UAV,"蛍パーティクルの描画情報(RWStructuredBuffer)"),
	};
	std::vector<D3D12_STATIC_SAMPLER_DESC>sampler;
	sampler.emplace_back(KuroEngine::WrappedSampler(true, false));

	KuroEngine::Shaders SHADERS;
	SHADERS.m_vs = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/DrawInstancing.hlsl", "VSmain", "vs_6_4");
	SHADERS.m_ps = KuroEngine::D3D12App::Instance()->CompileShader("resource/user/shaders/DrawInstancing.hlsl", "PSmain", "ps_6_4");

	std::vector<KuroEngine::RenderTargetInfo>RENDER_TARGET_INFO = { KuroEngine::RenderTargetInfo(KuroEngine::D3D12App::Instance()->GetBackBuffFormat(), KuroEngine::AlphaBlendMode_None) };

	m_gPipeline = KuroEngine::D3D12App::Instance()->GenerateGraphicsPipeline
	(
		PIPELINE_OPTION,
		SHADERS,
		KuroEngine::SpriteMesh::Vertex::GetInputLayout(),
		graphicRootParam,
		RENDER_TARGET_INFO,
		{ KuroEngine::WrappedSampler(true, false) }
	);

	rootsignature = KuroEngine::D3D12App::Instance()->GenerateRootSignature(graphicRootParam, sampler);
	//パイプラインの生成----------------------------------------

	//ExcuteIndirect----------------------------------------
	std::array<Vertex, 4>verticesArray;
	InitVerticesPos(&verticesArray[0].pos, &verticesArray[1].pos, &verticesArray[2].pos, &verticesArray[3].pos, { 0.5f,0.5f });
	m_particleVertex = KuroEngine::D3D12App::Instance()->GenerateVertexBuffer(sizeof(Vertex), static_cast<int>(verticesArray.size()), verticesArray.data());

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
	lInitData.rootsignature = rootsignature;

	std::array<D3D12_INDIRECT_ARGUMENT_DESC, 2> args{};
	args[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW;
	args[0].UnorderedAccessView.RootParameterIndex = 0;
	args[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

	lInitData.argument.push_back(args[0]);
	lInitData.argument.push_back(args[1]);
	excuteIndirect = std::make_unique<DrawExcuteIndirect>(lInitData);
	//ExcuteIndirect----------------------------------------

}

void GPUParticleRender::InitCount()
{
}

void GPUParticleRender::Draw(KuroEngine::Camera &camera)
{
	//viewProjMat = CameraMgr::Instance()->GetViewMatrix() * CameraMgr::Instance()->GetPerspectiveMatProjection();
	//computeCovertWorldMatToDrawMat.TransData(viewProjMatHandle, &viewProjMat, sizeof(DirectX::XMMATRIX));

	//computeCovertWorldMatToDrawMat.StackToCommandListAndCallDispatch(PIPELINE_COMPUTE_NAME_CONVERT_WORLDMAT_TO_DRAWMAT, { NUM.x,NUM.y,NUM.z });

	//auto plantGrassCount = *m_plantGrassCounterBuffer->GetResource()->GetBuffOnCpu<int>();


	//ここまではエラーなし----------------------------------------
	DirectX::XMMATRIX viewProjMat = camera.GetViewMat() * camera.GetProjectionMat();
	m_viewPorjBuffer->Mapping(&viewProjMat);
	std::vector<KuroEngine::RegisterDescriptorData>descData =
	{
		{m_fireFlyArrayBuffer,KuroEngine::UAV},
		{m_fireFlyDrawBuffer,KuroEngine::UAV},
		{m_viewPorjBuffer,KuroEngine::CBV},
	};
	KuroEngine::D3D12App::Instance()->DispathOneShot(m_cPipeline, { 1000,1,1 }, descData);

	excuteIndirect->Draw(*m_gPipeline, nullptr);

}

std::shared_ptr<KuroEngine::RWStructuredBuffer>GPUParticleRender::GetStackBuffer()const
{
	return m_fireFlyArrayBuffer;
}