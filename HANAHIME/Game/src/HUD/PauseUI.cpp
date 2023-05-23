#include "PauseUI.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"../TimeScaleMgr.h"
#include"DirectX12/D3D12App.h"
#include"FrameWork/WinApp.h"
#include"Common/PerlinNoise.h"
#include"../OperationConfig.h"
#include"../SoundConfig.h"
#include"../GameScene.h"

void PauseUI::OnActive()
{
	//�|�[�Y���O�̃^�C���X�P�[���i�[
	m_latestTimeScale = TimeScaleMgr::s_inGame.GetTimeScale();
	//�Q�[�������Ԃ��~�߂�
	TimeScaleMgr::s_inGame.Set(0.0f);

	//sin�J�[�u���[�g������
	m_sinCurveRateT = 0.0f;

	//�p�[�����m�C�Y���[�g������
	m_perlinNoiseRateT = 0.0f;
	//�p�[�����m�C�Y�V�[�h������
	m_perlinNoiseSeed.x = KuroEngine::GetRand(100);
	m_perlinNoiseSeed.y = KuroEngine::GetRand(100);

	//���ڃ��Z�b�g
	m_item = (PAUSE_ITEM)0;

	//SE
	SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
}

void PauseUI::OnNonActive()
{
	//�Q�[�������Ԃ��ĊJ
	TimeScaleMgr::s_inGame.Set(m_latestTimeScale);
}

PauseUI::PauseUI()
{
	using namespace KuroEngine;

	//�|�[�Y��ʂ̉摜���\�[�X�̃f�B���N�g��
	std::string pauseTexDir = "resource/user/tex/pause/";
	//���ڂ��Ƃ̉摜�t�@�C����
	std::array<std::string, PAUSE_ITEM_NUM>itemTexFileName =
	{
		"resume",
		"retry",
		"fast_travel",
		"setting",
		"return_to_title"
	};
	//���ڂ��Ƃ̉摜�ǂݍ���
	for (int itemIdx = 0; itemIdx < PAUSE_ITEM_NUM; ++itemIdx)
	{
		D3D12App::Instance()->GenerateTextureBuffer(m_itemTexArray[itemIdx].data(),
			pauseTexDir + itemTexFileName[itemIdx] + ".png", ITEM_STATUS_NUM, { 1,ITEM_STATUS_NUM });
	}
	//�I�𒆂̍��ڂɂ̂ݏo��e�摜�ǂݍ���
	m_selectItemShadowTex = D3D12App::Instance()->GenerateTextureBuffer(pauseTexDir + "shadow.png");

	//�Ԃ̉摜�ǂݍ���
	m_flowerTex = D3D12App::Instance()->GenerateTextureBuffer(pauseTexDir + "flower.png");
	//���W�Ԃ̐��e�N�X�`���ǂݍ���
	D3D12App::Instance()->GenerateTextureBuffer(m_flowerNumTexArray.data(),
		pauseTexDir + "flower_num.png", FLOWER_NUM_TEX_SIZE, { FLOWER_NUM_TEX_SIZE,1 });
	//���W�Ԃ́u x �v�e�N�X�`���ǂݍ���
	m_flowerMulTex = D3D12App::Instance()->GenerateTextureBuffer(pauseTexDir + "mul.png");

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
}

void PauseUI::Init()
{
	m_isActive = false;
}

