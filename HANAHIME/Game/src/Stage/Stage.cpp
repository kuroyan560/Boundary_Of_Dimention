#include "Stage.h"
#include"FrameWork/Importer.h"
#include"ForUser/JsonData.h"
#include"FrameWork/Importer.h"
#include"../Graphics/BasicDraw.h"

std::string Stage::s_terrianModelDir = "resource/user/model/terrian/";

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

void Stage::TerrianInit(float arg_scaling)
{
	for (auto& terrian : m_terrianArray)
	{
		terrian.m_transform.SetPos(terrian.m_initializedTransform.GetPos() * arg_scaling);
		terrian.m_transform.SetScale(terrian.m_initializedTransform.GetScale() * arg_scaling);
		terrian.m_transform.SetRotate(terrian.m_initializedTransform.GetRotate());
	}
}

void Stage::TerrianDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	for (auto& terrian : m_terrianArray)
	{
		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			terrian.m_model.lock(),
			terrian.m_transform);
	}
}

void Stage::Load(std::string arg_dir, std::string arg_fileName)
{
	using namespace KuroEngine;

	//�n�`���N���A
	m_terrianArray.clear();

	JsonData jsonData(arg_dir, arg_fileName);
	for (auto& obj : jsonData.m_jsonData["objects"])
	{
		//�n�`�̖��O�̃p�����[�^���Ȃ�
		if (!obj.contains("name"))continue;

		//���f���̖��O�̃p�����[�^���Ȃ�
		if (!obj.contains("file_name"))continue;

		//�g�����X�t�H�[���̃p�����[�^���Ȃ�
		if (!obj.contains("transform"))continue;

		//�n�`�̖��O�ݒ�
		auto name = obj["name"].get<std::string>();

		//���f���ݒ�
		auto model = Importer::Instance()->LoadModel(s_terrianModelDir, obj["file_name"].get<std::string>() + ".glb");

		//�g�����X�t�H�[���擾
		auto transformObj = obj["transform"];

		//���s�ړ�
		Vec3<float>translation = { -(float)transformObj["translation"][0],(float)transformObj["translation"][2],-(float)transformObj["translation"][1] };

		//��]
		Vec3<float>rotate = { -(float)transformObj["rotation"][1],-(float)transformObj["rotation"][2], (float)transformObj["rotation"][0] };
		//���W�A���ɒ���
		rotate.x = Angle::ConvertToRadian(rotate.x);
		rotate.y = Angle::ConvertToRadian(rotate.y);
		rotate.z = Angle::ConvertToRadian(rotate.z);

		//�X�P�[�����O
		Vec3<float>scaling = { (float)transformObj["scaling"][0],(float)transformObj["scaling"][2] ,(float)transformObj["scaling"][1] };

		//�g�����X�t�H�[���ݒ�
		Transform transform;
		transform.SetPos(translation);
		transform.SetRotate(XMQuaternionRotationRollPitchYaw(rotate.z, rotate.y, rotate.x));
		transform.SetScale(scaling);

		//�n�`�ǉ�
		m_terrianArray.emplace_back(name, model, transform);
	}
}
