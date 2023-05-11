#include "Grass.h"
#include"DirectX12/D3D12App.h"
#include"FrameWork/Importer.h"
#include"../Graphics/BasicDraw.h"
#include"../Graphics/WaterPaintBlend.h"
#include"../../../../src/engine/ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"KuroEngineDevice.h"
#include"Render/RenderObject/Camera.h"
#include"../Player/CollisionDetectionOfRayAndMesh.h"

Grass::Grass()
{
	using namespace KuroEngine;

	//コンピュートパイプライン生成
	{
		//ルートパラメータ
		std::vector<RootParam>rootParam =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"生成した草のバッファ"),
		};

		//初期化用パイプライン
		auto cs_init = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "Init", "cs_6_4");
		m_cPipeline[INIT] = D3D12App::Instance()->GenerateComputePipeline(cs_init, rootParam, { WrappedSampler(true,true) });

		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "生成する予定のスタックしたイニシャライザ配列バッファー(StructuredBuffer)");

		//生成用パイプライン
		auto cs_appear = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "Appear", "cs_6_4");
		m_cPipeline[APPEAR] = D3D12App::Instance()->GenerateComputePipeline(cs_appear, rootParam, { WrappedSampler(true,true) });

		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "ワールド座標");
		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "法線マップ");
		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "光が当たっている範囲のマップ");
		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "草むら以外のトランスフォームデータ");
		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "全草むらで共通する定数バッファ");
		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "削除する草のカウント");

		//削除用パイプライン
		auto cs_disappear = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "Disappear", "cs_6_4");
		m_cPipeline[DISAPPEAR] = D3D12App::Instance()->GenerateComputePipeline(cs_disappear, rootParam, { WrappedSampler(true,true) });

		//更新用パイプライン
		auto cs_update = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "Update", "cs_6_4");
		m_cPipeline[UPDATE] = D3D12App::Instance()->GenerateComputePipeline(cs_update, rootParam, { WrappedSampler(true,true) });

		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, "判定の結果を格納するバッファ(RWStructuredBuffer)");
		//判定用パイプライン
		auto cs_check = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass.hlsl", "SearchPlantPos", "cs_6_4");
		m_cPipeline[SEARCH_PLANT_POS] = D3D12App::Instance()->GenerateComputePipeline(cs_check, rootParam, { WrappedSampler(true,true) });
	}

	//描画用グラフィックスパイプラインパイプライン生成
	{
		//パイプライン設定
		PipelineInitializeOption pipelineOption(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//シェーダー情報
		Shaders shaders;
		shaders.m_vs = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass_Draw.hlsl", "VSmain", "vs_6_4");
		shaders.m_ps = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass_Draw.hlsl", "PSmain", "ps_6_4");

		//ルートパラメータ
		std::vector<RootParam>rootParam =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"生成した草のバッファー(RWStructuredBuffer)"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"カメラ情報"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"全草むらで共通する定数バッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "草むら以外のトランスフォームデータ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "テクスチャ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "マテリアル"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "草むらのインデックス配列"),
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>renderTargetInfo =
		{
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_Trans),	//通常描画
			RenderTargetInfo(DXGI_FORMAT_R32G32B32A32_FLOAT, AlphaBlendMode_Trans),	//エミッシブマップ
			RenderTargetInfo(DXGI_FORMAT_R16_FLOAT, AlphaBlendMode_None),	//深度マップ
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_None),	//エッジカラーマップ
		};

		//設定を基にパイプライン生成
		m_pipeline = D3D12App::Instance()->GenerateGraphicsPipeline(
			pipelineOption,
			shaders,
			ModelMesh::Vertex::GetInputLayout(),
			rootParam,
			renderTargetInfo,
			{ WrappedSampler(true,true) });
	}

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

	//ソートと削除処理で使うunsigned int のバッファー
	int initZero = 0;
	m_consumeCountBuffer = D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(int),
		1,
		&initZero,
		"Grass - ConsumeCount - ConstantBuffer"
	);

	//判定結果の格納用バッファ
	std::array<CheckResult, GRASSF_SEARCH_COUNT> checkResultInit;
	m_checkResultBuffer = D3D12App::Instance()->GenerateRWStructuredBuffer(
		sizeof(CheckResult),
		GRASSF_SEARCH_COUNT,
		checkResultInit.data(),
		"Grass - CheckResult - RWStructuredBuffer");

	//モデル
	for (int modelIdx = 0; modelIdx < s_modelNumMax; ++modelIdx)
	{
		std::string fileName = "Grass_" + std::to_string(modelIdx) + ".glb";
		m_modelArray[modelIdx] = Importer::Instance()->LoadModel("resource/user/model/", fileName);

		//草のインデックス配列バッファ
		m_grassIndiciesBuffer[modelIdx] = D3D12App::Instance()->GenerateStructuredBuffer(
			sizeof(int),
			m_plantGrassMax,
			nullptr,
			"Grass - GrassIndicies - StructuredBuffer");

		m_grassIndicies[modelIdx].reserve(m_plantGrassMax);
	}
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

	m_oldPlayerPos = { -1000,-1000,-1000 };
	m_plantTimer.Reset(0);
}

