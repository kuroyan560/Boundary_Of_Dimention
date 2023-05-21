#pragma once
#include<array>
#include<memory>
#include"ForUser/Timer.h"

namespace KuroEngine
{
	class TextureBuffer;
}

class OperationInfoUI
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

	void SetUIStatus(STATUS arg_status);

public:
	OperationInfoUI();
	void Init();
	void Update(float arg_timeScale);
	void Draw();
};