void PauseUI::Update(GameScene* arg_gameScene, float arg_timeScale)
{
	//��A�N�e�B�u
	if (!m_isActive)return;

	using namespace KuroEngine;

	const float SIN_CURVE_INTERVAL = 60.0f;
	const float PERLIN_NOISE_INTERVAL = 90.0f;

	//sin�J�[�u�X�V
	m_sinCurveRateT += 1.0f / SIN_CURVE_INTERVAL * arg_timeScale;

	//�p�[�����m�C�Y�X�V
	m_perlinNoiseRateT += 1.0f / PERLIN_NOISE_INTERVAL * arg_timeScale;

	//�I�𒆂̉e�̃p�[�����m�C�Y��]
	const Angle SELECT_ITEM_SHADOW_SPIN_MAX = Angle(5);
	m_selectItemShadowSpin = sin(m_sinCurveRateT * Angle::ROUND()) * SELECT_ITEM_SHADOW_SPIN_MAX;

	//�I�𒆂̉e�̃p�[�����m�C�Y�I�t�Z�b�g
	const Vec2<float>SELECT_ITEM_SHADOW_OFFSET_MAX = { 32.0f,64.0f };
	m_selectItemShadowOffset.x = PerlinNoise::GetRand(m_perlinNoiseRateT, 0.0f, m_perlinNoiseSeed.x) * SELECT_ITEM_SHADOW_OFFSET_MAX.x;
	m_selectItemShadowOffset.y = PerlinNoise::GetRand(m_perlinNoiseRateT, 0.0f, m_perlinNoiseSeed.y) * SELECT_ITEM_SHADOW_OFFSET_MAX.y;

	//���ڂ̈ړ�
	auto oldItem = m_item;
	if (m_item < PAUSE_ITEM_NUM - 1 && OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_DOWN))m_item = PAUSE_ITEM(m_item + 1);		//����
	else if (0 < m_item && OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_UP))m_item = PAUSE_ITEM(m_item - 1);		//���

	//���ڂɕω���������
	if (oldItem != m_item)
	{
		//SE�Đ�
		SoundConfig::Instance()->Play(SoundConfig::SE_SELECT);
	}

	//����{�^��
	if (OperationConfig::Instance()->GetOperationInput(OperationConfig::DONE, OperationConfig::ON_TRIGGER))
	{
		switch (m_item)
		{
			//�Q�[�����ĊJ
			case RESUME:
				this->SetInverseActive();
				break;
			//���g���C
			case RETRY:
				arg_gameScene->Retry();
				//�Q�[�������Ԃ��ĊJ
				TimeScaleMgr::s_inGame.Set(m_latestTimeScale);
				m_isActive = false;
				//���C���Q�[��������͎͂󂯕t���Ȃ��܂�
				break;
			case FAST_TRAVEL:
				break;
			case SETTING:
				break;
			case RETURN_TO_TITLE:
				break;
			default:
				break;
		}
		//SE�Đ�
		SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
	}

	//�L�����Z���{�^��
	bool cancelInput = OperationConfig::Instance()->GetOperationInput(OperationConfig::CANCEL, OperationConfig::ON_TRIGGER);
	if (cancelInput)
	{
		//�|�[�Y�I��
		this->SetInverseActive();

		//SE�Đ�
		SoundConfig::Instance()->Play(SoundConfig::SE_CANCEL);
	}
}

