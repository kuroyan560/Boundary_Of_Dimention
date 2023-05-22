#include "StageInfoUI.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"

void StageInfoUI::SetUIStatus(STATUS arg_status)
{
	using namespace KuroEngine;

	static const std::array<float, STATUS_NUM>INTERVALS =
	{
		35.0f,	//�o��
		250.0f,	//�ʏ�`��
		35.0f,	//�ޏ�
	};

	//���o���ԃZ�b�g
	m_timer.Reset(INTERVALS[arg_status]);
	//�X�e�[�^�X�X�V
	m_status = arg_status;
}

void StageInfoUI::Appear()
{
	if (m_status != DISAPPEAR)return;
	SetUIStatus(APPEAR);
}

void StageInfoUI::Disappear()
{
	if (m_status != DRAW)return;
	SetUIStatus(DISAPPEAR);
}

StageInfoUI::StageInfoUI()
{
	using namespace KuroEngine;

	//�X�e�[�W�֘A�̃e�N�X�`���̃f�B���N�g��
	std::string stageTexDir = "resource/user/tex/stage/";
	//�X�e�[�W���e�N�X�`��
	int stageIdx = 0;
	while (1)
	{
		std::string path = stageTexDir + std::to_string(stageIdx++) + ".png";
		if (!ExistFile(path))break;

		m_stageNameTex.emplace_back(D3D12App::Instance()->GenerateTextureBuffer(path));
	}
	m_stageNameDefaultTex = D3D12App::Instance()->GenerateTextureBuffer(stageTexDir + "default.png");
	//�X�e�[�W���̑��������摜
	m_underLineTex = D3D12App::Instance()->GenerateTextureBuffer(stageTexDir + "under_line.png");

	//���W�Ԃ̃e�N�X�`���̃f�B���N�g��
	std::string flowerTexDir = "resource/user/tex/in_game/flower/";
	//�Ԃ̉摜�i���j
	m_miniFlowerTex = D3D12App::Instance()->GenerateTextureBuffer(flowerTexDir + "flower_mini.png");
	//���W�����Ԃ̐����i�Ō�̃C���f�b�N�X�̉摜�� �u / �v�j
	D3D12App::Instance()->GenerateTextureBuffer(m_flowerNumTex.data(), flowerTexDir + "flower_num.png", FLOWER_NUM_TEX_SIZE, { FLOWER_NUM_TEX_SIZE ,1 });
}

void StageInfoUI::Init(int arg_stageNum, int arg_getFlowerNum)
{
	m_stageNameIdx = arg_stageNum;
	SetUIStatus(APPEAR);
	m_oldGetFlowerNum = arg_getFlowerNum;
	m_addFlowerNum = 0;
}

void StageInfoUI::Update(float arg_timeScale, int arg_getFlowerNum)
{
	using namespace KuroEngine;

	const float OFFSET_X_MAX = 300.0f;
	const float ADD_FLOWER_OFFSET_Y_MAX = -80.0f;
	const float ADD_FLOWER_APEEAR_TIME = 20.0f;

	m_timer.UpdateTimer(arg_timeScale);

	if (m_status == APPEAR)
	{
		m_offsetX = Math::Ease(Out, Back, m_timer.GetTimeRate(), OFFSET_X_MAX, 0.0f);
		m_alpha = Math::Lerp(0.0f, 1.0f, m_timer.GetTimeRate());
		if (m_timer.IsTimeUp())
		{
			SetUIStatus(DRAW);
		}
	}
	else if (m_status == DRAW)
	{
		m_addFlowerNumTimer.UpdateTimer(arg_timeScale);
		m_addFlowerAlpha = Math::Ease(Out, Exp, m_addFlowerNumTimer.GetTimeRate(), 0.0f, 1.0f);
		m_addFlowerOffsetY = Math::Ease(Out, Back, m_addFlowerNumTimer.GetTimeRate(), ADD_FLOWER_OFFSET_Y_MAX, 0.0f);

		if (m_addFlowerNum && m_timer.IsTimeUp())
		{
			m_addFlowerNum = 0;
			SetUIStatus(DISAPPEAR);
		}
	}
	else if (m_status == DISAPPEAR)
	{
		m_offsetX = Math::Ease(In, Back, m_timer.GetTimeRate(), 0.0f, OFFSET_X_MAX);
		m_alpha = Math::Lerp(1.0f, 0.0f, m_timer.GetTimeRate());
	}

	if (m_oldGetFlowerNum < arg_getFlowerNum)
	{
		m_addFlowerNumTimer.Reset(ADD_FLOWER_APEEAR_TIME);
		Appear();
		m_addFlowerNum = arg_getFlowerNum - m_oldGetFlowerNum;
		m_addFlowerAlpha = 0.0f;
		m_addFlowerOffsetY = ADD_FLOWER_OFFSET_Y_MAX;
	}
	m_oldGetFlowerNum = arg_getFlowerNum;
}

