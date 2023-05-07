#include "Stage.h"
#include"FrameWork/Importer.h"
#include"ForUser/JsonData.h"
#include"ForUser/Object/Model.h"
#include "StageParts.h"
#include"Enemy/Enemy.h"
#include"Switch.h"
#include<optional>

std::string Stage::s_terrianModelDir = "resource/user/model/terrian/";

bool Stage::CheckJsonKeyExist(std::string arg_fileName, nlohmann::json arg_json, std::string arg_key)
{
	bool exist = arg_json.contains(arg_key);
	if (!exist)
	{
		KuroEngine::AppearMessageBox("Stage : CheckJsonKeyExist() 失敗", arg_fileName + " に\"" + arg_key + "\"が含まれてないよ。");
	}
	return exist;
}

KuroEngine::Vec3<float> Stage::GetConsiderCoordinate(nlohmann::json arg_json)
{
	return KuroEngine::Vec3<float>(-(float)arg_json[0], (float)arg_json[2], -(float)arg_json[1]);
}

bool Stage::LoadTranslationArray(std::string arg_fileName, std::vector<KuroEngine::Vec3<float>>* arg_result, nlohmann::json arg_json)
{
	using namespace KuroEngine;

	if (!CheckJsonKeyExist(arg_fileName, arg_json, "translationArray"))return false;

	nlohmann::json jsonArray = arg_json["translationArray"];

	std::vector<Vec3<float>>translationArray;
	int idx = 0;
	std::string key = "translation_" + std::to_string(idx);

	while (jsonArray.contains(key))
	{
		translationArray.emplace_back();
		//平行移動
		translationArray.back() = GetConsiderCoordinate(jsonArray[key]);
		translationArray.back() *= m_terrianScaling;

		key = "translation_" + std::to_string(++idx);
	}

	*arg_result = translationArray;

	return true;
}

void Stage::LoadWithType(std::string arg_fileName, nlohmann::json arg_json, StageParts* arg_parent)
{
	using namespace KuroEngine;

	auto& obj = arg_json;

	//種別のパラメータがない
	if (!CheckJsonKeyExist(arg_fileName, obj, "type"))return;

	//モデルの名前のパラメータがない
	if (!CheckJsonKeyExist(arg_fileName, obj, "file_name"))return;

	//トランスフォームのパラメータがない
	if (!CheckJsonKeyExist(arg_fileName, obj, "transform"))return;

	//共通パラメータ
	//種別
	auto typeKey = obj["type"].get<std::string>();
	//モデル設定
	auto model = Importer::Instance()->LoadModel(s_terrianModelDir, obj["file_name"].get<std::string>() + ".glb");

	//トランスフォーム取得
	auto transformObj = obj["transform"];

	//平行移動
	Vec3<float>translation = GetConsiderCoordinate(transformObj["translation"]);

	//回転
	XMVECTOR quaternion = { (float)transformObj["rotation"][0],(float)transformObj["rotation"][2], -(float)transformObj["rotation"][1],(float)transformObj["rotation"][3] };

	//スケーリング
	Vec3<float>scaling = { (float)transformObj["scaling"][0],(float)transformObj["scaling"][2] ,(float)transformObj["scaling"][1] };

	//トランスフォーム設定
	Transform transform;
	transform.SetPos(translation * (arg_parent == nullptr ? m_terrianScaling : 1.0f));
	transform.SetRotate(quaternion);
	transform.SetScale(scaling * (arg_parent == nullptr ? m_terrianScaling : 1.0f));

	StageParts* newPart = nullptr;

	//種別に応じて変わるパラメータ
		//通常の地形
	if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::TERRIAN))
	{
		m_terrianArray.emplace_back(model, transform, arg_parent);
		newPart = &m_terrianArray[static_cast<int>(m_terrianArray.size()) - 1];
	}
	//スタート地点
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::START_POINT))
	{
		transform.SetScale(1.0f);
		m_startPoint = std::make_shared<StartPoint>(model, transform, arg_parent);
		newPart = m_startPoint.get();
	}
	//ゴール地点
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::GOAL_POINT))
	{
		//全てのレバーをオンにすることがクリア条件
		if (obj.contains("leverID") && obj["leverID"] != -1)
		{
			m_goalLeverID = obj["leverID"];
		}
		//目的地に到達することがクリア条件
		else
		{
			m_goalPoint = std::make_shared<GoalPoint>(model, transform, arg_parent);
		}
		newPart = m_goalPoint.get();
	}
	//見かけだけのオブジェクト
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::APPEARANCE))
	{
		m_gimmickArray.emplace_back(std::make_shared<Appearance>(model, transform, arg_parent));
		newPart = m_gimmickArray.back().get();
	}
	//動く足場
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::MOVE_SCAFFOLD))
	{
		std::vector<KuroEngine::Vec3<float>>translationArray;
		if (LoadTranslationArray(arg_fileName, &translationArray, obj))
		{
			m_gimmickArray.emplace_back(std::make_shared<MoveScaffold>(model, transform, arg_parent, translationArray));
		}
		newPart = m_gimmickArray.back().get();
	}
	//レバー
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::LEVER))
	{
		//必要なパラメータがない
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "id") || !CheckJsonKeyExist(arg_fileName, arg_json, "initFlg"))return;

		m_gimmickArray.emplace_back(std::make_shared<Lever>(model, transform, arg_parent, arg_json["id"], arg_json["initFlg"]));
		newPart = m_gimmickArray.back().get();
	}
	//ジップライン蔓
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::IVY_ZIP_LINE))
	{
		std::vector<KuroEngine::Vec3<float>>translationArray;
		if (LoadTranslationArray(arg_fileName, &translationArray, obj))
		{
			m_gimmickArray.emplace_back(std::make_shared<IvyZipLine>(model, transform, arg_parent, translationArray));
		}
		newPart = m_gimmickArray.back().get();
	}
	//蔓ブロック
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::IVY_BLOCK))
	{
		//必要なパラメータがない
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "block"))return;
		if (!CheckJsonKeyExist(arg_fileName, arg_json["block"], "left_top_front_pos"))return;
		if (!CheckJsonKeyExist(arg_fileName, arg_json["block"], "right_bottom_back_pos"))return;

		//左上手前
		Vec3<float>leftTopFront = GetConsiderCoordinate(arg_json["block"]["left_top_front_pos"]);
		//右下奥
		Vec3<float>rightBottomBack = GetConsiderCoordinate(arg_json["block"]["right_bottom_back_pos"]);

		m_gimmickArray.emplace_back(std::make_shared<IvyBlock>(model, transform, arg_parent, leftTopFront, rightBottomBack));
		newPart = m_gimmickArray.back().get();
	}
	//チビ虫
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::MINI_BUG))
	{
		//m_enemyArray.emplace_back(std::make_shared<MiniBug>(model, transform, arg_parent));
		//newPart = m_enemyArray.back().get();
	}
	//ドッスンリング
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::DOSSUN_RING))
	{
		//m_enemyArray.emplace_back(std::make_shared<DossunRing>(model, transform, arg_parent));
		//newPart = m_enemyArray.back().get();
	}
	else
	{
		AppearMessageBox("Warning : Stage::LoadWithType()", "ステージパーツの読み込み中に知らない種別キー \"" + typeKey + "\"があったけど大丈夫？");
	}

	//子供のパーツ読み込み
	if (obj.contains("children"))
	{
		for (auto child : obj["children"])
		{
			LoadWithType(arg_fileName, child, newPart);
		}
	}
}

