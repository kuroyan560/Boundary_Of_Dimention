#include "CheckPointUI.h"
#include"DirectX12/D3D12App.h"
#include"FrameWork/WinApp.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"

CheckPointUI::CheckPointUI()
{
	using namespace KuroEngine;

	std::string dir = "resource/user/tex/in_game/check_point/";
	m_unlockStrTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "open_str.png");
	m_accUnderLineTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "acc_under_line.png");
}

void CheckPointUI::Init()
{
	m_timer.Reset(0.0f);
}

void CheckPointUI::Update()
{
	using namespace KuroEngine;

	//�C���A���_�[���C����Y���W
	const float ACC_UNDER_LINE_Y = 222.0f;
	//�u�`�F�b�N�|�C���g��������܂����v�Ɖ����Ƃ̌��ԃI�t�Z�b�g
	const float SPACE_BETWEEN_STR_AND_LINE = 8.0f;
	//�u�`�F�b�N�|�C���g��������܂����v��Y���W
	const float STR_Y = ACC_UNDER_LINE_Y - m_accUnderLineTex->GetGraphSize().y * 0.5f - m_unlockStrTex->GetGraphSize().y * 0.5f - SPACE_BETWEEN_STR_AND_LINE;

	//���o����Y���W�I�t�Z�b�g�ő�
	const float STR_OFFSET_Y_MAX = 32.0f;

	//�o�ꎞ�Ԋ���
	const float APPEAR_TIME_RATE = 0.2f;
	//�ޏꎞ�Ԋ���
	const float DISAPPEAR_TIME_RATE = 0.8f;

	//��A�N�e�B�u
	if (m_timer.IsTimeUp())return;

	//�^�C�}�[�X�V
	m_timer.UpdateTimer();

	float strOffsetY = 0.0f;

	//�o��
	if (m_timer.GetTimeRate() < APPEAR_TIME_RATE)
	{
		float easeRate = m_timer.GetTimeRate(0.0f, APPEAR_TIME_RATE);
		m_alpha = Math::Lerp(0.0f, 1.0f, easeRate);
		strOffsetY = Math::Ease(Out, Quint, easeRate, -STR_OFFSET_Y_MAX, 0.0f);
	}
	//�ޏ�
	else if (DISAPPEAR_TIME_RATE < m_timer.GetTimeRate())
	{
		float easeRate = m_timer.GetTimeRate(DISAPPEAR_TIME_RATE, 1.0f);
		m_alpha = Math::Lerp(1.0f, 0.0f, easeRate);
		strOffsetY = Math::Ease(In, Back, easeRate, 0.0f, STR_OFFSET_Y_MAX);
	}

	//�E�B���h�E��X���W���S
	const auto winCenterX = WinApp::Instance()->GetExpandWinCenter().x;

	//�C���A���_�[���C���̍��W�Œ�
	m_accUnderLinePos = { winCenterX,ACC_UNDER_LINE_Y };
	//�u�`�F�b�N�|�C���g��������܂����v�̍��W
	m_unlockStrPos = { winCenterX,STR_Y + strOffsetY };
}

void CheckPointUI::Draw()
{
	return;

	using namespace KuroEngine;

	//��A�N�e�B�u
	if (m_timer.IsTimeUp())return;

	//�C���A���_�[���C��
	DrawFunc2D::DrawRotaGraph2D(m_accUnderLinePos, { 1.0f,1.0f }, 0.0f, m_accUnderLineTex, m_alpha);
	//�u�`�F�b�N�|�C���g��������܂����v
	DrawFunc2D::DrawRotaGraph2D(m_unlockStrPos, { 1.0f,1.0f }, 0.0f, m_unlockStrTex, m_alpha);
}
