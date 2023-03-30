#include "Grass.h"
#include"DirectX12/D3D12App.h"
#include"FrameWork/Importer.h"
#include"../Graphics/BasicDraw.h"
#include"../Graphics/WaterPaintBlend.h"
#include"KuroEngineDevice.h"
#include"Render/RenderObject/Camera.h"

Grass::Grass()
{
	using namespace KuroEngine;

	//仮====================================
		//仮置きの草ブロックモデル
	m_grassBlockModel = Importer::Instance()->LoadModel("resource/user/model/", "GrassBlock.gltf");
	//=====================================

	//コンピュートパイプライン生成
	{
		//ルートパラメータ
		std::vector<RootParam>rootParam =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"生成した草のバッファ(RWStructuredBuffer)"),
		};

		//初期化用パイプライン
		auto cs_init = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "Init", "cs_6_4");
		m_cPipeline[INIT] = D3D12App::Instance()->GenerateComputePipeline(cs_init, rootParam, {WrappedSampler(true,true)});

		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "生成する予定のスタックしたイニシャライザ配列バッファー(StructuredBuffer)");
		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "草むら以外のトランスフォームデータ");
		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "全草むらで共通する定数バッファ");

		//生成用パイプライン
		auto cs_appear = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "Appear", "cs_6_4");
		m_cPipeline[GENERATE] = D3D12App::Instance()->GenerateComputePipeline(cs_appear, rootParam, {WrappedSampler(true,true)});

		//更新用パイプライン
		auto cs_update = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "Update", "cs_6_4");
		m_cPipeline[UPDATE] = D3D12App::Instance()->GenerateComputePipeline(cs_update, rootParam, {WrappedSampler(true,true)});

		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, "判定の結果を格納するバッファ(RWStructuredBuffer)");
		//判定用パイプライン
		auto cs_check = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "Check", "cs_6_4");
		m_cPipeline[CHECK_AROUND] = D3D12App::Instance()->GenerateComputePipeline(cs_check, rootParam, { WrappedSampler(true,true) });
	}

	//描画用グラフィックスパイプラインパイプライン生成
	{
		//パイプライン設定
		PipelineInitializeOption pipelineOption(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		//シェーダー情報
		Shaders shaders;
		shaders.m_vs = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass_Draw.hlsl", "VSmain", "vs_6_4");
		shaders.m_gs = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass_Draw.hlsl", "GSmain", "gs_6_4");
		shaders.m_ps = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass_Draw.hlsl", "PSmain", "ps_6_4");

		//ルートパラメータ
		std::vector<RootParam>rootParam =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"生成した草のバッファー(RWStructuredBuffer)"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"カメラ情報"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"全草むらで共通する定数バッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "草むら以外のトランスフォームデータ"),
		};
		//テクスチャバッファ用ルートパラメータ設定
		for (int texIdx = 0; texIdx < s_textureNumMax; ++texIdx)
		{
			rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "テクスチャ情報");
		}

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>renderTargetInfo =
		{
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_Trans),	//通常描画
			RenderTargetInfo(DXGI_FORMAT_R32G32B32A32_FLOAT, AlphaBlendMode_Trans),	//エミッシブマップ
			RenderTargetInfo(DXGI_FORMAT_R16_FLOAT, AlphaBlendMode_None),	//深度マップ
			RenderTargetInfo(DXGI_FORMAT_R16G16B16A16_FLOAT, AlphaBlendMode_None),	//ノーマルマップ
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_None),	//エッジカラーマップ
		};

		//設定を基にパイプライン生成
		m_pipeline = D3D12App::Instance()->GenerateGraphicsPipeline(
			pipelineOption,
			shaders,
			{
				{InputLayoutParam("POSITION",DXGI_FORMAT_R32G32B32_FLOAT) },
			},
			rootParam,
			renderTargetInfo,
			{ WrappedSampler(true,true) });
	}

	//頂点バッファ
	Vec3<float>vertex = { 0,0,0 };
	m_vertBuffer = D3D12App::Instance()->GenerateVertexBuffer(
		sizeof(vertex),
		1,
		&vertex,
		"Grass - VertexBuffer");

	//プレイヤーのトランスフォーム情報用定数バッファ
	m_otherTransformConstBuffer = D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(TransformCBVData),
		1,
		nullptr,
		"Grass - PlayerTransform - ConstantBuffer");

	//行列以外のデータ用構造体
	m_constBuffer = D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(CBVdata),
		1,
		&m_constData,
		"Grass - Common - ConstantBuffer");

	//生成予定の草むらのイニシャライザをスタックしておくバッファ
	m_stackGrassInitializerBuffer = D3D12App::Instance()->GenerateStructuredBuffer(
		sizeof(GrassInitializer),
		GENERATE_MAX_ONCE,
		nullptr,
		"Grass - InitializerArray - StructuredBuffer");

	//生成した草むらのバッファ
	D3D12App::Instance()->GenerateRWStructuredBuffer(
		&m_plantGrassBuffer, &m_plantGrassCounterBuffer,
		sizeof(PlantGrass),
		m_plantGrassMax,
		nullptr,
		"Grass - PlantGrass - RWStructuredBuffer");

	//判定結果の格納用バッファ
	CheckResult checkResultInit;
	m_checkResultBuffer = D3D12App::Instance()->GenerateRWStructuredBuffer(
		sizeof(CheckResult),
		1,
		&checkResultInit,
		"Grass - CheckResult - RWStructuredBuffer");

	//テクスチャ
	m_texBuffer[0] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter01.png");
	m_texBuffer[1] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter02.png");
	m_texBuffer[2] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter03.png");
	m_texBuffer[3] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter04.png");
	m_texBuffer[4] = D3D12App::Instance()->GenerateTextureBuffer("resource/user/imposter/Imposter01.png");
}

