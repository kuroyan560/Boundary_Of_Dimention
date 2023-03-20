#include "Grass.h"
#include"DirectX12/D3D12App.h"

Grass::Grass()
{
	using namespace KuroEngine;

	//コンピュートシェーダーのルートパラメータ
	const std::vector<RootParam>rootParam =
	{
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"草シェーダーCBV"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"草シェーダーUAV"),
	};

	//コンピュートシェーダー作成
	std::string hlslPath = "resource/user/shaders/Grass_Compute.hlsl";

	//初期化用コンピュートシェーダー
	m_initComputePipeline = D3D12App::Instance()->GenerateComputePipeline(
		D3D12App::Instance()->CompileShader(hlslPath, "InitMain", "cs_6_4"),
		rootParam,
		{ WrappedSampler(false,false) }
	);

	//更新用コンピュートシェーダー
	m_updateComputePipeline = D3D12App::Instance()->GenerateComputePipeline(
		D3D12App::Instance()->CompileShader(hlslPath, "UpdateMain", "cs_6_4"),
		rootParam,
		{ WrappedSampler(false,false) }
	);

	//定数バッファ生成
	m_constBuffer = D3D12App::Instance()->GenerateConstantBuffer(sizeof(m_constData), 1, &m_constData, "GrassShader - ConstantBuffer");

	//頂点最大
	int maxVertexNum = 1000;
	m_uavDataArray.resize(maxVertexNum);
	//頂点バッファ生成
	D3D12App::Instance()->GenerateVertexBuffer(
		sizeof(UAVdata),
		maxVertexNum,
		m_uavDataArray.data(),
		"GrassShader - VertexBuffer",
		true);
}

void Grass::Init()
{
	using namespace KuroEngine;

	//初期化用コンピュートシェーダー実行
	D3D12App::Instance()->DispathOneShot(
		m_initComputePipeline,
		{ static_cast<int>(m_uavDataArray.size()) / THREAD_PER_NUM + 1,1,1 },
		{
			{m_constBuffer,CBV},
			{m_uavDataBuffer->GetRWStructuredBuff().lock(),UAV},
		});
}

void Grass::Update(float arg_timeScale)
{
	using namespace KuroEngine;

	//タイムスケールに変更があったら更新して送信
	if (m_constData.m_timeScale != arg_timeScale)
	{
		m_constData.m_timeScale = arg_timeScale;
		m_constBuffer->Mapping(&m_constData);
	}

	//更新用コンピュートシェーダー実行
	D3D12App::Instance()->DispathOneShot(
		m_updateComputePipeline,
		{ static_cast<int>(m_uavDataArray.size()) / THREAD_PER_NUM + 1,1,1 },
		{
			{m_constBuffer,CBV},
			{m_uavDataBuffer->GetRWStructuredBuff().lock(),UAV},
		});
}

void Grass::Draw()
{
}