void StageInfoUI::Draw(int arg_existFlowerNum, int arg_getFlowerNum)
{
	using namespace KuroEngine;

	//�X�e�[�W���̉E���̍��W
	static const Vec2<float>STAGE_NAME_RIGHT_BOTTOM_POS = { 1130.0f,150.0f };

	//�I�t�Z�b�g
	const Vec2<float>offsetX = { m_offsetX,0.0f };

	//�X�e�[�W���`��
	auto stageNameTex = m_stageNameDefaultTex;
	if (0 <= m_stageNameIdx && m_stageNameIdx < static_cast<int>(m_stageNameTex.size()))stageNameTex = m_stageNameTex[m_stageNameIdx];
	const Vec2<float>stageNamePos = STAGE_NAME_RIGHT_BOTTOM_POS - stageNameTex->GetGraphSize().Float();
	DrawFunc2D::DrawGraph(stageNamePos + offsetX, stageNameTex, m_alpha);

	//�X�e�[�W���̑��������`��
	static const Vec2<float>UNDER_LINE_CENTER_POS = { 958.0f,161.0f };
	DrawFunc2D::DrawRotaGraph2D(UNDER_LINE_CENTER_POS + offsetX, { 1.0f,1.0f }, 0.0f, m_underLineTex, m_alpha);

	//�ԃA�C�R���̕`��
	static const Vec2<float>MINI_FLOWER_CENTER_POS = { 1012.0f,202.0f };
	DrawFunc2D::DrawRotaGraph2D(MINI_FLOWER_CENTER_POS + offsetX, { 1.0f,1.0f }, 0.0f, m_miniFlowerTex, m_alpha);

	//���̐����̕`��
	static const Vec2<float>LEFT_NUM_CENTER_POS = { 1051.0f,205.0f };
	DrawFunc2D::DrawRotaGraph2D(LEFT_NUM_CENTER_POS + offsetX, { 1.0f,1.0f }, 0.0f, m_flowerNumTex[arg_getFlowerNum], m_alpha);

	//�u / �v�̕`��
	static const Vec2<float>SLASH_CENTER_POS = { 1076.0f,208.0f };
	DrawFunc2D::DrawRotaGraph2D(SLASH_CENTER_POS + offsetX, { 1.0f,1.0f }, 0.0f, m_flowerNumTex[FLOWER_NUM_SLASH_IDX], m_alpha);

	//�E�̐����̕`��
	static const Vec2<float>RIGHT_NUM_CENTER_POS = { 1098.0f,208.0f };
	DrawFunc2D::DrawRotaGraph2D(RIGHT_NUM_CENTER_POS + offsetX, { 1.0f,1.0f }, 0.0f, m_flowerNumTex[arg_existFlowerNum], m_alpha);

	//���莞
	if (m_addFlowerNum)
	{
		const Vec2<float>addFlowerOffsetY = { 0.0f,m_addFlowerOffsetY };

		//�u + �v�̕`��
		const Vec2<float>PLUS_CENTER_POS = { 1058.0f,256.0f };
		DrawFunc2D::DrawRotaGraph2D(PLUS_CENTER_POS + addFlowerOffsetY, { 1.0f,1.0f }, 0.0f, m_flowerNumTex[FLOWER_NUM_PLUS_IDX], m_addFlowerAlpha);

		//�������Ԃ̐��`��
		const Vec2<float>ADD_FLOWER_NUM_CENTER_POS = { 1080.0f,252.0f };
		DrawFunc2D::DrawRotaGraph2D(ADD_FLOWER_NUM_CENTER_POS + addFlowerOffsetY, { 1.0f,1.0f }, 0.0f, m_flowerNumTex[m_addFlowerNum], m_addFlowerAlpha);
	}
}
