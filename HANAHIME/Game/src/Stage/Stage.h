#pragma once
#include"Common/Transform.h"
#include"../../../../src/engine/Render/RenderObject/ModelInfo/ModelMesh.h"
#include<vector>
#include"StageParts.h"
#include"json.hpp"
#include<memory>
#include<map>

namespace KuroEngine
{
	class Model;
	class TextureBuffer;
	class LightManager;
	class Camera;
}

class Switch;

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

	//�S�Ă��I����Ԃɂ��邱�Ƃ��N���A�����ƂȂ郌�o�[�̎��ʔԍ�
	int m_goalLeverID = Lever::INVALID_ID;
	//�N���A�̃X�C�b�`
	std::shared_ptr<Switch>m_goalSwitch;

	//�L�[��json�t�@�C���Ɋ܂܂�Ă��邩�A�܂܂�Ă��Ȃ�������G���[�ŏI��
	bool CheckJsonKeyExist(std::string arg_fileName, nlohmann::json arg_json, std::string arg_key);

	//���W�n���l���������W�ǂݍ���
	KuroEngine::Vec3<float>GetConsiderCoordinate(nlohmann::json arg_json);

	//���W�z��̓ǂݍ���
	bool LoadTranslationArray(std::string arg_fileName,
		std::vector<KuroEngine::Vec3<float>>* arg_result,
		nlohmann::json arg_json);

	//��ʂɉ����ēǂݍ��݂𕪊򂳂���
	void LoadWithType(std::string arg_fileName, std::string arg_typeKey, nlohmann::json arg_json);

public:
	Stage();
	
	/// <summary>
	/// �n�`�̃g�����X�t�H�[��������
	/// </summary>
	void GimmickInit();

	void GimmickUpdate(Player& arg_player);

	//�N���A����
	bool IsClear()const;

	//�n�`�̕`��
	void TerrianDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//�X�e�[�W���ǂݍ���
	void Load(std::string arg_dir, std::string arg_fileName, float arg_terrianScaling, bool arg_hasGoal = true);

	//�ʏ�̒n�`�̔z��擾
	const std::vector<Terrian>& GetTerrianArray()const { return m_terrianArray; }
	std::list<std::shared_ptr<StageParts>>& GetGimmickArray(){ return m_gimmickArray; }

	//�v���C���[�̏������g�����X�t�H�[��
	KuroEngine::Transform GetPlayerSpawnTransform()const
	{
		if (m_startPoint)return m_startPoint->GetTransform();
		return KuroEngine::Transform();
	}

	KuroEngine::Transform GetGoalTransform()const
	{
		if (m_goalPoint)return m_goalPoint->GetTransform();
		return KuroEngine::Transform();
	};

	std::shared_ptr<GoalPoint>GetGoalModel()
	{
		return m_goalPoint;
	}

//���f���̃Q�b�^
	//�X�J�C�h�[��
	std::weak_ptr<KuroEngine::Model>GetSkydomeModel() { return m_skydomeModel; }
	//�X�щ~��
	std::weak_ptr<KuroEngine::Model>GetWoodsCylinderModel() { return m_woodsCylinderModel; }
	//�n��
	std::weak_ptr<KuroEngine::TextureBuffer>GetGroundTex() { return m_groundTex; }


};