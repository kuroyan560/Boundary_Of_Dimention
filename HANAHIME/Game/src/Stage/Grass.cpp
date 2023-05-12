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

	//光っているかの判定コンピュートパイプライン生成
	{
		//ルートパラメータ
		std::vector<RootParam>rootParam =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, "判定の結果を格納するバッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "草の座標配列バッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "光が当たっている範囲のマップ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "カメラ情報"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "プレイヤー情報"),
		};

		//光っているか
		auto cs_checkBright = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass_CheckBright.hlsl", "CheckBright", "cs_6_4");
		m_cPipeline[CHECK_IS_BRIGHT] = D3D12App::Instance()->GenerateComputePipeline(cs_checkBright, rootParam, { WrappedSampler(true,true) });
	}

	//生成位置計算コンピュートパイプライン生成
	{
		//ルートパラメータ
		std::vector<RootParam>rootParam =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, "判定の結果を格納するバッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "草の座標配列バッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "ワールド座標"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "法線マップ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "光が当たっている範囲のマップ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "定数バッファ"),
		};

		//生成位置判定用パイプライン
		auto cs_searchPlant = D3D12App::Instance()->CompileShader("resource/user/shaders/Grass_SearchPlantPos.hlsl", "SearchPlantPos", "cs_6_4");
		m_cPipeline[SEARCH_PLANT_POS] = D3D12App::Instance()->GenerateComputePipeline(cs_searchPlant, rootParam, { WrappedSampler(true,true) });
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
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"生成した草のワールド行列バッファー(StructuredBuffer)"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"カメラ情報"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"全草むらで共通する定数バッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "草むら以外のトランスフォームデータ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "テクスチャ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "マテリアル"),
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

	//プレイヤー情報用定数バッファ
	m_playerInfoBuffer = D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(PlayerInfo),
		1,
		nullptr,
		"Grass - PlayerInfo - ConstantBuffer");

	//生成位置を計算するのに使用する定数バッファ
	m_searchPlantPosConstBuffer = D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(SearchPlantPosConstData),
		1,
		nullptr,
		"Grass - SearchPlantPos - ConstantBuffer");

	//生成位置探索の結果の格納用バッファ
	m_searchPlantResultBuffer = D3D12App::Instance()->GenerateRWStructuredBuffer(
		sizeof(SearchPlantResult),
		GRASSF_SEARCH_COUNT,
		nullptr,
		"Grass - SearchPlantResult - RWStructuredBuffer");

	//光っているかの判定結果の確報バッファ
	m_checkBrightResultBuffer = D3D12App::Instance()->GenerateRWStructuredBuffer(
		sizeof(int),
		s_plantGrassMax,
		nullptr,
		"Grass - CheckBrightResult - RWStructuredBuffer");

	//GPU上の草データ
	m_plantGrassPosArray.reserve(s_plantGrassMax);
	m_plantGrassPosArrayBuffer = D3D12App::Instance()->GenerateStructuredBuffer(
		sizeof(KuroEngine::Vec3<float>),
		s_plantGrassMax,
		nullptr,
		"Grass - PlantGrassPosArray - StructuredBuffer");

	//モデル
	for (int modelIdx = 0; modelIdx < s_modelNumMax; ++modelIdx)
	{
		std::string fileName = "Grass_" + std::to_string(modelIdx) + ".glb";
		m_modelArray[modelIdx] = Importer::Instance()->LoadModel("resource/user/model/", fileName);

		//草の描画用ワールド行列配列バッファ
		m_grassWorldMatriciesBuffer[modelIdx] = D3D12App::Instance()->GenerateStructuredBuffer(
			sizeof(Matrix),
			s_plantGrassMax,
			nullptr,
			"Grass - GrassIndicies - StructuredBuffer");

		m_grassWorldMatricies[modelIdx].reserve(s_plantGrassMax);
	}
}