void Grass::Init()
{
	using namespace KuroEngine;

	//植えた草のカウント取得
	auto plantGrassCount = *m_plantGrassCounterBuffer->GetResource()->GetBuffOnCpu<int>();

	//草むらの初期化（全消し）
	if (plantGrassCount)
	{
		D3D12App::Instance()->DispathOneShot(
			m_cPipeline[INIT],
			{ plantGrassCount,1,1 },
			{
				{m_plantGrassBuffer,UAV},
			});
	}

	//ワールド行列配列初期化
	m_grassWorldMatArray.clear();

	m_oldPlayerPos = { -1000,-1000,-1000 };
	m_plantTimer.Reset(0);
}

void Grass::Update(const float arg_timeScale, const KuroEngine::Vec3<float> arg_playerPos, const KuroEngine::Quaternion arg_playerRotate, KuroEngine::Transform arg_camTransform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend)
{
	using namespace KuroEngine;

	//トランスフォームに流し込む
	Transform grassTransform;
	grassTransform.SetPos(arg_playerPos);
	grassTransform.SetRotate(arg_playerRotate);
	grassTransform.SetScale({ 1.0f,1.0f,1.0f });

	//トランスフォーム情報をGPUに送信
	TransformCBVData transformData;
	transformData.m_playerPos = arg_playerPos;
	transformData.m_playerUp = grassTransform.GetUp();
	transformData.m_camPos = { arg_camTransform.GetMatWorld().r[3].m128_f32[0],arg_camTransform.GetMatWorld().r[3].m128_f32[1],arg_camTransform.GetMatWorld().r[3].m128_f32[2] };
	m_otherTransformConstBuffer->Mapping(&transformData);

	//プレイヤーが移動した and 周りに草がない。
	bool isMovePlayer = !((arg_playerPos - m_oldPlayerPos).Length() < 0.1f);
	if (isMovePlayer && !IsGrassAround(arg_playerPos))
	{
		if (m_plantTimer.IsTimeUp())
		{
			PlantGrassBlock(grassTransform, arg_grassPosScatter, arg_waterPaintBlend);
			m_plantTimer.Reset(3);
		}
		m_plantTimer.UpdateTimer();
	}

	m_oldPlayerPos = arg_playerPos;

	//登録するディスクリプタの情報配列
	std::vector<RegisterDescriptorData>descData =
	{
		{m_plantGrassBuffer,UAV},
		{m_stackGrassInitializerBuffer,SRV},
		{m_otherTransformConstBuffer,CBV},
		{m_constBuffer,CBV},
	};

	//植えた草むらのカウントのポインタ取得
	auto plantGrassCountPtr = m_plantGrassCounterBuffer->GetResource()->GetBuffOnCpu<int>();

	//植えた草むらの更新
	if (*plantGrassCountPtr)
	{
		D3D12App::Instance()->DispathOneShot(
			m_cPipeline[UPDATE],
			{ *plantGrassCountPtr,1,1 },
			descData);
	}

	//スタックしておいた草むらを生やす
	if (!m_grassInitializerArray.empty())
	{
		//一度に生成できる量を超えてる
		int generateNum = static_cast<int>(m_grassInitializerArray.size());
		if (GENERATE_MAX_ONCE < generateNum)
		{
			AppearMessageBox("Grass : Update() 失敗", "一度に生成できる量を超えてるよ");
			exit(1);
		}

		//イニシャライザ配列を送信
		m_stackGrassInitializerBuffer->Mapping(m_grassInitializerArray.data(), generateNum);

		//生成
		D3D12App::Instance()->DispathOneShot(
			m_cPipeline[GENERATE],
			{ generateNum,1,1 },
			descData);

		//上限到達
		if (m_plantGrassMax < *plantGrassCountPtr)
		{
			AppearMessageBox("Grass : Update() 失敗", "生成できる上限を超えたよ");
			exit(1);
		}

		//スタックしたイニシャライザをリセット
		m_grassInitializerArray.clear();
	}

	//定数バッファ1の草の揺れ具合を更新。
	m_constData.m_sineWave += 0.02f;
	//定数バッファ1をGPUに転送。
	m_constBuffer->Mapping(&m_constData);

}

