#include "Stage.h"
#include"FrameWork/Importer.h"
#include"ForUser/JsonData.h"
#include"FrameWork/Importer.h"
#include"ForUser/Object/Model.h"
#include "StageParts.h"
#include"Switch.h"

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

bool Stage::LoadMoveScaffold(std::string arg_fileName, std::shared_ptr<StageParts>* arg_result, nlohmann::json arg_json, std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
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
		translationArray.back() = { -(float)jsonArray[key][0],(float)jsonArray[key][2],-(float)jsonArray[key][1] };
		translationArray.back() *= m_terrianScaling;

		key = "translation_" + std::to_string(++idx);
	}

	*arg_result = std::make_shared<MoveScaffold>(arg_model, arg_initTransform, translationArray);

	return true;
}

void Stage::LoadWithType(std::string arg_fileName, std::string arg_typeKey, nlohmann::json arg_json)
{
	using namespace KuroEngine;

	auto& obj = arg_json;

//���ʃp�����[�^
	//���f���ݒ�
	auto model = Importer::Instance()->LoadModel(s_terrianModelDir, obj["file_name"].get<std::string>() + ".glb");

	//�g�����X�t�H�[���擾
	auto transformObj = obj["transform"];

	//���s�ړ�
	Vec3<float>translation = { -(float)transformObj["translation"][0],(float)transformObj["translation"][2],-(float)transformObj["translation"][1] };

	//��]
	XMVECTOR quaternion = { (float)transformObj["rotation"][0],(float)transformObj["rotation"][2], -(float)transformObj["rotation"][1],(float)transformObj["rotation"][3] };

	//�X�P�[�����O
	Vec3<float>scaling = { (float)transformObj["scaling"][0],(float)transformObj["scaling"][2] ,(float)transformObj["scaling"][1] };

	//�g�����X�t�H�[���ݒ�
	Transform transform;
	transform.SetPos(translation * m_terrianScaling);
	transform.SetRotate(quaternion);
	transform.SetScale(scaling * m_terrianScaling);

//��ʂɉ����ĕς��p�����[�^
	//�ʏ�̒n�`
	if (arg_typeKey == StageParts::GetTypeKeyOnJson(StageParts::TERRIAN))
	{
		m_terrianArray.emplace_back(model, transform);
	}
	//�X�^�[�g�n�_
	else if (arg_typeKey == StageParts::GetTypeKeyOnJson(StageParts::START_POINT))
	{
		transform.SetScale(1.0f);
		m_startPoint = std::make_shared<StartPoint>(model, transform);
	}
	//�S�[���n�_
	else if (arg_typeKey == StageParts::GetTypeKeyOnJson(StageParts::GOAL_POINT))
	{
		//�S�Ẵ��o�[���I���ɂ��邱�Ƃ��N���A����
		if (obj.contains("leverID"))
		{
			m_goalLeverID = obj["leverID"];
		}
		//�ړI�n�ɓ��B���邱�Ƃ��N���A����
		else
		{
			m_goalPoint = std::make_shared<GoalPoint>(model, transform);
		}
	}
	//�����������̃I�u�W�F�N�g
	else if (arg_typeKey == StageParts::GetTypeKeyOnJson(StageParts::APPEARANCE))
	{
		m_gimmickArray.emplace_back(std::make_shared<Appearance>(model, transform));
	}
	//��������
	else if (arg_typeKey == StageParts::GetTypeKeyOnJson(StageParts::MOVE_SCAFFOLD))
	{
		std::shared_ptr<StageParts>gimmick;
		if (LoadMoveScaffold(arg_fileName, &gimmick, obj, model, transform))
		{
			m_gimmickArray.emplace_back(gimmick);
		}
	}
	//���o�[
	else if (arg_typeKey == StageParts::GetTypeKeyOnJson(StageParts::LEVER))
	{
		//�K�v�ȃp�����[�^���Ȃ�
		if (!CheckJsonKeyExist(arg_fileName, arg_json, "id") || !CheckJsonKeyExist(arg_fileName, arg_json, "initFlg"))return;

		m_gimmickArray.emplace_back(std::make_shared<Lever>(model, transform, arg_json["id"], arg_json["initFlg"]));
	}
	else
	{
		AppearMessageBox("Warning : Stage::LoadWithType()", "�X�e�[�W�p�[�c�̓ǂݍ��ݒ��ɒm��Ȃ���ʃL�[ \"" + arg_typeKey + "\"�����������Ǒ��v�H");
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

void Stage::GimmickInit()
{
	for (auto& gimmick : m_gimmickArray)
	{
		gimmick->Init();
	}
}

void Stage::GimmickUpdate(Player& arg_player)
{
	for (auto& gimmick : m_gimmickArray)
	{
		gimmick->Update(arg_player);
	}

	if (m_goalPoint)m_goalPoint->Update(arg_player);
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

void Stage::TerrianDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
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

void Stage::Load(std::string arg_dir, std::string arg_fileName, float arg_terrianScaling, bool arg_hasGoal)
{
	using namespace KuroEngine;

	m_terrianScaling = arg_terrianScaling;

	//�n�`���N���A
	m_terrianArray.clear();
	m_gimmickArray.clear();

	JsonData jsonData(arg_dir, arg_fileName);

	//�X�e�[�W���łȂ�
	if (!CheckJsonKeyExist(arg_fileName,jsonData.m_jsonData, "stage"))return;

	auto stageJsonData = jsonData.m_jsonData["stage"];
	for (auto& obj : stageJsonData["objects"])
	{
		//��ʂ̃p�����[�^���Ȃ�
		if (!CheckJsonKeyExist(arg_fileName,obj, "type"))break;

		//���f���̖��O�̃p�����[�^���Ȃ�
		if (!CheckJsonKeyExist(arg_fileName,obj, "file_name"))break;

		//�g�����X�t�H�[���̃p�����[�^���Ȃ�
		if (!CheckJsonKeyExist(arg_fileName,obj, "transform"))break;

		LoadWithType(arg_fileName, obj["type"].get<std::string>(), obj);
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