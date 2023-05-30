#pragma once
#include"Common/Transform.h"
#include"Render/RenderObject/ModelInfo/ModelMesh.h"
#include<vector>
#include"StageParts.h"
#include"json.hpp"
#include<memory>
#include<map>
#include"../Effect/GuideInsect.h"
#include"../GPUParticle/GPUParticleRender.h"
#include"FunaCameraPoint.h"

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
	//�G�z��
	std::list<std::shared_ptr<StageParts>>m_enemyArray;
	//�X�^�[�g�n�_
	std::shared_ptr<StartPoint>m_startPoint;
	//�S�[���n�_
	std::shared_ptr<GoalPoint>m_goalPoint;
	//�Q�[�g�z��
	std::vector<std::weak_ptr<Gate>>m_gateArray;
	//�`�F�b�N�|�C���g�z��
	std::vector<std::weak_ptr<CheckPoint>>m_checkPointArray;

	//�}�b�v�s�����w���p�[�c�̃f�[�^�̈ꎞ�i�[�p
	struct MapPinPointData
	{
		std::weak_ptr<StageParts>m_part;
		int m_order;	//����
	};
	//�}�b�v�s�����w���p�[�c�̃f�[�^�z��
	std::vector<std::weak_ptr<StageParts>>m_mapPinPoint;
	//�S�Ă̖ړI�n�������
	bool m_isCompleteMapPin = false;

	//�X�^�[�R�C���̃|�C���^�z��
	std::vector<std::weak_ptr<StarCoin>>m_starCoinArray;

	//FunaCamera�̔z��
	std::vector<FunaCameraPoint>m_funaCamPtArray;

	//���f��
		//�n�`���f���̑��݂���f�B���N�g��
	static std::string s_stageModelDir;

	//�摜
		//�n��
	std::shared_ptr<KuroEngine::TextureBuffer>m_groundTex;

	std::shared_ptr<KuroEngine::Model>m_skydomeModel;

	std::shared_ptr<KuroEngine::Model>m_woodsCylinderModel;

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
		std::vector<KuroEngine::Vec3<float>> *arg_result,
		nlohmann::json arg_json);

	//��ʂɉ����ēǂݍ��݂𕪊򂳂���
	void LoadWithType(std::string arg_fileName, nlohmann::json arg_json, StageParts *arg_parent, std::vector<MapPinPointData> &arg_mapPinDataArray);

public:
	Stage();

	void Init();
	void Update(Player &arg_player);
	void Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr);

	//�N���A����
	bool IsClear()const;

	//�X�e�[�W���ǂݍ���
	void Load(int arg_ownStageIdx, std::string arg_dir, std::string arg_fileName, float arg_terrianScaling, bool arg_hasGoal = true);

	//�ʏ�̒n�`�̔z��擾
	const std::vector<Terrian> &GetTerrianArray()const { return m_terrianArray; }
	//�M�~�b�N�z��擾
	std::list<std::shared_ptr<StageParts>> &GetGimmickArray() { return m_gimmickArray; }

	//���肵���X�^�[�R�C���̐�
	int GetStarCoinNum()const;
	//���݂���X�^�[�R�C���̐�
	int ExistStarCoinNum()const { return static_cast<int>(m_starCoinArray.size()); }

	//�Q�[�g��ID����Q�[�g�̃g�����X�t�H�[�����擾
	KuroEngine::Transform GetGateTransform(int arg_gateID)const;

	//�v���C���[�̏������g�����X�t�H�[���Q�b�^
	KuroEngine::Transform GetStartPointTransform()const
	{
		if (m_startPoint)return m_startPoint->GetTransform();
		return KuroEngine::Transform();
	}

	//�S�[���̃g�����X�t�H�[���Q�b�^
	KuroEngine::Transform GetGoalTransform()const
	{
		if (m_goalPoint)return m_goalPoint->GetTransform();
		return KuroEngine::Transform();
	};

	std::shared_ptr<GoalPoint>GetGoalModel()
	{
		return m_goalPoint;
	}

	void SetCompleteMapPinFlg(bool arg_flg) { m_isCompleteMapPin = arg_flg; }
	const bool &GetCompleteMapPin()const { return m_isCompleteMapPin; }

	//���f���̃Q�b�^
		//�X�J�C�h�[��
	std::weak_ptr<KuroEngine::Model>GetSkydomeModel() { return m_skydomeModel; }
	//�X�щ~��
	std::weak_ptr<KuroEngine::Model>GetWoodsCylinderModel() { return m_woodsCylinderModel; }
	//�n��
	std::weak_ptr<KuroEngine::TextureBuffer>GetGroundTex() { return m_groundTex; }

	//�}�b�v�s�����w���p�[�c�̔z��
	std::vector<std::weak_ptr<StageParts>>GetMapPinPointArray()const { return m_mapPinPoint; }
	//�`�F�b�N�|�C���g�z��
	std::vector<std::weak_ptr<CheckPoint>>GetCheckPointArray()const { return m_checkPointArray; }
	//FunaCameraPoint�z��
	std::vector<FunaCameraPoint>GetFunaCamPtArray()const { return m_funaCamPtArray; }

	//�K�C�h�p�̒�
	GuideInsect m_guideInsect;

	void CheckPointReset();
};