void PauseUI::Draw(int arg_totalGetFlowerNum)
{
	//��A�N�e�B�u
	if (!m_isActive)return;

	using namespace KuroEngine;

	//�E�B���h�E�̒��S���WX
	static const float WIN_CENTER_X = WinApp::Instance()->GetExpandWinCenter().x;
	//�E�B���h�E�T�C�Y
	static const auto WIN_SIZE = WinApp::Instance()->GetExpandWinSize();

	//���������l�p�w�i�`��
	static const float SQUARE_WIDTH_HALF = 284.0f;
	static const float SQUARE_ALPHA = 0.2f;
	DrawFunc2D::DrawBox2D(
		{ WIN_CENTER_X - SQUARE_WIDTH_HALF,0.0f }, { WIN_CENTER_X + SQUARE_WIDTH_HALF,WIN_SIZE.y },
		Color(0.0f, 0.0f, 0.0f, SQUARE_ALPHA), true);

	//�X�e�[�W���̒��S���W
	static const Vec2<float>STAGE_NAME_CENTER_POS = { WIN_CENTER_X,91.0f };
	//�X�e�[�W���`��
	auto stageNameTex = m_stageNameDefaultTex;
	if (0 <= m_stageNameIdx && m_stageNameIdx < static_cast<int>(m_stageNameTex.size()))stageNameTex = m_stageNameTex[m_stageNameIdx];
	DrawFunc2D::DrawRotaGraph2D(STAGE_NAME_CENTER_POS, { 1.0f,1.0f }, 0.0f, stageNameTex);

	//�X�e�[�W���̑��������`��
	static const Vec2<float>UNDER_LINE_CENTER_POS = { WIN_CENTER_X,132.0f };
	DrawFunc2D::DrawRotaGraph2D(UNDER_LINE_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_underLineTex);

	//��ԏ�̍��ڂ̒��S���W
	const Vec2<float>TOP_ITEM_CENTER_POS = { WIN_CENTER_X,215.0f };
	//���ڂ̍s��
	const float ITEM_SPACE_Y = 79.0f;
	//�I������Ă��Ȃ����ڂ̃A���t�@
	const float NO_SELECT_ITEM_ALPHA = 0.8f;
	//�I�𒆍��ڂ̉�]�e�̃A���t�@
	const float SELECT_ITEM_SPIN_SHADOW_ALPHA = 0.4f;

	//���ڂ̕`��
	for (int itemIdx = 0; itemIdx < PAUSE_ITEM_NUM; ++itemIdx)
	{
		//���W�v�Z
		const auto pos = TOP_ITEM_CENTER_POS + Vec2<float>(0.0f, ITEM_SPACE_Y * itemIdx);
		//���ڂ��I�𒆂�
		bool isSelected = (PAUSE_ITEM)itemIdx == m_item;

		//�I�𒆂Ȃ�e�`��
		if (isSelected)
		{
			//��]����1
			DrawFunc2D::DrawRotaGraph2D(pos + m_selectItemShadowOffset, { 1.0f,1.0f }, m_selectItemShadowSpin, m_selectItemShadowTex, SELECT_ITEM_SPIN_SHADOW_ALPHA);
			//��]����2
			DrawFunc2D::DrawRotaGraph2D(pos - m_selectItemShadowOffset, { 1.0f,1.0f }, -m_selectItemShadowSpin, m_selectItemShadowTex, SELECT_ITEM_SPIN_SHADOW_ALPHA);
			//��]�Ȃ�
			DrawFunc2D::DrawRotaGraph2D(pos, { 1.0f,1.0f }, 0.0f, m_selectItemShadowTex);
		}


		//�X�e�[�^�X
		ITEM_STATUS itemStatus = isSelected ? SELECT : DEFAULT;
		//�e�N�X�`������
		auto& tex = m_itemTexArray[itemIdx][itemStatus];
		//�A���t�@����
		float alpha = isSelected ? 1.0f : NO_SELECT_ITEM_ALPHA;
		DrawFunc2D::DrawRotaGraph2D(pos, { 1.0f,1.0f }, 0.0f, tex, alpha);
	}

	//�ԕ`��
	const Vec2<float>FLOWER_CENTER_POS = { 1128.0f,86.0f };
	DrawFunc2D::DrawRotaGraph2D(FLOWER_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_flowerTex);
	//�u x �v�`��
	const Vec2<float>FLOWER_MUL_CENTER_POS = { 1159.0f,127.0f };
	DrawFunc2D::DrawRotaGraph2D(FLOWER_MUL_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_flowerMulTex);
	//�Ԃ̐��`��
	const Vec2<float>FLOWER_NUM_LEFT_UP_POS = { 1180.0f,102.0f };
	DrawFunc2D::DrawNumber2D(arg_totalGetFlowerNum, FLOWER_NUM_LEFT_UP_POS, m_flowerNumTexArray.data());

}

void PauseUI::SetInverseActive()
{
	m_isActive = !m_isActive;

	if (m_isActive == true)
	{
		OnActive();

	}
	else
	{
		OnNonActive();
	}

	//�C���Q�[���̑�����͎�t�؂�ւ�
	OperationConfig::Instance()->SetInGameOperationActive(!m_isActive);
}
