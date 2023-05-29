#pragma once
#include<array>
#include<memory>
#include"ForUser/Timer.h"
#include"InGameUI.h"

namespace KuroEngine
{
	class TextureBuffer;
}

class OperationInfoUI : public InGameUI
{
	//���o�X�e�[�^�X
	enum STATUS { APPEAR, DRAW, DISAPPEAR, STATUS_NUM }m_status;
	//���o���Ԍv��
	KuroEngine::Timer m_timer;

	//����\�L�̃x�[�X
	std::shared_ptr<KuroEngine::TextureBuffer>m_opeBaseTex;

	//���͂��Ă��邩�ǂ���
	enum INPUT_STATUS { ON, OFF, INPUT_STATUS_NUM };
	enum BUTTON { X, LT, RT, BUTTON_NUM, };
	//���͂��Ă���Ƃ��Ƃ��Ă��Ȃ��Ƃ��̉摜�e�Q����
	std::array<std::array<std::shared_ptr<KuroEngine::TextureBuffer>, INPUT_STATUS_NUM>, BUTTON_NUM>m_opeButtonTexArray;

	//�{�^���\�L�̍��W�I�t�Z�b�gX
	float m_opeButtonOffsetX = 0.0f;
	//�{�^���\�L�̃A���t�@
	float m_opeButtonAlpha = 1.0f;

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
	OperationInfoUI();
	void Init();
	void Update(float arg_timeScale);
	void Draw();
};