void Grass::Init()
{
	using namespace KuroEngine;

	m_plantGrassDataArray.clear();
	m_plantGrassWorldMatArray.clear();

	m_oldPlayerPos = { -1000,-1000,-1000 };
	m_plantTimer.Reset(0);
}

void Grass::Update(const float arg_timeScale, const KuroEngine::Transform arg_playerTransform, std::weak_ptr<KuroEngine::Camera> arg_cam, float arg_plantInfluenceRange, const std::weak_ptr<Stage>arg_nowStage, bool arg_isAttack)
{
	using namespace KuroEngine;

	//トランスフォーム情報をGPUに送信
	PlayerInfo playerInfo;
	playerInfo.m_pos = arg_playerTransform.GetPos();
	playerInfo.m_plantLighrRante = arg_plantInfluenceRange;
	m_playerInfoBuffer->Mapping(&playerInfo);

	//プレイヤーの座標を保存。
	m_playerPos = arg_playerTransform.GetPos();

	//生成した数
	int generateCount = 0;
	//削除した数
	int consumeCount = 0;


	//if (m_plantTimer.IsTimeUp() && 0.01f < KuroEngine::Vec3<float>(m_oldPlayerPos - arg_playerTransform.GetPos()).Length())
	if (true)
	{
		//トランスフォームに流し込む
		Transform grassTransform;
		grassTransform.SetPos(arg_playerTransform.GetPos());
		grassTransform.SetRotate(arg_playerTransform.GetRotate());
		grassTransform.SetScale({ 1.0f,1.0f,1.0f });

		//草をはやす場所を取得。
		std::array<Grass::SearchPlantResult, Grass::GRASSF_SEARCH_COUNT> plantData = SearchPlantPos(arg_playerTransform);

		for (int count = 0; count < GRASSF_SEARCH_COUNT; ++count)
		{
			if (plantData[count].m_isSuccess != 1) continue;
			if (plantData[count].m_plantNormal.Length() <= FLT_EPSILON)continue;

			//草の生成
			m_plantGrassDataArray.emplace_back();
			m_plantGrassDataArray.back().m_pos = plantData[count].m_plantPos;
			m_plantGrassDataArray.back().m_normal = plantData[count].m_plantNormal;
			m_plantGrassDataArray.back().m_sineLength = KuroEngine::GetRand(40) / 100.0f;
			m_plantGrassDataArray.back().m_modelIdx = KuroEngine::GetRand(3 - 1);
			m_plantGrassDataArray.back().m_appearY = 0.1f;
			m_plantGrassDataArray.back().m_appearYTimer = 0.1f;

			//生成した数のカウント
			generateCount++;
		}

		//上限到達
		if (s_plantGrassMax < m_plantGrassDataArray.size())
		{
			KuroEngine::AppearMessageBox("Grass : Update() 失敗", "生成できる上限を超えたよ");
			exit(1);
		}

		m_plantTimer.Reset(0);
	}
	m_plantTimer.UpdateTimer();

	m_oldPlayerPos = arg_playerTransform.GetPos();

	//植えた草むらの更新
	if (!m_plantGrassDataArray.empty())
	{
		std::vector<RegisterDescriptorData>descData =
		{
			{m_checkBrightResultBuffer,UAV},
			{m_plantGrassPosArrayBuffer,SRV},
			{BasicDraw::Instance()->GetRenderTarget(BasicDraw::BRIGHT),SRV},
			{arg_cam.lock()->GetBuff(),CBV},
			{m_playerInfoBuffer,CBV},
		};

		D3D12App::Instance()->DispathOneShot(
			m_cPipeline[CHECK_IS_BRIGHT],
			{ static_cast<int>(m_plantGrassDataArray.size()),1,1 },
			descData);

		for (int grassIdx = 0; grassIdx < static_cast<int>(m_plantGrassDataArray.size()); ++grassIdx)
		{
			auto grass = m_plantGrassDataArray[grassIdx];

			//草のイージングの更新処理
			UpdateGrassEasing(grass, grassIdx);

			//草の当たり判定
			GrassCheckHit(grass, arg_nowStage);

			//草にトランスフォームを適応。
			//Indexが-1だったら処理を飛ばす。
			if (grass.m_terrianIdx != -1) {

				auto terrian = arg_nowStage.lock()->GetGimmickArray().begin();
				std::advance(terrian, grass.m_terrianIdx);

				auto move = terrian->get()->GetMoveAmount();
				grass.m_pos += move;

			}

			if (grass.m_isDead) {
				consumeCount++;
			}

			m_plantGrassDataArray[grassIdx] = grass;
		}

		auto result = std::remove_if(m_plantGrassDataArray.begin(), m_plantGrassDataArray.end(), [](GrassData grass)
			{
				return grass.m_isDead;
			});
		consumeCount = static_cast<int>(std::distance(result, m_plantGrassDataArray.end()));
		m_plantGrassDataArray.erase(result, m_plantGrassDataArray.end());
	}


	//if (consumeCount || generateCount)
	//{
	for (auto& worldMatricies : m_grassWorldMatricies)worldMatricies.clear();
	m_plantGrassPosArray.clear();

	Transform grassTransform;
	for (int grassIdx = 0; grassIdx < static_cast<int>(m_plantGrassDataArray.size()); ++grassIdx)
	{
		auto& grass = m_plantGrassDataArray[grassIdx];

		KuroEngine::Vec3<float>appearOffset = { 0.0f,-0.5f,0.0f };
		appearOffset *= (3.0f - grass.m_appearY * 3.0f);
		grassTransform.SetPos(grass.m_pos + appearOffset);
		grassTransform.SetUp(grass.m_normal);
		m_grassWorldMatricies[grass.m_modelIdx].emplace_back(grassTransform.GetMatWorld());
		m_plantGrassPosArray.emplace_back(grass.m_pos + appearOffset);
	}

	for (int modelIdx = 0; modelIdx < s_modelNumMax; ++modelIdx)
	{
		m_grassWorldMatriciesBuffer[modelIdx]->Mapping(m_grassWorldMatricies[modelIdx].data(), static_cast<int>(m_grassWorldMatricies[modelIdx].size()));
	}
	m_plantGrassPosArrayBuffer->Mapping(m_plantGrassPosArray.data(), static_cast<int>(m_plantGrassPosArray.size()));
	//}
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
				{m_grassWorldMatriciesBuffer[modelIdx],SRV},
				{arg_cam.GetBuff(),CBV},
				{mesh.material->buff,CBV},
				{m_playerInfoBuffer,CBV},
				{mesh.material->texBuff[COLOR_TEX],SRV},
			};

			KuroEngineDevice::Instance()->Graphics().ObjectRender(
				mesh.mesh->vertBuff,
				mesh.mesh->idxBuff,
				descData,
				0,
				true,
				static_cast<int>(m_grassWorldMatricies[modelIdx].size()));
		}
	}
}

