#pragma once
#include"Common/Singleton.h"
#include"ForUser/Debugger.h"
#include"Common/Transform.h"
#include"StageParts.h"
#include"MapPinUI.h"

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
	//�X�щ~���̔��a
	float m_woodsRadius = 1.0f;
	//�X�щ~���̍���
	float m_woodsHeight = 1.0f;
	//�n�ʂ̑傫��
	float m_groundScaling = 1.0f;

	//�z�[���X�e�[�W
	std::shared_ptr<Stage>m_homeStage;

	//�f�o�b�O�p�e�X�g�X�e�[�W
	std::vector<std::shared_ptr<Stage>>m_stageArray;

	//���݂̃X�e�[�W
	std::shared_ptr<Stage>m_nowStage;

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

	KuroEngine::Transform GetGateTransform(int arg_stageIdx, int arg_gateID)const;

	int GetAllStageNum()
	{
		return static_cast<int>(m_stageArray.size());
	}

	//�N���A����
	bool IsClearNowStage()const;

	//�v���C���[�̏������g�����X�t�H�[��
	KuroEngine::Transform GetPlayerSpawnTransform()const;


	KuroEngine::Transform GetGoalTransform()const;

	std::shared_ptr<GoalPoint>GetGoalModel();
};