void Grass::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	if (s_instanceMax <= static_cast<int>(m_grassWorldMatArray.size()))
	{
		KuroEngine::AppearMessageBox("Grass : Draw() 失敗", "インスタンスの上限超えちゃった");
		exit(1);
	}
	if (m_grassWorldMatArray.empty())return;

	using namespace KuroEngine;

	KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(m_pipeline);

	std::vector<RegisterDescriptorData>descData =
	{
			{m_plantGrassBuffer,UAV},
			{arg_cam.GetBuff(),CBV},
			{m_constBuffer,CBV},
			{m_otherTransformConstBuffer,CBV},
	};
	//テクスチャ情報もセット
	for (int texIdx = 0; texIdx < s_textureNumMax; ++texIdx)descData.emplace_back(m_texBuffer[texIdx], SRV);

	//植えた草むらのカウント取得
	int plantGrassCount = *m_plantGrassCounterBuffer->GetResource()->GetBuffOnCpu<int>();
	KuroEngineDevice::Instance()->Graphics().ObjectRender(
		m_vertBuffer,
		descData,
		0.0f,
		true,
		plantGrassCount);

	m_drawParam = IndividualDrawParameter::GetDefault();
	//マスクレイヤーに描き込む設定にする
	m_drawParam.m_drawMask = 1;

	//BasicDraw::Instance()->InstancingDraw(
	//	arg_cam,
	//	arg_ligMgr,
	//	m_grassBlockModel,
	//	m_grassWorldMatArray,
	//	m_drawParam,
	//	false,
	//	AlphaBlendMode_Trans);
}

void Grass::PlantGrassBlock(KuroEngine::Transform arg_transform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend)
{
	//円柱草を生やす処理
	m_grassWorldMatArray.push_back(arg_transform.GetMatWorld());

	//板ポリを生成。
	for (int count = 0; count < 3; ++count) {
			Plant(arg_transform, arg_grassPosScatter, arg_waterPaintBlend);
	}
}

void Grass::Plant(KuroEngine::Transform arg_transform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend)
{
	KuroEngine::Vec3<float> pos = arg_transform.GetPos();

	//草を生やす位置をランダムで散らす。
	KuroEngine::Vec3<float>posScatter = arg_transform.GetRight() * KuroEngine::GetRand(-arg_grassPosScatter.x, arg_grassPosScatter.x);
	posScatter += arg_transform.GetFront() * KuroEngine::GetRand(-arg_grassPosScatter.y, arg_grassPosScatter.y);

	//イニシャライザのスタック
	m_grassInitializerArray.emplace_back();
	m_grassInitializerArray.back().m_posScatter = posScatter;
	//とりあえず乱数でテクスチャ決定
	//m_vertices[m_deadVertexIdx].m_texIdx = KuroEngine::GetRand(s_textureNumMax - 1);
	m_grassInitializerArray.back().m_texIdx = KuroEngine::GetRand(3 - 1);
	m_grassInitializerArray.back().m_sineLength = KuroEngine::GetRand(40) / 100.0f;

	arg_waterPaintBlend.DropMaskInk(pos + KuroEngine::Vec3<float>(0.0f, 1.0f, 0.0f));
}

bool Grass::IsGrassAround(const KuroEngine::Vec3<float> arg_playerPos)
{
	using namespace KuroEngine;

	//植えた草むらのカウント取得
	int plantGrassCount = *m_plantGrassCounterBuffer->GetResource()->GetBuffOnCpu<int>();

	//草が生えてないので判定の必要なし
	if (!plantGrassCount)return false;

	auto checkResultPtr = m_checkResultBuffer->GetResource()->GetBuffOnCpu<CheckResult>();

	//判定結果の初期化
	auto initializer = *checkResultPtr;
	initializer.m_aroundGrassCount = 0;
	m_checkResultBuffer->Mapping(&initializer);

	//判定用コンピュートパイプライン実行
	//登録するディスクリプタの情報配列
	std::vector<RegisterDescriptorData>descData =
	{
		{m_plantGrassBuffer,UAV},
		{m_stackGrassInitializerBuffer,SRV},
		{m_otherTransformConstBuffer,CBV},
		{m_constBuffer,CBV},
		{m_checkResultBuffer,UAV}
	};

	//生成してある草むらの数を取得
	D3D12App::Instance()->DispathOneShot(
		m_cPipeline[CHECK_AROUND],
		{ plantGrassCount ,1,1 },
		descData);

	//判定結果の取得
	return 0 < checkResultPtr->m_aroundGrassCount;
}