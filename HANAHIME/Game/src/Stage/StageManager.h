#pragma once
#include"Common/Singleton.h"
#include"ForUser/Debugger.h"
#include"Common/Transform.h"
#include"StageParts.h"
#include"../HUD/MapPinUI.h"
#include"../HUD/CheckPointUI.h"
#include"../Effect/GuideInsect.h"

#include<memory>
namespace KuroEngine
{
	class Model;
	class Camera;
	class LightManager;
}

class Stage;
class Player;

class StageManager : public KuroEngine::DesignPattern::Singleton<StageManager>, public KuroEngine::Debugger
{
	friend class KuroEngine::DesignPattern::Singleton<StageManager>;
	StageManager();

	//�X�J�C�h�[���̑傫��
	float m_skydomeScaling = 1.0f;

	//�z�[���X�e�[�W
	std::shared_ptr<Stage>m_homeStage;

	//�f�o�b�O�p�e�X�g�X�e�[�W
	std::vector<std::shared_ptr<Stage>>m_stageArray;

	//���݂̃X�e�[�W
	std::shared_ptr<Stage>m_nowStage;
	int m_nowStageIdx;

	//�}�b�v�s��UI
	MapPinUI m_mapPinUI;
	//���݃}�b�v�s�����w���n�_�̃C���f�b�N�X
	int m_nowMapPinPointIdx;
public:
	void SetStage(int stage_num = -1);

	void Update(Player& arg_player);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);
	void DrawUI(KuroEngine::Camera& arg_cam, KuroEngine::Vec3<float>arg_playerPos);

	//���݂̃X�e�[�W�̃Q�b�^
	std::weak_ptr<Stage>GetNowStage() { return m_nowStage; }
	const int& GetNowStageIdx()const{ return m_nowStageIdx; }

	KuroEngine::Transform GetGateTransform(int arg_stageIdx, int arg_gateID)const;

	//�X�e�[�W�̐�
	int GetAllStageNum()
	{
		return static_cast<int>(m_stageArray.size());
	}

	//�N���A����
	bool IsClearNowStage()const;

	//�v���C���[�̏������g�����X�t�H�[��
	KuroEngine::Transform GetStartPointTransform()const;

	bool GetNowMapPinTransform(KuroEngine::Transform* arg_destPos);

	KuroEngine::Transform GetGoalTransform()const;

	std::shared_ptr<GoalPoint>GetGoalModel();

	//���肵���X�^�[�R�C���̐�
	int GetStarCoinNum()const;
	//���݂���X�^�[�R�C���̐�
	int ExistStarCoinNum()const;

	//�X�J�C�h�[���̃X�P�[�����O�Q�b�^
	const float& GetSkyDomeScaling()const { return m_skydomeScaling; }

	//����ς̃`�F�b�N�|�C���g�̃g�����X�t�H�[���z��
	//std::vector<std::vector<KuroEngine::Transform>>GetUnlockedCheckPointTransformArray()const;
	bool GetUnlockedCheckPointInfo(std::vector<std::vector<KuroEngine::Transform>>* arg_transformArray, int* arg_recentStageNum, int* arg_recentIdx)const;
};