void Grass::Update(const float arg_timeScale, const KuroEngine::Transform arg_playerTransform, std::weak_ptr<KuroEngine::Camera> arg_cam, float arg_plantInfluenceRange, const std::weak_ptr<Stage>arg_nowStage, bool arg_isAttack)
{
	using namespace KuroEngine;

	//トランスフォーム情報をGPUに送信
	TransformCBVData transformData;
	transformData.m_playerPos = arg_playerTransform.GetPos();
	transformData.m_playerPlantLightRange = arg_plantInfluenceRange;
	transformData.m_camPos = { arg_cam.lock()->GetTransform().GetMatWorld().r[3].m128_f32[0],arg_cam.lock()->GetTransform().GetMatWorld().r[3].m128_f32[1],arg_cam.lock()->GetTransform().GetMatWorld().r[3].m128_f32[2] };
	m_otherTransformConstBuffer->Mapping(&transformData);

	//プレイヤーの座標を保存。
	m_playerPos = arg_playerTransform.GetPos();

	//if (m_plantTimer.IsTimeUp() && 0.01f < KuroEngine::Vec3<float>(m_oldPlayerPos - arg_playerTransform.GetPos()).Length())
	if (true)
	{
		//トランスフォームに流し込む
		Transform grassTransform;
		grassTransform.SetPos(arg_playerTransform.GetPos());
		grassTransform.SetRotate(arg_playerTransform.GetRotate());
		grassTransform.SetScale({ 1.0f,1.0f,1.0f });

		Plant(grassTransform, arg_playerTransform);
		m_plantTimer.Reset(0);
	}
	m_plantTimer.UpdateTimer();

	m_oldPlayerPos = arg_playerTransform.GetPos();

	//定数バッファ1の草の揺れ具合を更新。
	m_constData.m_sineWave += 0.02f;
	//座標を保存。
	m_constData.m_playerPos = arg_playerTransform.GetPos() + arg_playerTransform.GetUp() * arg_playerTransform.GetScale().y;
	//登場速度を設定。
	m_constData.m_appearEaseSpeed = 0.1f;
	//プレイヤーの座標を取得。
	m_constData.m_playerPos = arg_playerTransform.GetPos();
	//カメラの情報を入れる。
	m_constData.matView = arg_cam.lock()->GetViewMat();
	m_constData.matProjection = arg_cam.lock()->GetProjectionMat();
	m_constData.eye = arg_cam.lock()->GetEye();
	//定数バッファ1をGPUに転送。
	m_constBuffer->Mapping(&m_constData);

	//登録するディスクリプタの情報配列
	std::vector<RegisterDescriptorData>descData =
	{
		{m_plantGrassBuffer,UAV},
		{m_stackGrassInitializerBuffer, SRV},
	};

	//植えた草むらの配列のポインタ取得
	auto aliveGrassArrayBufferPtr = m_plantGrassBuffer->GetResource()->GetBuffOnCpu<PlantGrass>();
	//植えた草むらのカウントのポインタ取得
	auto plantGrassCountPtr = m_plantGrassCounterBuffer->GetResource()->GetBuffOnCpu<int>();

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
			m_cPipeline[APPEAR],
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

	//植えた草むらの更新
	if (*plantGrassCountPtr)
	{

		descData.emplace_back(BasicDraw::Instance()->GetRenderTarget(BasicDraw::WORLD_POS), SRV);
		descData.emplace_back(BasicDraw::Instance()->GetRenderTarget(BasicDraw::NORMAL_GRASS), SRV);
		descData.emplace_back(BasicDraw::Instance()->GetRenderTarget(BasicDraw::BRIGHT), SRV);
		descData.emplace_back(m_otherTransformConstBuffer, CBV);
		descData.emplace_back(m_constBuffer, CBV);
		descData.emplace_back(m_consumeCountBuffer, CBV);

		D3D12App::Instance()->DispathOneShot(
			m_cPipeline[UPDATE],
			{ *plantGrassCountPtr,1,1 },
			descData);

		//下方向にレイを飛ばして、そこが動く足場だったら親子関係を結ぶ。
		for (int grassIdx = 0; grassIdx < *plantGrassCountPtr;++grassIdx) 
		{
			auto& index = aliveGrassArrayBufferPtr[grassIdx];

			if (!index.m_isAlive)continue;
			if (index.m_isCheckGround) continue;

			int terrianIdx = 0;
			for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
			{
				//動く足場でない
				if (terrian->GetType() != StageParts::MOVE_SCAFFOLD)continue;

				//動く足場としてキャスト
				auto moveScaffold = dynamic_pointer_cast<MoveScaffold>(terrian);
				//モデル情報取得
				auto model = terrian->GetModel();

				//メッシュを走査
				for (auto& modelMesh : model.lock()->m_meshes)
				{

					//当たり判定用メッシュ
					auto checkHitMesh = moveScaffold->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

					CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(index.m_worldPos, -index.m_normal, checkHitMesh);

					//当たっていたら
					if (output.m_isHit && 0 < output.m_distance && output.m_distance <= 1.0f) {

						//親子関係を持たせる。
						index.m_terrianIdx = terrianIdx;

					}


				}
				++terrianIdx;
			}
			index.m_isCheckGround = true;

			//草にトランスフォームを適応。
			//Indexが-1だったら処理を飛ばす。
			if (index.m_terrianIdx == -1) continue;

			auto terrian = arg_nowStage.lock()->GetGimmickArray().begin();
			std::advance(terrian, index.m_terrianIdx);

			index.m_worldPos += terrian->get()->GetMoveAmount();

			//原点付近に生える草は削除
			//攻撃中で、既定の範囲内だったらAppearYを超でかくする。
			if (arg_isAttack && (arg_playerTransform.GetPos() - index.m_worldPos).Length() < arg_plantInfluenceRange) {
				index.m_appearY = 10.0f;
			}

			if (5.0f <= index.m_worldPos.Length()) continue;
			index.m_isAlive = false;
		}

		//GPU上でソートしたら草消えるバグ置きたのでCPU側でソート。動けば勝ちや！！！！！！！！！！！
		std::vector<PlantGrass>aliveGrassArray;
		//GPU上の草データをvectorにいれる。
		aliveGrassArray.resize(*plantGrassCountPtr);
		std::memcpy(aliveGrassArray.data(), aliveGrassArrayBufferPtr, sizeof(PlantGrass)* (*plantGrassCountPtr));

		//削除する数のカウント
		int consumeCount = static_cast<int>(std::count_if(aliveGrassArray.begin(), aliveGrassArray.end(), [](PlantGrass& grass)
			{
				return grass.m_isAlive == 0;
			}));

		//フラグがfalseになった草を最後尾へソート
		std::sort(aliveGrassArray.begin(), aliveGrassArray.end(), [](PlantGrass& a, PlantGrass& b) {
			return a.m_isAlive > b.m_isAlive;
			});

		//GPUに送信
		m_plantGrassBuffer->Mapping(aliveGrassArray.data(), *plantGrassCountPtr);
		m_consumeCountBuffer->Mapping(&consumeCount);

		//死んでいるものを削除
		D3D12App::Instance()->DispathOneShot(
			m_cPipeline[DISAPPEAR],
			{ 1,1,1 },
			descData);

		//草むらのインデックスの配列
		for (auto& indicies : m_grassIndicies)
		{
			indicies.clear();
		}
		//モデルごとの草のインデックス配列作成
		for (int grassIdx = 0; grassIdx < *plantGrassCountPtr; ++grassIdx)
		{
			auto& grass = aliveGrassArrayBufferPtr[grassIdx];
			m_grassIndicies[grass.m_modelIdx].emplace_back(grassIdx);
		}
		//インデックス配列情報を送信
		for (int modelIdx = 0; modelIdx < s_modelNumMax; ++modelIdx)
		{
			m_grassIndiciesBuffer[modelIdx]->Mapping(m_grassIndicies[modelIdx].data(), static_cast<int>(m_grassIndicies[modelIdx].size()));
		}
	}
}

