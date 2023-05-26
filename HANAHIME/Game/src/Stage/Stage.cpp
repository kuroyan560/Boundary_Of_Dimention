#include "Stage.h"
#include"FrameWork/Importer.h"
#include"ForUser/JsonData.h"
#include"ForUser/Object/Model.h"
#include "StageParts.h"
#include"Enemy/Enemy.h"
#include"Switch.h"
#include<optional>

std::string Stage::s_stageModelDir = "resource/user/model/stage/";

bool Stage::CheckJsonKeyExist(std::string arg_fileName, nlohmann::json arg_json, std::string arg_key)
{
	bool exist = arg_json.contains(arg_key);
	if (!exist)
	{
		KuroEngine::AppearMessageBox("Stage : CheckJsonKeyExist() 失敗", arg_fileName + " の " + arg_json["name"].get<std::string>() + " に\"" + arg_key + "\"が含まれてないよ。");
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

void Stage::LoadWithType(std::string arg_fileName, nlohmann::json arg_json, StageParts* arg_parent, std::vector<MapPinPointData>& arg_mapPinDataArray)
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
	auto model = Importer::Instance()->LoadModel(s_stageModelDir, obj["file_name"].get<std::string>() + ".glb");

	//トランスフォーム取得
	auto transformObj = obj["transform"];

	//平行移動
	Vec3<float>translation = GetConsiderCoordinate(transformObj["translation"]);

	//回転
	XMVECTOR quaternion =
	{
		(float)transformObj["rotation"][0],
		(float)transformObj["rotation"][2],
		-(float)transformObj["rotation"][1],
		(float)transformObj["rotation"][3]
	};

	//スケーリング
	Vec3<float>scaling = { (float)transformObj["scaling"][0],(float)transformObj["scaling"][2] ,(float)transformObj["scaling"][1] };

	//トランスフォーム設定
	Transform transform;
	transform.SetPos(translation * (arg_parent == nullptr ? m_terrianScaling : 1.0f));
	transform.SetRotate(quaternion);
	transform.SetScale(scaling * (arg_parent == nullptr ? m_terrianScaling : 1.0f));

	//親がいるならトランスフォームの親子関係形成
	if (arg_parent)transform.SetParent(&arg_parent->GetTransform());

	//コリジョン判定用のモデルファイルが設定されているか
	auto collisionModel = model;
	if (obj.contains("CollisionModelFileName"))
	{
		collisionModel = Importer::Instance()->LoadModel(s_stageModelDir, obj["CollisionModelFileName"].get<std::string>() + ".glb");
	}

	StageParts* newPart = nullptr;

	//種別に応じて変わるパラメータ
		//通常の地形
	if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::TERRIAN))
	{
		m_terrianArray.emplace_back(model, transform, collisionModel);
		newPart = &m_terrianArray[static_cast<int>(m_terrianArray.size()) - 1];
	}
	//スタート地点
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::START_POINT))
	{
		transform.SetScale(1.0f);
		m_startPoint = std::make_shared<StartPoint>(model, transform);
		newPart = m_startPoint.get();
	}
	//ゴール地点
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::GOAL_POINT))
	{
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "CheckPointOrder"))return;

		//全てのレバーをオンにすることがクリア条件
		if (obj.contains("leverID") && obj["leverID"] != -1)
		{
			m_goalLeverID = obj["leverID"];
		}
		//目的地に到達することがクリア条件
		else
		{
			m_goalPoint = std::make_shared<GoalPoint>(model, transform);
		}
		newPart = m_goalPoint.get();

		//マップピンデータに追加
		arg_mapPinDataArray.emplace_back();
		arg_mapPinDataArray.back().m_order = arg_json["CheckPointOrder"].get<int>();
		arg_mapPinDataArray.back().m_part = m_goalPoint;
	}
	//見かけだけのオブジェクト
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::APPEARANCE))
	{
		m_gimmickArray.emplace_back(std::make_shared<Appearance>(model, transform, collisionModel));
		newPart = m_gimmickArray.back().get();
	}
	//動く足場
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::MOVE_SCAFFOLD))
	{
		std::vector<KuroEngine::Vec3<float>>translationArray;
		if (LoadTranslationArray(arg_fileName, &translationArray, obj))
		{
			m_gimmickArray.emplace_back(std::make_shared<MoveScaffold>(model, transform, translationArray, collisionModel));
			newPart = m_gimmickArray.back().get();
		}
	}
	//レバー
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::LEVER))
	{
		//必要なパラメータがない
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "id") || !CheckJsonKeyExist(arg_fileName, arg_json, "initFlg"))return;

		m_gimmickArray.emplace_back(std::make_shared<Lever>(model, transform, arg_json["id"], arg_json["initFlg"]));
		newPart = m_gimmickArray.back().get();
	}
	//ジップライン蔓
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::IVY_ZIP_LINE))
	{
		std::vector<KuroEngine::Vec3<float>>translationArray;
		if (LoadTranslationArray(arg_fileName, &translationArray, obj))
		{
			m_gimmickArray.emplace_back(std::make_shared<IvyZipLine>(model, transform, translationArray));
			newPart = m_gimmickArray.back().get();
		}
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

		m_gimmickArray.emplace_back(std::make_shared<IvyBlock>(model, transform, leftTopFront, rightBottomBack, collisionModel));
		newPart = m_gimmickArray.back().get();
	}
	//スプラトゥーン風フェンス
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::SPLATOON_FENCE))
	{
		m_gimmickArray.emplace_back(std::make_shared<SplatoonFence>(model, transform, collisionModel));
		newPart = m_gimmickArray.back().get();
	}
	//ゲート
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::GATE))
	{
		//必要なパラメータがない
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "GateID"))return;
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "DestStageNum"))return;
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "DestGateID"))return;

		int gateID = arg_json["GateID"].get<int>();
		int destStageNum = arg_json["DestStageNum"].get<int>() - 1;
		int destGateID = arg_json["DestGateID"].get<int>();
		m_gimmickArray.emplace_back(std::make_shared<Gate>(model, transform, gateID, destStageNum, destGateID));
		newPart = m_gimmickArray.back().get();
		m_gateArray.emplace_back(std::dynamic_pointer_cast<Gate>(m_gimmickArray.back()));

		//マップピンデータに追加
		if (arg_json.contains("CheckPointOrder"))
		{
			arg_mapPinDataArray.emplace_back();
			arg_mapPinDataArray.back().m_order = arg_json["CheckPointOrder"].get<int>();
			arg_mapPinDataArray.back().m_part = m_gimmickArray.back();
		}
	}
	//チェックポイント
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::CHECK_POINT))
	{
		//必要なパラメータがない
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "CheckPointOrder"))return;

		int order = arg_json["CheckPointOrder"].get<int>();
		m_gimmickArray.emplace_back(std::make_shared<CheckPoint>(model, transform, order));
		newPart = m_gimmickArray.back().get();

		//マップピンデータに追加
		arg_mapPinDataArray.emplace_back();
		arg_mapPinDataArray.back().m_order = order;
		arg_mapPinDataArray.back().m_part = m_gimmickArray.back();
	}
	//スターコイン
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::STAR_COIN))
	{
		m_gimmickArray.emplace_back(std::make_shared<StarCoin>(model, transform));
		newPart = m_gimmickArray.back().get();
		m_starCoinArray.emplace_back(std::dynamic_pointer_cast<StarCoin>(m_gimmickArray.back()));
	}
	//背景
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::BACKGROUND))
	{
		m_gimmickArray.emplace_back(std::make_shared<BackGround>(model, transform));
		newPart = m_gimmickArray.back().get();
	}
	//チビ虫
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::MINI_BUG))
	{
		//パラメータがない
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "Loop"))return;

		std::vector<KuroEngine::Vec3<float>>translationArray;

		bool isLoopFlag = arg_json["Loop"].get<int>();
		if (LoadTranslationArray(arg_fileName, &translationArray, obj))
		{
			m_enemyArray.emplace_back(std::make_shared<MiniBug>(model, transform, translationArray, isLoopFlag));
			newPart = m_enemyArray.back().get();
		}
	}
	//ドッスンリング
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::DOSSUN_RING))
	{
		//パラメータがない
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "ATKPat"))return;

		ENEMY_ATTACK_PATTERN attackPattern;

		auto patternName = arg_json["ATKPat"].get<std::string>();
		if (patternName.compare("NORMAL") == 0)attackPattern = ENEMY_ATTACK_PATTERN_NORMAL;
		else if (patternName.compare("ALWAYS") == 0)attackPattern = ENEMY_ATTACK_PATTERN_ALWAYS;
		else KuroEngine::AppearMessageBox("Stage : GetAttackPattern() 失敗", "知らない攻撃パターン名\"" + patternName + "\"が含まれているよ。");

		m_enemyArray.emplace_back(std::make_shared<DossunRing>(model, transform, attackPattern));
		newPart = m_enemyArray.back().get();
	}
	//砲台敵
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::BATTERY))
	{
		//パラメータがない
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "BarrelPat"))return;
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "BulletScale"))return;

		ENEMY_BARREL_PATTERN barrelPattern;
		auto patternName = arg_json["BarrelPat"].get<std::string>();
		if (patternName.compare("FIXED") == 0)barrelPattern = ENEMY_BARREL_PATTERN_FIXED;
		else if (patternName.compare("ROCKON") == 0)barrelPattern = ENEMY_BARREL_PATTERN_ROCKON;
		else KuroEngine::AppearMessageBox("Stage : GetBarrelPattern() 失敗", "知らない射撃パターン名\"" + patternName + "\"が含まれているよ。");

		float bulletScale = arg_json["BulletScale"].get<float>();

		std::vector<KuroEngine::Vec3<float>>translationArray;
		if (LoadTranslationArray(arg_fileName, &translationArray, obj))
		{

			//X軸方向を向いている敵だったら上下を反転させる。
			if (0.9f < fabs(KuroEngine::Vec3<float>(1, 0, 0).Dot(transform.GetUp()))) {
				transform.SetRotate(DirectX::XMQuaternionMultiply(transform.GetRotate(), DirectX::XMQuaternionRotationAxis(transform.GetFront(), DirectX::XM_PI)));
			}

			m_enemyArray.emplace_back(std::make_shared<Battery>(model, transform, translationArray, bulletScale, barrelPattern));
			newPart = m_enemyArray.back().get();
		}
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
			LoadWithType(arg_fileName, child, newPart, arg_mapPinDataArray);
		}
	}
}