Stage::Stage()
{
	using namespace KuroEngine;

	//デフォルトのモデル
		//スカイドーム
	static std::shared_ptr<Model>s_defaultSkydomeModel
		= Importer::Instance()->LoadModel("resource/user/model/", "Skydome.glb");
	//森林円柱
	static std::shared_ptr<Model>s_defaultWoodsCylinderModel
		= Importer::Instance()->LoadModel("resource/user/model/", "Woods.glb");

	//デフォルトの画像
		//地面
	static std::shared_ptr<TextureBuffer>s_defaultGroundTex
		= D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/ground.png");

	//デフォルト値設定
	m_skydomeModel = s_defaultSkydomeModel;
	m_woodsCylinderModel = s_defaultWoodsCylinderModel;
	m_groundTex = s_defaultGroundTex;
}

void Stage::Init()
{
	for (auto& gimmick : m_gimmickArray)
	{
		gimmick->Init();
	}

	for (auto& enemy : m_enemyArray)
	{
		enemy->Init();
	}
}

void Stage::Update(Player& arg_player)
{
	for (auto& gimmick : m_gimmickArray)
	{
		gimmick->Update(arg_player);
	}

	//動く足場同士の当たり判定を行う。
	for (auto& gimmickA : m_gimmickArray) {

		//動く足場じゃなかったら処理を飛ばす。
		if (gimmickA->GetType() != StageParts::STAGE_PARTS_TYPE::MOVE_SCAFFOLD) continue;

		for (auto& gimmickB : m_gimmickArray) {

			//動く足場じゃなかったら処理を飛ばす。
			if (gimmickB->GetType() != StageParts::STAGE_PARTS_TYPE::MOVE_SCAFFOLD) continue;

			//同じオブジェクトだったら処理を飛ばす。
			if (gimmickA == gimmickB) continue;

			//当たり判定を行う。
			auto moveScaffoldA = dynamic_pointer_cast<MoveScaffold>(gimmickA);
			auto moveScaffoldB = dynamic_pointer_cast<MoveScaffold>(gimmickB);
			std::optional<AABB::CollisionInfo> result;

			//すべてのメッシュを走査して当たり判定を行う。
			for (auto& meshA : moveScaffoldA->m_collider.m_aabb) {
				for (auto& aabbA : meshA) {

					for (auto& meshB : moveScaffoldB->m_collider.m_aabb) {
						for (auto& aabbB : meshB) {

							result = aabbA.CheckAABBCollision(aabbB);
							if (!result) continue;
							//極小の誤差は無視する。
							if (result->m_pushBack.Length() < 0.001f) continue;

							//当たっていたらギミックの動きを止める。
							moveScaffoldA->Stop();
							moveScaffoldB->Stop();

							////押し戻す。
							//if (moveScaffoldA->GetIsActive() && moveScaffoldB->GetIsActive()) {
							//	moveScaffoldA->PushBack(result->m_pushBack / 2.0f);
							//	moveScaffoldB->PushBack(result->m_pushBack / 2.0f);
							//}
							//else if (moveScaffoldA->GetIsActive()) {
							//	moveScaffoldA->PushBack(result->m_pushBack);
							//}
							//else if (moveScaffoldB->GetIsActive()) {
							//	moveScaffoldB->PushBack(result->m_pushBack);
							//}

							//moveScaffoldA->BuildCollisionMesh();
							//moveScaffoldB->BuildCollisionMesh();

						}
					}
				}

			}

		}

	}

	for (auto& enemy : m_enemyArray)
	{
		enemy->Update(arg_player);
	}

	if (m_goalPoint)m_goalPoint->Update(arg_player);
}