void Grass::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, float arg_plantInfluenceRange, bool arg_isAttack)
{
	using namespace KuroEngine;

	KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(m_pipeline);

	for (int modelIdx = 0; modelIdx < s_modelNumMax; ++modelIdx)
	{
		for (auto& mesh : m_modelArray[modelIdx]->m_meshes)
		{
			std::vector<RegisterDescriptorData>descData =
			{
				{m_plantGrassBuffer,UAV},
				{arg_cam.GetBuff(),CBV},
				{m_constBuffer,CBV},
				{m_otherTransformConstBuffer,CBV},
				{mesh.material->texBuff[COLOR_TEX],SRV},
				{mesh.material->buff,CBV},
				{m_grassIndiciesBuffer[modelIdx],SRV},
			};

			KuroEngineDevice::Instance()->Graphics().ObjectRender(
				mesh.mesh->vertBuff,
				mesh.mesh->idxBuff,
				descData,
				0,
				true,
				static_cast<int>(m_grassIndicies[modelIdx].size()));
		}
	}
}

void Grass::Plant(KuroEngine::Transform arg_transform, KuroEngine::Transform arg_playerTransform)
{

	//草をはやす場所を取得。
	std::array<Grass::CheckResult, Grass::GRASSF_SEARCH_COUNT> plantData = SearchPlantPos(arg_playerTransform);

	for (int count = 0; count < GRASSF_SEARCH_COUNT; ++count)
	{

		if (plantData[count].m_isSuccess != 1) continue;

		//イニシャライザのスタック
		m_grassInitializerArray.emplace_back();
		m_grassInitializerArray.back().m_pos = plantData[count].m_plantPos;
		m_grassInitializerArray.back().m_up = plantData[count].m_plantNormal;
		//とりあえず乱数でテクスチャ決定
		//m_vertices[m_deadVertexIdx].m_texIdx = KuroEngine::GetRand(s_textureNumMax - 1);
		m_grassInitializerArray.back().m_modelIdx = KuroEngine::GetRand(3 - 1);
		m_grassInitializerArray.back().m_sineLength = KuroEngine::GetRand(40) / 100.0f;
	}

	//インクマスクを落とす
	//arg_waterPaintBlend.DropMaskInk(arg_transform.GetPos() + KuroEngine::Vec3<float>(0.0f, 1.0f, 0.0f));
}

