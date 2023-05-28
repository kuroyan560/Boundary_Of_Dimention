#include "SystemSetting.h"
#include"DirectX12/D3D12App.h"
#include"../OperationConfig.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"FrameWork/WinApp.h"
#include"../SoundConfig.h"

std::string SystemSetting::TEX_DIR = "resource/user/tex/setting/";

void SystemSetting::SelectItem::LoadTex(std::string arg_path)
{
	KuroEngine::D3D12App::Instance()->GenerateTextureBuffer(m_tex.data(), arg_path, ITEM_STATUS_NUM, { 1,ITEM_STATUS_NUM });
}

SystemSetting::MainMenuGroup::MainMenuGroup()
{
	using namespace KuroEngine;
	//���o��
	m_headTex = D3D12App::Instance()->GenerateTextureBuffer(TEX_DIR + "head.png");
	//�u���ʁv
	m_items[ITEM_SOUND].LoadTex(TEX_DIR + "sound.png");
	//�u����v
	m_items[ITEM_OPE].LoadTex(TEX_DIR + "ope.png");
	//�u�߂�v
	m_items[ITEM_BACK].LoadTex(TEX_DIR + "back.png");
}

void SystemSetting::MainMenuGroup::Update(SystemSetting* arg_parent)
{
	bool upInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_UP);
	bool downInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_DOWN);
	bool doneInput = OperationConfig::Instance()->GetOperationInput(OperationConfig::DONE, OperationConfig::ON_TRIGGER);
	bool cancelInput = OperationConfig::Instance()->GetOperationInput(OperationConfig::CANCEL, OperationConfig::ON_TRIGGER);

	//�ω��O�̍���
	auto oldItem = m_nowItem;

	//���ڏ��
	if (0 < m_nowItem && upInput)m_nowItem = (ITEM)(m_nowItem - 1);
	//���ډ���
	if (m_nowItem < ITEM_NUM - 1 && downInput)m_nowItem = (ITEM)(m_nowItem + 1);

	//���ڕω�����
	if (oldItem != m_nowItem)SoundConfig::Instance()->Play(SoundConfig::SE_SELECT);

	//����
	if (doneInput)
	{
		//���ʐݒ�
		if (m_nowItem == ITEM_SOUND)arg_parent->m_mode = MODE_SOUND;
		//����ݒ�
		else if (m_nowItem == ITEM_OPE)arg_parent->m_mode = MODE_OPERATION;
		//�߂�
		else arg_parent->m_isActive = false;

		SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
	}

	//�L�����Z��
	if (cancelInput)
	{
		SoundConfig::Instance()->Play(SoundConfig::SE_CANCEL);
		arg_parent->m_isActive = false;
	}
}

void SystemSetting::MainMenuGroup::Draw()
{
	using namespace KuroEngine;

	//�E�B���h�E�Z���^�[
	const auto winCenter = WinApp::Instance()->GetExpandWinCenter();

	//���o��
	DrawFunc2D::DrawRotaGraph2D({ winCenter.x,171.0f }, { 1.0f,1.0f }, 0.0f, m_headTex);

	//���ڂ̕`��
	const std::array<Vec2<float>, ITEM_NUM>ITEM_CENTER_POS =
	{
		Vec2<float>(winCenter.x,310.0f),
		Vec2<float>(winCenter.x,418.0f),
		Vec2<float>(959.0f,570.0f),
	};

	for (int itemIdx = 0; itemIdx < ITEM_NUM; ++itemIdx)
	{
		int texIdx = (m_nowItem == itemIdx ? ITEM_STATUS::SELECTED : ITEM_STATUS::DEFAULT);
		DrawFunc2D::DrawRotaGraph2D(ITEM_CENTER_POS[itemIdx], { 1.0f,1.0f }, 0.0f, m_items[itemIdx].m_tex[texIdx]);
	}
}

SystemSetting::SoundMenuGroup::SoundMenuGroup()
{
	using namespace KuroEngine;

	const std::string dir = TEX_DIR + "soundMenu/";
	//���o��
	m_headTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "head.png");
	//�u�}�X�^�[���ʁv
	m_items[ITEM_MASTER].LoadTex(dir + "master.png");
	//�u�T�E���h�G�t�F�N�g�v
	m_items[ITEM_SE].LoadTex(dir + "se.png");
	//�u�w�i���y�v
	m_items[ITEM_BGM].LoadTex(dir + "bgm.png");
	//�u�߂�v
	m_items[ITEM_BACK].LoadTex(TEX_DIR + "back.png");
	//�n�ʃQ�[�W
	m_groundLineTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "ground_line.png");
	//�Q�[�W�̃p�^�[����
	D3D12App::Instance()->GenerateTextureBuffer(m_gagePatternTex.data(), dir + "pattern.png", GAGE_PATTERN_NUM, { GAGE_PATTERN_NUM ,1 });
}