std::array<Grass::SearchPlantResult, Grass::GRASSF_SEARCH_COUNT> Grass::SearchPlantPos(KuroEngine::Transform arg_playerTransform)
{
	using namespace KuroEngine;

	//判定結果の初期化
	auto checkResultPtr = m_searchPlantResultBuffer->GetResource()->GetBuffOnCpu<SearchPlantResult>();
	checkResultPtr->m_plantPos = KuroEngine::Vec3<float>();

	//必要なデータを送信
	SearchPlantPosConstData constData;
	constData.m_grassCount = static_cast<int>(m_plantGrassDataArray.size());
	constData.m_seed = KuroEngine::GetRand(0, 100000) / 100.0f;
	m_searchPlantPosConstBuffer->Mapping(&constData);

	//判定用コンピュートパイプライン実行
	//登録するディスクリプタの情報配列
	std::vector<RegisterDescriptorData>descData =
	{
		{m_searchPlantResultBuffer,UAV},
		{m_plantGrassPosArrayBuffer,SRV},
		{BasicDraw::Instance()->GetRenderTarget(BasicDraw::WORLD_POS),SRV},
		{BasicDraw::Instance()->GetRenderTarget(BasicDraw::NORMAL),SRV},
		{BasicDraw::Instance()->GetRenderTarget(BasicDraw::BRIGHT),SRV},
		{m_searchPlantPosConstBuffer,CBV},
	};

	//生成してある草むらの数を取得
	D3D12App::Instance()->DispathOneShot(
		m_cPipeline[SEARCH_PLANT_POS],
		{ GRASSF_SEARCH_DISPATCH_X ,GRASSF_SEARCH_DISPATCH_Y,1 },
		descData);

	//判定結果の取得
	std::array<Grass::SearchPlantResult, Grass::GRASSF_SEARCH_COUNT> result;

	checkResultPtr = m_searchPlantResultBuffer->GetResource()->GetBuffOnCpu<SearchPlantResult>();

	for (int index = 0; index < GRASSF_SEARCH_COUNT; ++index) {

		result[index].m_isSuccess = checkResultPtr[index].m_isSuccess;
		result[index].m_plantNormal = checkResultPtr[index].m_plantNormal.GetNormal();
		result[index].m_plantPos = checkResultPtr[index].m_plantPos + checkResultPtr[index].m_plantNormal;	//埋まってしまうので法線方向に少しだけ動かす。

	}

	return result;

}