std::array<Grass::CheckResult, Grass::GRASSF_SEARCH_COUNT> Grass::SearchPlantPos(KuroEngine::Transform arg_playerTransform)
{
	using namespace KuroEngine;

	//植えた草むらのカウント取得
	int plantGrassCount = *m_plantGrassCounterBuffer->GetResource()->GetBuffOnCpu<int>();

	//判定結果の初期化
	auto checkResultPtr = m_checkResultBuffer->GetResource()->GetBuffOnCpu<CheckResult>();
	checkResultPtr->m_plantPos = KuroEngine::Vec3<float>();

	//必要なデータを送信
	auto transformCBVPtr = m_otherTransformConstBuffer->GetResource()->GetBuffOnCpu<TransformCBVData>();
	transformCBVPtr->m_seed = KuroEngine::GetRand(0, 100000) / 100.0f;
	transformCBVPtr->m_grassCount = plantGrassCount;
	transformCBVPtr->m_playerPos = arg_playerTransform.GetPosWorld();

	//判定用コンピュートパイプライン実行
	//登録するディスクリプタの情報配列
	std::vector<RegisterDescriptorData>descData =
	{
		{m_plantGrassBuffer,UAV},
		{m_stackGrassInitializerBuffer,SRV},
		{BasicDraw::Instance()->GetRenderTarget(BasicDraw::WORLD_POS),SRV},
		{BasicDraw::Instance()->GetRenderTarget(BasicDraw::NORMAL),SRV},
		{BasicDraw::Instance()->GetRenderTarget(BasicDraw::BRIGHT),SRV},
		{m_otherTransformConstBuffer,CBV},
		{m_constBuffer,CBV},
		{m_consumeCountBuffer,CBV},
		{m_checkResultBuffer,UAV}
	};

	//生成してある草むらの数を取得
	D3D12App::Instance()->DispathOneShot(
		m_cPipeline[SEARCH_PLANT_POS],
		{ GRASSF_SEARCH_DISPATCH_X ,GRASSF_SEARCH_DISPATCH_Y,1 },
		descData);

	//判定結果の取得
	std::array<Grass::CheckResult, Grass::GRASSF_SEARCH_COUNT> result;

	checkResultPtr = m_checkResultBuffer->GetResource()->GetBuffOnCpu<CheckResult>();

	for (int index = 0; index < GRASSF_SEARCH_COUNT; ++index) {

		result[index].m_isSuccess = checkResultPtr[index].m_isSuccess;
		result[index].m_plantNormal = checkResultPtr[index].m_plantNormal.GetNormal();
		result[index].m_plantPos = checkResultPtr[index].m_plantPos + checkResultPtr[index].m_plantNormal;	//埋まってしまうので法線方向に少しだけ動かす。

	}

	return result;

}