void Stage::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	for (auto& enemy : m_enemyArray)
	{
		enemy->Draw(arg_cam, arg_ligMgr);
	}

	for (auto& terrian : m_terrianArray)
	{
		terrian.Draw(arg_cam, arg_ligMgr);
	}

	for (auto& gimmick : m_gimmickArray)
	{
		gimmick->Draw(arg_cam, arg_ligMgr);
	}

	if (m_startPoint)
	{
		m_startPoint->Draw(arg_cam, arg_ligMgr);
	}
	if (m_goalPoint)
	{
		m_goalPoint->Draw(arg_cam, arg_ligMgr);
	}
}

bool Stage::IsClear() const
{
	//レバークリア
	if (m_goalLeverID != Lever::INVALID_ID)return m_goalSwitch->IsBooting();

	//目的地到達
	if (m_goalPoint)return m_goalPoint->HitPlayer();

	//クリアが存在しない
	return false;
}

void Stage::Load(std::string arg_dir, std::string arg_fileName, float arg_terrianScaling, bool arg_hasGoal)
{
	using namespace KuroEngine;

	m_terrianScaling = arg_terrianScaling;

	//地形情報クリア
	m_terrianArray.clear();
	m_gimmickArray.clear();

	JsonData jsonData(arg_dir, arg_fileName);

	//ステージ情報でない
	if (!CheckJsonKeyExist(arg_fileName, jsonData.m_jsonData, "stage"))return;

	auto stageJsonData = jsonData.m_jsonData["stage"];
	for (auto& obj : stageJsonData["objects"])
	{
		LoadWithType(arg_fileName, obj, nullptr);
	}

	//スタート地点があるか
	if (!m_startPoint)
	{
		AppearMessageBox("Stage : Load() 警告", arg_fileName + " にスタート地点の情報がないよ。");
	}
	//ゴールがあるか
	if (arg_hasGoal && !m_goalPoint && m_goalLeverID == Lever::INVALID_ID)
	{
		AppearMessageBox("Stage : Load() 警告", arg_fileName + "にゴールの情報がないよ。");
	}

	//レバーとスイッチの関係構築（ゴール）
	if (m_goalLeverID != Lever::INVALID_ID)
	{
		m_goalSwitch = std::make_shared<Switch>();
		m_goalSwitch->m_leverID = m_goalLeverID;

		std::vector<std::weak_ptr<Lever>>goalLeverArray;
		for (auto& gimmick : m_gimmickArray)
		{
			//レバーじゃない
			if (gimmick->GetType() != StageParts::LEVER)continue;

			//レバーのポインタに変換
			auto lever = dynamic_pointer_cast<Lever>(gimmick);

			//レバーの識別番号が異なる
			if (lever->m_id != m_goalLeverID)continue;

			//関係構築
			lever->m_parentSwitch = m_goalSwitch.get();
			m_goalSwitch->m_leverArray.emplace_back(lever);
		}
	}

}