void SystemSetting::SoundMenuGroup::Update(SystemSetting* arg_parent)
{
	bool upInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_UP);
	bool downInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_DOWN);
	bool leftInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_LEFT);
	bool rightInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_RIGHT);
	bool doneInput = OperationConfig::Instance()->GetOperationInput(OperationConfig::DONE, OperationConfig::ON_TRIGGER);
	bool cancelInput = OperationConfig::Instance()->GetOperationInput(OperationConfig::CANCEL, OperationConfig::ON_TRIGGER);

	//�ω��O�̍���
	auto oldItem = m_nowItem;

	//���ڏ��
	if (0 < m_nowItem && upInput)m_nowItem = (ITEM)(m_nowItem - 1);
	//���ډ���
	if (m_nowItem < ITEM_NUM - 1 && downInput)m_nowItem = (ITEM)(m_nowItem + 1);

	//���ڕω�����
	if (oldItem != m_nowItem)SoundConfig::Instance()->Play(SoundConfig::SE_SELECT);

	//����u�߂�v
	if (doneInput && m_nowItem == ITEM_BACK)
	{
		arg_parent->m_mode = MODE_MAIN_MENU;
		SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
	}
	//�L�����Z�����u�߂�v
	if (cancelInput)
	{
		arg_parent->m_mode = MODE_MAIN_MENU;
		SoundConfig::Instance()->Play(SoundConfig::SE_CANCEL);
	}
}

void SystemSetting::SoundMenuGroup::Draw()
{
	using namespace KuroEngine;

	//���o��
	DrawFunc2D::DrawGraph({ 0.0f,32.0f }, m_headTex);

	//�n�ʃQ�[�W��X
	const float GROUND_GAGE_X = 588.0f;
	const float GROUND_GAGE_OFFSEY_Y = 16.0f;

	//���ڂ̕`��
	const std::array<Vec2<float>, ITEM_NUM>ITEM_CENTER_POS =
	{
		Vec2<float>(412.0f,122.0f),
		Vec2<float>(344.0f,311.0f),
		Vec2<float>(405.0f,513.0f),
		Vec2<float>(439.0f,651.0f),
	};
	for (int itemIdx = 0; itemIdx < ITEM_NUM; ++itemIdx)
	{
		int texIdx = (m_nowItem == itemIdx ? ITEM_STATUS::SELECTED : ITEM_STATUS::DEFAULT);
		DrawFunc2D::DrawRotaGraph2D(ITEM_CENTER_POS[itemIdx], { 1.0f,1.0f }, 0.0f, m_items[itemIdx].m_tex[texIdx]);

		//�n�ʃQ�[�W
		if (itemIdx != ITEM_BACK)
		{
			DrawFunc2D::DrawGraph({ GROUND_GAGE_X,ITEM_CENTER_POS[itemIdx].y + GROUND_GAGE_OFFSEY_Y }, m_groundLineTex);
		}
	}
}

SystemSetting::OpeMenuGroup::OpeMenuGroup()
{
	using namespace KuroEngine;

	const std::string dir = TEX_DIR + "operationMenu/";
	//���o��
	m_headTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "head.png");
	
	//�u�J�����~���[�v
	m_items[ITEM_MIRROR].LoadTex(dir + "cam_mirror.png");
	//�u�J�������x�v
	m_items[ITEM_SENSITIVITY].LoadTex(dir + "cam_sensitivity.png");
	//�u�߂�v
	m_items[ITEM_BACK].LoadTex(TEX_DIR + "back.png");

	//���
	m_arrowTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "arrow.png");
	//�J�����̃~���[�̃`�F�b�N�{�b�N�X
	m_camMirrorCheckBoxTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "check_box.png");
	//�`�F�b�N�}�[�N
	m_checkMarkTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "check_box.png");
	//�n�ʃQ�[�W
	m_groundLineTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "ground.png");
	//�n�ʃQ�[�W�̃C���f�b�N�X�̗t
	m_leafTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "leaf.png");
}

