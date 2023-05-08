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
		KuroEngine::AppearMessageBox("Stage : CheckJsonKeyExist() ���s", arg_fileName + " ��\"" + arg_key + "\"���܂܂�ĂȂ���B");
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

void Stage::LoadWithType(std::string arg_fileName, nlohmann::json arg_json, StageParts* arg_parent)
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
	auto model = Importer::Instance()->LoadModel(s_terrianModelDir, obj["file_name"].get<std::string>() + ".glb");

	//�g�����X�t�H�[���擾
	auto transformObj = obj["transform"];

	//���s�ړ�
	Vec3<float>translation = GetConsiderCoordinate(transformObj["translation"]);

	//��]
	XMVECTOR quaternion = { (float)transformObj["rotation"][0],(float)transformObj["rotation"][2], -(float)transformObj["rotation"][1],(float)transformObj["rotation"][3] };

	//�X�P�[�����O
	Vec3<float>scaling = { (float)transformObj["scaling"][0],(float)transformObj["scaling"][2] ,(float)transformObj["scaling"][1] };

	//�g�����X�t�H�[���ݒ�
	Transform transform;
	transform.SetPos(translation * (arg_parent == nullptr ? m_terrianScaling : 1.0f));
	transform.SetRotate(quaternion);
	transform.SetScale(scaling * (arg_parent == nullptr ? m_terrianScaling : 1.0f));

	StageParts* newPart = nullptr;

	//��ʂɉ����ĕς��p�����[�^
		//�ʏ�̒n�`
	if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::TERRIAN))
	{
		m_terrianArray.emplace_back(model, transform, arg_parent);
		newPart = &m_terrianArray[static_cast<int>(m_terrianArray.size()) - 1];
	}
	//�X�^�[�g�n�_
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::START_POINT))
	{
		transform.SetScale(1.0f);
		m_startPoint = std::make_shared<StartPoint>(model, transform, arg_parent);
		newPart = m_startPoint.get();
	}
	//�S�[���n�_
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::GOAL_POINT))
	{
		//�S�Ẵ��o�[���I���ɂ��邱�Ƃ��N���A����
		if (obj.contains("leverID") && obj["leverID"] != -1)
		{
			m_goalLeverID = obj["leverID"];
		}
		//�ړI�n�ɓ��B���邱�Ƃ��N���A����
		else
		{
			m_goalPoint = std::make_shared<GoalPoint>(model, transform, arg_parent);
		}
		newPart = m_goalPoint.get();
	}
	//�����������̃I�u�W�F�N�g
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::APPEARANCE))
	{
		m_gimmickArray.emplace_back(std::make_shared<Appearance>(model, transform, arg_parent));
		newPart = m_gimmickArray.back().get();
	}
	//��������
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::MOVE_SCAFFOLD))
	{
		std::vector<KuroEngine::Vec3<float>>translationArray;
		if (LoadTranslationArray(arg_fileName, &translationArray, obj))
		{
			m_gimmickArray.emplace_back(std::make_shared<MoveScaffold>(model, transform, arg_parent, translationArray));
		}
		newPart = m_gimmickArray.back().get();
	}
	//���o�[
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::LEVER))
	{
		//�K�v�ȃp�����[�^���Ȃ�
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "id") || !CheckJsonKeyExist(arg_fileName, arg_json, "initFlg"))return;

		m_gimmickArray.emplace_back(std::make_shared<Lever>(model, transform, arg_parent, arg_json["id"], arg_json["initFlg"]));
		newPart = m_gimmickArray.back().get();
	}
	//�W�b�v���C����
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::IVY_ZIP_LINE))
	{
		std::vector<KuroEngine::Vec3<float>>translationArray;
		if (LoadTranslationArray(arg_fileName, &translationArray, obj))
		{
			m_gimmickArray.emplace_back(std::make_shared<IvyZipLine>(model, transform, arg_parent, translationArray));
		}
		newPart = m_gimmickArray.back().get();
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

		m_gimmickArray.emplace_back(std::make_shared<IvyBlock>(model, transform, arg_parent, leftTopFront, rightBottomBack));
		newPart = m_gimmickArray.back().get();
	}
	//�`�r��
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::MINI_BUG))
	{
		//m_enemyArray.emplace_back(std::make_shared<MiniBug>(model, transform, arg_parent));
		//newPart = m_enemyArray.back().get();
	}
	//�h�b�X�������O
	else if (typeKey == StageParts::GetTypeKeyOnJson(StageParts::DOSSUN_RING))
	{
		//m_enemyArray.emplace_back(std::make_shared<DossunRing>(model, transform, arg_parent));
		//newPart = m_enemyArray.back().get();
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
			LoadWithType(arg_fileName, child, newPart);
		}
	}
}

Stage::Stage()
{
	using namespace KuroEngine;

	//�f�t�H���g�̃��f��
		//�X�J�C�h�[��
	static std::shared_ptr<Model>s_defaultSkydomeModel
		= Importer::Instance()->LoadModel("resource/user/model/", "Skydome.glb");
	//�X�щ~��
	static std::shared_ptr<Model>s_defaultWoodsCylinderModel
		= Importer::Instance()->LoadModel("resource/user/model/", "Woods.glb");

	//�f�t�H���g�̉摜
		//�n��
	static std::shared_ptr<TextureBuffer>s_defaultGroundTex
		= D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/ground.png");

	//�f�t�H���g�l�ݒ�
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

void Stage::Load(std::string arg_dir, std::string arg_fileName, float arg_terrianScaling, bool arg_hasGoal)
{
	using namespace KuroEngine;

	m_terrianScaling = arg_terrianScaling;

	//�n�`���N���A
	m_terrianArray.clear();
	m_gimmickArray.clear();

	JsonData jsonData(arg_dir, arg_fileName);

	//�X�e�[�W���łȂ�
	if (!CheckJsonKeyExist(arg_fileName, jsonData.m_jsonData, "stage"))return;

	auto stageJsonData = jsonData.m_jsonData["stage"];
	for (auto& obj : stageJsonData["objects"])
	{
		LoadWithType(arg_fileName, obj, nullptr);
	}

	//�X�^�[�g�n�_�����邩
	if (!m_startPoint)
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