Stage::Stage()
{
	using namespace KuroEngine;
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

void Stage::Load(int arg_ownStageIdx, std::string arg_dir, std::string arg_fileName, float arg_terrianScaling, bool arg_hasGoal)
{
	using namespace KuroEngine;

	m_terrianScaling = arg_terrianScaling;

	//地形情報クリア
	m_terrianArray.clear();
	m_gimmickArray.clear();

	JsonData jsonData(arg_dir, arg_fileName);

	//ステージ情報でない
	if (!CheckJsonKeyExist(arg_fileName, jsonData.m_jsonData, "stage"))return;

	//マップピンを指す地点のデータ配列
	std::vector<MapPinPointData>mapPinPointDataArray;

	auto stageJsonData = jsonData.m_jsonData["stage"];
	for (auto& obj : stageJsonData["objects"])
	{
		LoadWithType(arg_fileName, obj, nullptr, mapPinPointDataArray);
	}

	//マップピンデータ配列を順番通りにソート
	std::sort(mapPinPointDataArray.begin(), mapPinPointDataArray.end(), [](MapPinPointData& a, MapPinPointData& b)
		{
			return a.m_order < b.m_order;
		});

	//マップピン地点を記録
	for (int pinIdx = 0; pinIdx < static_cast<int>(mapPinPointDataArray.size()); ++pinIdx)
	{
		//数字に被りがある場合警告
		if (pinIdx != static_cast<int>(mapPinPointDataArray.size()) - 1 && mapPinPointDataArray[pinIdx].m_order == mapPinPointDataArray[pinIdx + 1].m_order)
		{
			AppearMessageBox("Stage : Load() 警告", arg_fileName + "の CheckPointOrder の数字が被ってるけど大丈夫？");
		}
		m_mapPinPoint.emplace_back(mapPinPointDataArray[pinIdx].m_part);
	}

	//スタート地点があるか
	if (arg_ownStageIdx == 0 && !m_startPoint)
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

int Stage::GetStarCoinNum() const
{
	int getCoinNum = static_cast<int>(std::count_if(m_starCoinArray.begin(), m_starCoinArray.end(), [](std::weak_ptr<StarCoin>coin)
		{
			return coin.lock()->IsGet();
		}));
	return getCoinNum;
}

KuroEngine::Transform Stage::GetGateTransform(int arg_gateID) const
{
	for (auto& gate : m_gateArray)
	{
		if (!gate.lock()->CheckID(arg_gateID))continue;
		return gate.lock()->GetInitTransform();
	}
	return KuroEngine::Transform();
}