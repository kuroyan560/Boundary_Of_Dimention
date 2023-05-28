#pragma once
#include<vector>
#include<array>
#include<memory>
#include"ForUser/Timer.h"
#include"InGameUI.h"

namespace KuroEngine
{
	class TextureBuffer;
}

class StageInfoUI : public InGameUI
{
	//���o�X�e�[�^�X
	enum STATUS { APPEAR, DRAW, DISAPPEAR, STATUS_NUM }m_status;
	//���o���Ԍv��
	KuroEngine::Timer m_timer;

	//�X�e�[�W���摜
	std::vector<std::shared_ptr<KuroEngine::TextureBuffer>>m_stageNameTex;
	std::shared_ptr<KuroEngine::TextureBuffer>m_stageNameDefaultTex;
	//�X�e�[�W���̑��������摜
	std::shared_ptr<KuroEngine::TextureBuffer>m_underLineTex;
	//�X�e�[�W���̃C���f�b�N�X
	int m_stageNameIdx = 0;

	//�Ԃ̉摜�i���j
	std::shared_ptr<KuroEngine::TextureBuffer>m_miniFlowerTex;
	//���W�����Ԃ̐����e�N�X�`���̃T�C�Y
	static const int FLOWER_NUM_TEX_SIZE = 12;
	//�u + �v
	static const int FLOWER_NUM_PLUS_IDX = 11;
	//�u / �v
	static const int FLOWER_NUM_SLASH_IDX = 10;
	//���W�����Ԃ̐���
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, FLOWER_NUM_TEX_SIZE>m_flowerNumTex;

	//���W�I�t�Z�b�gX
	float m_offsetX = 0.0f;
	//�A���t�@
	float m_alpha = 1.0f;

	//�O�t���[���ł̉Ԃ̏�����
	int m_oldGetFlowerNum;
	//�Ԃ̎��W�ʑ���UI���o�̃^�C�}�[
	KuroEngine::Timer m_addFlowerNumTimer;
	//�Ԃ̎��W�ʑ���UI���o
	int m_addFlowerNum = 0;
	//�ԑ���UI�A���t�@
	float m_addFlowerAlpha = 1.0f;
	//�ԑ���UI���W�I�t�Z�b�gY
	float m_addFlowerOffsetY = 0.0f;

	//�����삩�炵�΂炭�o�������Ă���
	KuroEngine::Timer m_idleTimer;
	//�ޏ�̗\��i�A�C�h�����Ԃ̂Ƃ��j
	bool m_disappearCall = false;

	void SetUIStatus(STATUS arg_status);

	void Appear()override;
	void Disappear()override;
	bool IsAppeared()override { return m_status == DRAW; }
	bool IsDisappeared()override { return m_status == DISAPPEAR && m_timer.IsTimeUp(); }

public:
	StageInfoUI();
	void Init(int arg_stageNum, int arg_getFlowerNum);
	void Update(float arg_timeScale, int arg_getFlowerNum);
	void Draw(int arg_existFlowerNum, int arg_getFlowerNum);
};

