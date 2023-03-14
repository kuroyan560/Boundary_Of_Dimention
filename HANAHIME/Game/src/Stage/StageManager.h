#pragma once
#include"Common/Singleton.h"
#include"ForUser/Debugger.h"

#include<memory>
namespace KuroEngine
{
	class Model;
	class Camera;
}

class Stage;

class StageManager : public KuroEngine::DesignPattern::Singleton<StageManager>,public KuroEngine::Debugger
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

	////�X�J�C�h�[���̑傫��
	//float m_skydomeScaling = 500.0f;
	////�X�щ~���̔��a
	//float m_woodsRadius = 309.0f;
	////�X�щ~���̍���
	//float m_woodsHeight = 186.0f;
	////�n�ʂ̑傫��
	//float m_groundScaling = 311.4f;

	//�f�o�b�O�p�e�X�g�X�e�[�W
	std::shared_ptr<Stage>m_testStage;

	//���݂̃X�e�[�W
	std::shared_ptr<Stage>m_nowStage;

	//Imgui�f�o�b�O�֐��I�[�o�[���C�h
	void OnImguiItems()override;

public:
	void Draw(KuroEngine::Camera& arg_cam);
};

