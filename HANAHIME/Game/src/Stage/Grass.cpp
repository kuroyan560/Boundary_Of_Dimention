#include "Grass.h"
#include"DirectX12/D3D12App.h"
#include"FrameWork/Importer.h"
#include"../Graphics/BasicDraw.h"

Grass::Grass()
{
	using namespace KuroEngine;

	//仮置きの草ブロックモデル
	m_grassBlockModel = Importer::Instance()->LoadModel("resource/user/model/", "GrassBlock.gltf");

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
	m_uavDataBuffer = D3D12App::Instance()->GenerateVertexBuffer(
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

	//ワールド行列配列初期化
	m_grassWorldMatArray.clear();

	m_oldPlayerPos = { -1000,-1000,-1000 };
	m_plantTimer.Reset(0);
}

void Grass::Update(const float arg_timeScale, const KuroEngine::Vec3<float> arg_playerPos, const KuroEngine::Quaternion arg_playerRotate)
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

	//プレイヤーが移動した
	if (!((arg_playerPos - m_oldPlayerPos).Length() < FLT_MIN))
	{
		if (m_plantTimer.IsTimeUp())
		{
			Transform grassTransform;
			grassTransform.SetPos(arg_playerPos);
			grassTransform.SetRotate(arg_playerRotate);
			grassTransform.SetScale({ 1.0f,1.0f,1.0f });
			Plant(grassTransform.GetMatWorld());
			m_plantTimer.Reset(3);
		}
		m_plantTimer.UpdateTimer();
	}

	m_oldPlayerPos = arg_playerPos;
}

void Grass::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	BasicDraw::Instance()->InstancingDraw(
		arg_cam,
		arg_ligMgr,
		m_grassBlockModel,
		m_grassWorldMatArray,
		false,
		KuroEngine::AlphaBlendMode_Trans);
}

void Grass::Plant(KuroEngine::Matrix arg_worldMat)
{
	m_grassWorldMatArray.push_back(arg_worldMat);
}