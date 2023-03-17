#include "Stage.h"
#include"FrameWork/Importer.h"
#include"ForUser/JsonData.h"
#include"FrameWork/Importer.h"

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

		//�n�`�ǉ�
		m_terrianArray.emplace_back();
		//�V�����n�`���̎Q��
		auto& newTerrian = m_terrianArray.back();

		//�n�`�̖��O�ݒ�
		newTerrian.m_name = obj["name"].get<std::string>();

		//���f���ݒ�
		newTerrian.m_model = Importer::Instance()->LoadModel(s_terrianModelDir, obj["file_name"].get<std::string>() + ".glb");

		//�g�����X�t�H�[���擾
		auto transform = obj["transform"];

		//���s�ړ�
		Vec3<float>translation = { -(float)transform["translation"][0],(float)transform["translation"][2],-(float)transform["translation"][1] };

		//��]
		Vec3<float>rotate = { -(float)transform["rotation"][1],-(float)transform["rotation"][2], (float)transform["rotation"][0] };
		//���W�A���ɒ���
		rotate.x = Angle::ConvertToRadian(rotate.x);
		rotate.y = Angle::ConvertToRadian(rotate.y);
		rotate.z = Angle::ConvertToRadian(rotate.z);

		//�X�P�[�����O
		Vec3<float>scaling = { (float)transform["scaling"][0],(float)transform["scaling"][2] ,(float)transform["scaling"][1] };

		//�g�����X�t�H�[���ݒ�
		newTerrian.m_transform.SetPos(translation);
		newTerrian.m_transform.SetRotate(XMQuaternionRotationRollPitchYaw(rotate.z, rotate.y, rotate.x));
		newTerrian.m_transform.SetScale(scaling);
	}
}
