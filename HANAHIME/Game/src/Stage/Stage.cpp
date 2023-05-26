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
		KuroEngine::AppearMessageBox("Stage : CheckJsonKeyExist() ���s", arg_fileName + " �� " + arg_json["name"].get<std::string>() + " ��\"" + arg_key + "\"���܂܂�ĂȂ���B");
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
		//���s�ړ�
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

	//��ʂ̃p�����[�^���Ȃ�
	if (!CheckJsonKeyExist(arg_fileName, obj, "type"))return;

	//���f���̖��O�̃p�����[�^���Ȃ�
	if (!CheckJsonKeyExist(arg_fileName, obj, "file_name"))return;

	//�g�����X�t�H�[���̃p�����[�^���Ȃ�
	if (!CheckJsonKeyExist(arg_fileName, obj, "transform"))return;

	//���ʃp�����[�^
	//���
	auto typeKey = obj["type"].get<std::string>();
	//���f���ݒ�
	auto model = Importer::Instance()->LoadModel(s_stageModelDir, obj["file_name"].get<std::string>() + ".glb");

	//�g�����X�t�H�[���擾
	auto transformObj = obj["transform"];

	//���s�ړ�
	Vec3<float>translation = GetConsiderCoordinate(transformObj["translation"]);

	//��]
	XMVECTOR quaternion =
	{
		(float)transformObj["rotation"][0],
		(float)transformObj["rotation"][2],
		-(float)transformObj["rotation"][1],
		(float)transformObj["rotation"][3]
	};

	//�X�P�[�����O
	Vec3<float>scaling = { (float)transformObj["scaling"][0],(float)transformObj["scaling"][2] ,(float)transformObj["scaling"][1] };

	//�g�����X�t�H�[���ݒ�
	Transform transform;
	transform.SetPos(translation * (arg_parent == nullptr ? m_terrianScaling : 1.0f));
	transform.SetRotate(quaternion);
	transform.SetScale(scaling * (arg_parent == nullptr ? m_terrianScaling : 1.0f));

	//�e������Ȃ�g�����X�t�H�[���̐e�q�֌W�`��
	if (arg_parent)transform.SetParent(&arg_parent->GetTransform());

	//�R���W��������p�̃��f���t�@�C�����ݒ肳��Ă��邩
	auto collisionModel = model;
	if (obj.contains("CollisionModelFileName"))
	{
		collisionModel = Importer::Instance()->LoadModel(s_stageModelDir, obj["CollisionModelFileName"].get<std::string>() + ".glb");
	}

	StageParts* newPart = nullptr;

	//��ʂɉ����ĕς��p�����[�^
		//�ʏ�̒n�`
	if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::TERRIAN))
	{
		m_terrianArray.emplace_back(model, transform, collisionModel);
		newPart = &m_terrianArray[static_cast<int>(m_terrianArray.size()) - 1];
	}
	//�X�^�[�g�n�_
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::START_POINT))
	{
		transform.SetScale(1.0f);
		m_startPoint = std::make_shared<StartPoint>(model, transform);
		newPart = m_startPoint.get();
	}
	//�S�[���n�_
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::GOAL_POINT))
	{
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "CheckPointOrder"))return;

		//�S�Ẵ��o�[���I���ɂ��邱�Ƃ��N���A����
		if (obj.contains("leverID") && obj["leverID"] != -1)
		{
			m_goalLeverID = obj["leverID"];
		}
		//�ړI�n�ɓ��B���邱�Ƃ��N���A����
		else
		{
			m_goalPoint = std::make_shared<GoalPoint>(model, transform);
		}
		newPart = m_goalPoint.get();

		//�}�b�v�s���f�[�^�ɒǉ�
		arg_mapPinDataArray.emplace_back();
		arg_mapPinDataArray.back().m_order = arg_json["CheckPointOrder"].get<int>();
		arg_mapPinDataArray.back().m_part = m_goalPoint;
	}
	//�����������̃I�u�W�F�N�g
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::APPEARANCE))
	{
		m_gimmickArray.emplace_back(std::make_shared<Appearance>(model, transform, collisionModel));
		newPart = m_gimmickArray.back().get();
	}
	//��������
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::MOVE_SCAFFOLD))
	{
		std::vector<KuroEngine::Vec3<float>>translationArray;
		if (LoadTranslationArray(arg_fileName, &translationArray, obj))
		{
			m_gimmickArray.emplace_back(std::make_shared<MoveScaffold>(model, transform, translationArray, collisionModel));
			newPart = m_gimmickArray.back().get();
		}
	}
	//���o�[
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::LEVER))
	{
		//�K�v�ȃp�����[�^���Ȃ�
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "id") || !CheckJsonKeyExist(arg_fileName, arg_json, "initFlg"))return;

		m_gimmickArray.emplace_back(std::make_shared<Lever>(model, transform, arg_json["id"], arg_json["initFlg"]));
		newPart = m_gimmickArray.back().get();
	}
	//�W�b�v���C����
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::IVY_ZIP_LINE))
	{
		std::vector<KuroEngine::Vec3<float>>translationArray;
		if (LoadTranslationArray(arg_fileName, &translationArray, obj))
		{
			m_gimmickArray.emplace_back(std::make_shared<IvyZipLine>(model, transform, translationArray));
			newPart = m_gimmickArray.back().get();
		}
	}
	//���u���b�N
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::IVY_BLOCK))
	{
		//�K�v�ȃp�����[�^���Ȃ�
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "block"))return;
		if (!CheckJsonKeyExist(arg_fileName, arg_json["block"], "left_top_front_pos"))return;
		if (!CheckJsonKeyExist(arg_fileName, arg_json["block"], "right_bottom_back_pos"))return;

		//�����O
		Vec3<float>leftTopFront = GetConsiderCoordinate(arg_json["block"]["left_top_front_pos"]);
		//�E����
		Vec3<float>rightBottomBack = GetConsiderCoordinate(arg_json["block"]["right_bottom_back_pos"]);

		m_gimmickArray.emplace_back(std::make_shared<IvyBlock>(model, transform, leftTopFront, rightBottomBack, collisionModel));
		newPart = m_gimmickArray.back().get();
	}
	//�X�v���g�D�[�����t�F���X
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::SPLATOON_FENCE))
	{
		m_gimmickArray.emplace_back(std::make_shared<SplatoonFence>(model, transform, collisionModel));
		newPart = m_gimmickArray.back().get();
	}
	//�Q�[�g
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::GATE))
	{
		//�K�v�ȃp�����[�^���Ȃ�
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "GateID"))return;
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "DestStageNum"))return;
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "DestGateID"))return;

		int gateID = arg_json["GateID"].get<int>();
		int destStageNum = arg_json["DestStageNum"].get<int>() - 1;
		int destGateID = arg_json["DestGateID"].get<int>();
		m_gimmickArray.emplace_back(std::make_shared<Gate>(model, transform, gateID, destStageNum, destGateID));
		newPart = m_gimmickArray.back().get();
		m_gateArray.emplace_back(std::dynamic_pointer_cast<Gate>(m_gimmickArray.back()));

		//�}�b�v�s���f�[�^�ɒǉ�
		if (arg_json.contains("CheckPointOrder"))
		{
			arg_mapPinDataArray.emplace_back();
			arg_mapPinDataArray.back().m_order = arg_json["CheckPointOrder"].get<int>();
			arg_mapPinDataArray.back().m_part = m_gimmickArray.back();
		}
	}
	//�`�F�b�N�|�C���g
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::CHECK_POINT))
	{
		//�K�v�ȃp�����[�^���Ȃ�
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "CheckPointOrder"))return;

		int order = arg_json["CheckPointOrder"].get<int>();
		m_gimmickArray.emplace_back(std::make_shared<CheckPoint>(model, transform, order));
		newPart = m_gimmickArray.back().get();

		//�}�b�v�s���f�[�^�ɒǉ�
		arg_mapPinDataArray.emplace_back();
		arg_mapPinDataArray.back().m_order = order;
		arg_mapPinDataArray.back().m_part = m_gimmickArray.back();
	}
	//�X�^�[�R�C��
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::STAR_COIN))
	{
		m_gimmickArray.emplace_back(std::make_shared<StarCoin>(model, transform));
		newPart = m_gimmickArray.back().get();
		m_starCoinArray.emplace_back(std::dynamic_pointer_cast<StarCoin>(m_gimmickArray.back()));
	}
	//�w�i
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::BACKGROUND))
	{
		m_gimmickArray.emplace_back(std::make_shared<BackGround>(model, transform));
		newPart = m_gimmickArray.back().get();
	}
	//�`�r��
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::MINI_BUG))
	{
		//�p�����[�^���Ȃ�
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "Loop"))return;

		std::vector<KuroEngine::Vec3<float>>translationArray;

		bool isLoopFlag = arg_json["Loop"].get<int>();
		if (LoadTranslationArray(arg_fileName, &translationArray, obj))
		{
			m_enemyArray.emplace_back(std::make_shared<MiniBug>(model, transform, translationArray, isLoopFlag));
			newPart = m_enemyArray.back().get();
		}
	}
	//�h�b�X�������O
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::DOSSUN_RING))
	{
		//�p�����[�^���Ȃ�
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "ATKPat"))return;

		ENEMY_ATTACK_PATTERN attackPattern;

		auto patternName = arg_json["ATKPat"].get<std::string>();
		if (patternName.compare("NORMAL") == 0)attackPattern = ENEMY_ATTACK_PATTERN_NORMAL;
		else if (patternName.compare("ALWAYS") == 0)attackPattern = ENEMY_ATTACK_PATTERN_ALWAYS;
		else KuroEngine::AppearMessageBox("Stage : GetAttackPattern() ���s", "�m��Ȃ��U���p�^�[����\"" + patternName + "\"���܂܂�Ă����B");

		m_enemyArray.emplace_back(std::make_shared<DossunRing>(model, transform, attackPattern));
		newPart = m_enemyArray.back().get();
	}
	//�C��G
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::BATTERY))
	{
		//�p�����[�^���Ȃ�
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "BarrelPat"))return;
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "BulletScale"))return;

		ENEMY_BARREL_PATTERN barrelPattern;
		auto patternName = arg_json["BarrelPat"].get<std::string>();
		if (patternName.compare("FIXED") == 0)barrelPattern = ENEMY_BARREL_PATTERN_FIXED;
		else if (patternName.compare("ROCKON") == 0)barrelPattern = ENEMY_BARREL_PATTERN_ROCKON;
		else KuroEngine::AppearMessageBox("Stage : GetBarrelPattern() ���s", "�m��Ȃ��ˌ��p�^�[����\"" + patternName + "\"���܂܂�Ă����B");

		float bulletScale = arg_json["BulletScale"].get<float>();

		std::vector<KuroEngine::Vec3<float>>translationArray;
		if (LoadTranslationArray(arg_fileName, &translationArray, obj))
		{

			//X�������������Ă���G��������㉺�𔽓]������B
			if (0.9f < fabs(KuroEngine::Vec3<float>(1, 0, 0).Dot(transform.GetUp()))) {
				transform.SetRotate(DirectX::XMQuaternionMultiply(transform.GetRotate(), DirectX::XMQuaternionRotationAxis(transform.GetFront(), DirectX::XM_PI)));
			}

			m_enemyArray.emplace_back(std::make_shared<Battery>(model, transform, translationArray, bulletScale, barrelPattern));
			newPart = m_enemyArray.back().get();
		}
	}
	else
	{
		AppearMessageBox("Warning : Stage::LoadWithType()", "�X�e�[�W�p�[�c�̓ǂݍ��ݒ��ɒm��Ȃ���ʃL�[ \"" + typeKey + "\"�����������Ǒ��v�H");
	}

	//�q���̃p�[�c�ǂݍ���
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

	//�������ꓯ�m�̓����蔻����s���B
	for (auto& gimmickA : m_gimmickArray) {

		//�������ꂶ��Ȃ������珈�����΂��B
		if (gimmickA->GetType() != StageParts::STAGE_PARTS_TYPE::MOVE_SCAFFOLD) continue;

		for (auto& gimmickB : m_gimmickArray) {

			//�������ꂶ��Ȃ������珈�����΂��B
			if (gimmickB->GetType() != StageParts::STAGE_PARTS_TYPE::MOVE_SCAFFOLD) continue;

			//�����I�u�W�F�N�g�������珈�����΂��B
			if (gimmickA == gimmickB) continue;

			//�����蔻����s���B
			auto moveScaffoldA = dynamic_pointer_cast<MoveScaffold>(gimmickA);
			auto moveScaffoldB = dynamic_pointer_cast<MoveScaffold>(gimmickB);
			std::optional<AABB::CollisionInfo> result;

			//���ׂẴ��b�V���𑖍����ē����蔻����s���B
			for (auto& meshA : moveScaffoldA->m_collider.m_aabb) {
				for (auto& aabbA : meshA) {

					for (auto& meshB : moveScaffoldB->m_collider.m_aabb) {
						for (auto& aabbB : meshB) {

							result = aabbA.CheckAABBCollision(aabbB);
							if (!result) continue;
							//�ɏ��̌덷�͖�������B
							if (result->m_pushBack.Length() < 0.001f) continue;

							//�������Ă�����M�~�b�N�̓������~�߂�B
							moveScaffoldA->Stop();
							moveScaffoldB->Stop();

							////�����߂��B
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
	//���o�[�N���A
	if (m_goalLeverID != Lever::INVALID_ID)return m_goalSwitch->IsBooting();

	//�ړI�n���B
	if (m_goalPoint)return m_goalPoint->HitPlayer();

	//�N���A�����݂��Ȃ�
	return false;
}

void Stage::Load(int arg_ownStageIdx, std::string arg_dir, std::string arg_fileName, float arg_terrianScaling, bool arg_hasGoal)
{
	using namespace KuroEngine;

	m_terrianScaling = arg_terrianScaling;

	//�n�`���N���A
	m_terrianArray.clear();
	m_gimmickArray.clear();

	JsonData jsonData(arg_dir, arg_fileName);

	//�X�e�[�W���łȂ�
	if (!CheckJsonKeyExist(arg_fileName, jsonData.m_jsonData, "stage"))return;

	//�}�b�v�s�����w���n�_�̃f�[�^�z��
	std::vector<MapPinPointData>mapPinPointDataArray;

	auto stageJsonData = jsonData.m_jsonData["stage"];
	for (auto& obj : stageJsonData["objects"])
	{
		LoadWithType(arg_fileName, obj, nullptr, mapPinPointDataArray);
	}

	//�}�b�v�s���f�[�^�z������Ԓʂ�Ƀ\�[�g
	std::sort(mapPinPointDataArray.begin(), mapPinPointDataArray.end(), [](MapPinPointData& a, MapPinPointData& b)
		{
			return a.m_order < b.m_order;
		});

	//�}�b�v�s���n�_���L�^
	for (int pinIdx = 0; pinIdx < static_cast<int>(mapPinPointDataArray.size()); ++pinIdx)
	{
		//�����ɔ�肪����ꍇ�x��
		if (pinIdx != static_cast<int>(mapPinPointDataArray.size()) - 1 && mapPinPointDataArray[pinIdx].m_order == mapPinPointDataArray[pinIdx + 1].m_order)
		{
			AppearMessageBox("Stage : Load() �x��", arg_fileName + "�� CheckPointOrder �̐���������Ă邯�Ǒ��v�H");
		}
		m_mapPinPoint.emplace_back(mapPinPointDataArray[pinIdx].m_part);
	}

	//�X�^�[�g�n�_�����邩
	if (arg_ownStageIdx == 0 && !m_startPoint)
	{
		AppearMessageBox("Stage : Load() �x��", arg_fileName + " �ɃX�^�[�g�n�_�̏�񂪂Ȃ���B");
	}
	//�S�[�������邩
	if (arg_hasGoal && !m_goalPoint && m_goalLeverID == Lever::INVALID_ID)
	{
		AppearMessageBox("Stage : Load() �x��", arg_fileName + "�ɃS�[���̏�񂪂Ȃ���B");
	}

	//���o�[�ƃX�C�b�`�̊֌W�\�z�i�S�[���j
	if (m_goalLeverID != Lever::INVALID_ID)
	{
		m_goalSwitch = std::make_shared<Switch>();
		m_goalSwitch->m_leverID = m_goalLeverID;

		std::vector<std::weak_ptr<Lever>>goalLeverArray;
		for (auto& gimmick : m_gimmickArray)
		{
			//���o�[����Ȃ�
			if (gimmick->GetType() != StageParts::LEVER)continue;

			//���o�[�̃|�C���^�ɕϊ�
			auto lever = dynamic_pointer_cast<Lever>(gimmick);

			//���o�[�̎��ʔԍ����قȂ�
			if (lever->m_id != m_goalLeverID)continue;

			//�֌W�\�z
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