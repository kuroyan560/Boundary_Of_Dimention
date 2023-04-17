#include "Stage.h"
#include"FrameWork/Importer.h"
#include"ForUser/JsonData.h"
#include"FrameWork/Importer.h"
#include"ForUser/Object/Model.h"
#include "StageParts.h"
#include<cmath>

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

bool Stage::LoadMoveScaffold(std::string arg_fileName, std::shared_ptr<StageParts> *arg_result, nlohmann::json arg_json, std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
{
	using namespace KuroEngine;

	if (!CheckJsonKeyExist(arg_fileName, arg_json, "translationArray"))return false;

	std::vector<Vec3<float>>translationArray;
	for (auto &transformObj : arg_json["translationArray"])
	{
		translationArray.emplace_back();
		//���s�ړ�
		translationArray.back() = { -(float)transformObj["translation"][0],(float)transformObj["translation"][2],-(float)transformObj["translation"][1] };
		translationArray.back() *= m_terrianScaling;
	}

	*arg_result = std::make_shared<MoveScaffold>(arg_model, arg_initTransform, translationArray);

	return true;
}

void Stage::LoadWithType(std::string arg_fileName, std::string arg_typeKey, nlohmann::json arg_json)
{
	using namespace KuroEngine;

	auto &obj = arg_json;

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
		m_goalPoint = std::make_shared<GoalPoint>(model, transform);
	}
	//�����������̃I�u�W�F�N�g
	else if (arg_typeKey == StageParts::GetTypeKeyOnJson(StageParts::APPEARANCE))
	{
		m_gimmickArray.emplace_back(std::make_shared<Appearance>(model, transform));
	}
	//�M�~�b�N
	else if (arg_typeKey == StageParts::GetTypeKeyOnJson(StageParts::MOVE_SCAFFOLD))
	{
		std::shared_ptr<StageParts>gimmick;
		if (LoadMoveScaffold(arg_fileName, &gimmick, obj, model, transform))
		{
			m_gimmickArray.emplace_back(gimmick);
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

void Stage::GimmickInit()
{
	for (auto &gimmick : m_gimmickArray)
	{
		gimmick->Init();
	}
}

void Stage::GimmickUpdate(Player &arg_player)
{
	for (auto &gimmick : m_gimmickArray)
	{
		gimmick->Update(arg_player);
	}
}

void Stage::TerrianDraw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	for (auto &terrian : m_terrianArray)
	{
		terrian.Draw(arg_cam, arg_ligMgr);
	}

	for (auto &gimmick : m_gimmickArray)
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
	if (!CheckJsonKeyExist(arg_fileName, jsonData.m_jsonData, "stage"))return;

	auto stageJsonData = jsonData.m_jsonData["stage"];
	for (auto &obj : stageJsonData["objects"])
	{
		//��ʂ̃p�����[�^���Ȃ�
		if (!CheckJsonKeyExist(arg_fileName, obj, "type"))break;

		//���f���̖��O�̃p�����[�^���Ȃ�
		if (!CheckJsonKeyExist(arg_fileName, obj, "file_name"))break;

		//�g�����X�t�H�[���̃p�����[�^���Ȃ�
		if (!CheckJsonKeyExist(arg_fileName, obj, "transform"))break;

		LoadWithType(arg_fileName, obj["type"].get<std::string>(), obj);
	}

	//�X�^�[�g�n�_�����邩
	if (!m_startPoint)
	{
		AppearMessageBox("Stage : Load() �x��", arg_fileName + " �ɃX�^�[�g�n�_�̏�񂪂Ȃ���B");
	}
	//�S�[���n�_�����邩
	if (arg_hasGoal && !m_goalPoint)
	{
		AppearMessageBox("Stage : Load() �x��", arg_fileName + "�ɃS�[���n�_�̏�񂪂Ȃ���B");
	}
}