void Grass::UpdateGrassEasing(Grass::GrassData& arg_grass, int arg_index)
{

	//コンピュートシェーダーで計算した結果を取得
	bool isBright = m_checkBrightResultBuffer->GetResource()->GetBuffOnCpu<int>()[arg_index];

	if (isBright)
	{
		static const float appearEaseSpeed = 0.05f;
		//イージングタイマー更新
		arg_grass.m_appearYTimer = std::min(arg_grass.m_appearYTimer + appearEaseSpeed, 1.0f);
	}
	else
	{
		static const float deadEaseSpeed = 0.03f;
		//イージングタイマー更新
		arg_grass.m_appearYTimer = std::max(arg_grass.m_appearYTimer - deadEaseSpeed, 0.0f);

		//0以下になったらフラグを折る。
		if (arg_grass.m_appearYTimer <= FLT_EPSILON)
		{
			arg_grass.m_isDead = true;
		}
	}

	//イージング量を求める
	if (1.0f < arg_grass.m_appearY)
	{
		arg_grass.m_appearY += (1.0f - arg_grass.m_appearY) / 10.0f;
	}
	else
	{
		arg_grass.m_appearY = arg_grass.m_appearYTimer;
	}

}

void Grass::GrassCheckHit(Grass::GrassData& arg_grass, const std::weak_ptr<Stage>arg_nowStage) {

	if (!arg_grass.m_isCheckGround) {

		//下方向にレイを飛ばして、そこが動く足場だったら親子関係を結ぶ。
		int terrianIdx = 0;
		for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
		{
			//動く足場でない
			if (terrian->GetType() != StageParts::MOVE_SCAFFOLD)	continue;

			//動く足場としてキャスト
			auto moveScaffold = dynamic_pointer_cast<MoveScaffold>(terrian);
			//モデル情報取得
			auto model = terrian->GetModel();

			//メッシュを走査
			for (auto& modelMesh : model.lock()->m_meshes)
			{

				//当たり判定用メッシュ
				auto checkHitMesh = moveScaffold->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

				CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_grass.m_pos, -arg_grass.m_normal, checkHitMesh);

				//当たっていたら
				if (output.m_isHit && 0 < output.m_distance && output.m_distance <= 1.0f) {

					//親子関係を持たせる。
					arg_grass.m_terrianIdx = terrianIdx;

				}
			}
			++terrianIdx;
		}
		arg_grass.m_isCheckGround = true;

	}

}