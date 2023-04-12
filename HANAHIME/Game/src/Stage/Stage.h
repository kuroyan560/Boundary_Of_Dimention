#pragma once
#include"Common/Transform.h"
#include"../../../../src/engine/Render/RenderObject/ModelInfo/ModelMesh.h"
#include<vector>
#include"StageParts.h"
#include"json.hpp"
#include<memory>

namespace KuroEngine
{
	class Model;
	class TextureBuffer;
	class LightManager;
	class Camera;
}

class Stage
{
private:
	//�n�`�̃X�P�[��
	float m_terrianScaling = 1.0f;

	//�n�`���z��
	std::vector<Terrian>m_terrianArray;
	//�M�~�b�N�z��i�v�f�̃T�C�Y���قȂ邽��list�𗘗p�j
	std::list<std::shared_ptr<StageParts>>m_gimmickArray;
	//�X�^�[�g�n�_
	std::shared_ptr<StartPoint>m_startPoint;
	//�S�[���n�_
	std::shared_ptr<GoalPoint>m_goalPoint;

//���f��
	//�n�`���f���̑��݂���f�B���N�g��
	static std::string s_terrianModelDir;
	//�X�J�C�h�[��
	std::shared_ptr<KuroEngine::Model>m_skydomeModel;
	//�X�щ~��
	std::shared_ptr<KuroEngine::Model>m_woodsCylinderModel;

//�摜
	//�n��
	std::shared_ptr<KuroEngine::TextureBuffer>m_groundTex;

	//�L�[��json�t�@�C���Ɋ܂܂�Ă��邩�A�܂܂�Ă��Ȃ�������G���[�ŏI��
	bool CheckJsonKeyExist(std::string arg_fileName, nlohmann::json arg_json, std::string arg_key);

	//��������̓ǂݍ���
	bool LoadMoveScaffold(std::string arg_fileName,
		std::shared_ptr<StageParts>* arg_result, 
		nlohmann::json arg_json,
		std::weak_ptr<KuroEngine::Model>arg_model,
		KuroEngine::Transform arg_initTransform);

	//��ʂɉ����ēǂݍ��݂𕪊򂳂���
	void LoadWithType(std::string arg_fileName, std::string arg_typeKey, nlohmann::json arg_json);

public:
	Stage();

	
	/// <summary>
	/// �n�`�̃g�����X�t�H�[��������
	/// </summary>
	void TerrianInit();

	//�n�`�̕`��
	void TerrianDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//�X�e�[�W���ǂݍ���
	void Load(std::string arg_dir, std::string arg_fileName, float arg_terrianScaling, bool arg_hasGoal = true);

	//�ʏ�̒n�`�̔z��擾
	const std::vector<Terrian>& GetTerrianArray()const { return m_terrianArray; }

	//�v���C���[�̏������g�����X�t�H�[��
	KuroEngine::Transform GetPlayerSpawnTransform()const
	{
		if (m_startPoint)return m_startPoint->GetTransform();
		return KuroEngine::Transform();
	}

//���f���̃Q�b�^
	//�X�J�C�h�[��
	std::weak_ptr<KuroEngine::Model>GetSkydomeModel() { return m_skydomeModel; }
	//�X�щ~��
	std::weak_ptr<KuroEngine::Model>GetWoodsCylinderModel() { return m_woodsCylinderModel; }
	//�n��
	std::weak_ptr<KuroEngine::TextureBuffer>GetGroundTex() { return m_groundTex; }

private:
};