void SystemSetting::OpeMenuGroup::Update(SystemSetting* arg_parent)
{
	bool upInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_UP);
	bool downInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_DOWN);
	bool leftInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_LEFT);
	bool rightInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_RIGHT);
	bool doneInput = OperationConfig::Instance()->GetOperationInput(OperationConfig::DONE, OperationConfig::ON_TRIGGER);
	bool cancelInput = OperationConfig::Instance()->GetOperationInput(OperationConfig::CANCEL, OperationConfig::ON_TRIGGER);

	//�ω��O�̍���
	auto oldItem = m_nowItem;

	//���ڏ��
	if (0 < m_nowItem && upInput)m_nowItem = (ITEM)(m_nowItem - 1);
	//���ډ���
	if (m_nowItem < ITEM_NUM - 1 && downInput)m_nowItem = (ITEM)(m_nowItem + 1);

	//���ڕω�����
	if (oldItem != m_nowItem)SoundConfig::Instance()->Play(SoundConfig::SE_SELECT);

	//����u�߂�v
	if (doneInput && m_nowItem == ITEM_BACK)
	{
		arg_parent->m_mode = MODE_MAIN_MENU;
		SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
	}
	//�L�����Z��=�u�߂�v
	if (cancelInput)
	{
		arg_parent->m_mode = MODE_MAIN_MENU;
		SoundConfig::Instance()->Play(SoundConfig::SE_CANCEL);
	}
}

void SystemSetting::OpeMenuGroup::Draw()
{
	using namespace KuroEngine;

	//���o��
	DrawFunc2D::DrawGraph({ 0.0f,32.0f }, m_headTex);

	//���ڂ̕`��
	const std::array<Vec2<float>, ITEM_NUM>ITEM_CENTER_POS =
	{
		Vec2<float>(563.0f,152.0f),
		Vec2<float>(355.0f,373.0f),
		Vec2<float>(346.0f,642.0f),
	};
	for (int itemIdx = 0; itemIdx < ITEM_NUM; ++itemIdx)
	{
		int texIdx = (m_nowItem == itemIdx ? ITEM_STATUS::SELECTED : ITEM_STATUS::DEFAULT);
		DrawFunc2D::DrawRotaGraph2D(ITEM_CENTER_POS[itemIdx], { 1.0f,1.0f }, 0.0f, m_items[itemIdx].m_tex[texIdx]);
	}

	//�`�F�b�N�{�b�N�X
	DrawFunc2D::DrawRotaGraph2D({ 921.0f,ITEM_CENTER_POS[ITEM_MIRROR].y }, { 1.0f,1.0f }, 0.0f, m_camMirrorCheckBoxTex);

	//�n�ʃQ�[�W
	DrawFunc2D::DrawGraph({ 504.0f,390.0f }, m_groundLineTex);
}

void SystemSetting::Update()
{
	if (!m_isActive)return;

	auto oldMode = m_mode;

	//�X�V
	if (m_mode == MODE_MAIN_MENU)
	{
		m_mainMenu.Update(this);
	}
	else if (m_mode == MODE_SOUND)
	{
		m_soundMenu.Update(this);
	}
	else if (m_mode == MODE_OPERATION)
	{
		m_opeMenu.Update(this);
	}

	//�ݒ胁�j���[���[�h�ω�����
	if (oldMode != m_mode)
	{
		switch (m_mode)
		{
			case MODE_MAIN_MENU:
				//m_mainMenu.Init();
				break;
			case MODE_SOUND:
				m_soundMenu.Init();
				break;
			case MODE_OPERATION:
				m_opeMenu.Init();
				break;
		}
	}

	OperationConfig::Instance()->SetAllInputActive(true);
}

void SystemSetting::Draw()
{
	using namespace KuroEngine;

	if (!m_isActive)return;

	//�w�i�F�`��
	DrawFunc2D::DrawBox2D({ 0,0 }, WinApp::Instance()->GetExpandWinSize(),
		Color(221, 236, 212, 255), true, AlphaBlendMode_None);

	//�`��
	if (m_mode == MODE_MAIN_MENU)
	{
		m_mainMenu.Draw();
	}
	else if (m_mode == MODE_SOUND)
	{
		m_soundMenu.Draw();
	}
	else if (m_mode == MODE_OPERATION)
	{
		m_opeMenu.Draw();
	}
}

void SystemSetting::Activate()
{
	m_isActive = true;
	m_mode = MODE_MAIN_MENU;
	m_mainMenu.Init();
	OperationConfig::Instance()->SetAllInputActive(false);
}