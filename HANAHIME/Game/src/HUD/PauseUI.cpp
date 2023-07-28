#include "PauseUI.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"../TimeScaleMgr.h"
#include"DirectX12/D3D12App.h"
#include"FrameWork/WinApp.h"
#include"Common/PerlinNoise.h"
#include"../OperationConfig.h"
#include"../SoundConfig.h"
#include"../GameScene.h"
#include"../System/SaveDataManager.h"

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

	//���j���[�X�e�[�^�X���Z�b�g
	m_menuStatus = DEFAULT_MENU;
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
		"resume.png",
		"retry.png",
		//"fast_travel.png",
		"stage_select.png",
		"setting.png",
		"return_to_title.png"
	};

	//���j���[
	{
		std::string menuTexDir = pauseTexDir + "menu/";

		//���ڂ��Ƃ̉摜�ǂݍ���
		for (int itemIdx = 0; itemIdx < PAUSE_ITEM_NUM; ++itemIdx)
		{
			D3D12App::Instance()->GenerateTextureBuffer(m_defaultMenu.m_itemTexArray[itemIdx].data(),
				menuTexDir + itemTexFileName[itemIdx], ITEM_STATUS_NUM, { 1,ITEM_STATUS_NUM });
		}
		//�I�𒆂̍��ڂɂ̂ݏo��e�摜�ǂݍ���
		m_defaultMenu.m_selectItemShadowTex = D3D12App::Instance()->GenerateTextureBuffer(menuTexDir + "shadow.png");

		//�X�e�[�W�֘A�̃e�N�X�`���̃f�B���N�g��
		std::string stageTexDir = "resource/user/tex/stage/";
		//�X�e�[�W���e�N�X�`��
		int stageIdx = 0;
		while (1)
		{
			std::string path = stageTexDir + std::to_string(stageIdx++) + ".png";
			if (!ExistFile(path))break;

			m_defaultMenu.m_stageNameTex.emplace_back(D3D12App::Instance()->GenerateTextureBuffer(path));
		}
		m_defaultMenu.m_stageNameDefaultTex = D3D12App::Instance()->GenerateTextureBuffer(stageTexDir + "default.png");
		//�X�e�[�W���̑��������摜
		m_defaultMenu.m_underLineTex = D3D12App::Instance()->GenerateTextureBuffer(stageTexDir + "under_line.png");

		//�Ԃ̉摜�ǂݍ���
		m_defaultMenu.m_flowerTex = D3D12App::Instance()->GenerateTextureBuffer(pauseTexDir + "flower.png");
		//���W�Ԃ̐��e�N�X�`���ǂݍ���
		D3D12App::Instance()->GenerateTextureBuffer(m_defaultMenu.m_flowerNumTexArray.data(),
			pauseTexDir + "flower_num.png", DefaultMenu::FLOWER_NUM_TEX_SIZE, { DefaultMenu::FLOWER_NUM_TEX_SIZE,1 });
		//���W�Ԃ́u x �v�e�N�X�`���ǂݍ���
		m_defaultMenu.m_flowerMulTex = D3D12App::Instance()->GenerateTextureBuffer(pauseTexDir + "mul.png");

		//�u�|�[�Y�v
		m_defaultMenu.m_pauseStrTex = D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/pause/pause_str.png");
	}

	//�ŏI�m�F���j���[
	{
		std::string menuTexDir = pauseTexDir + "confirm/";

		//�A�C�R���摜�ǂݍ���
		m_confirmMenu.m_iconTex = D3D12App::Instance()->GenerateTextureBuffer(menuTexDir + "icon.png");
		//�u�������v�摜�ǂݍ���
		D3D12App::Instance()->GenerateTextureBuffer(m_confirmMenu.m_answerTexArray[ConfirmMenu::NO].data(), menuTexDir + "no.png", ITEM_STATUS_NUM, { 1,ITEM_STATUS_NUM });
		//�u�͂��v�摜�ǂݍ���
		D3D12App::Instance()->GenerateTextureBuffer(m_confirmMenu.m_answerTexArray[ConfirmMenu::YES].data(), menuTexDir + "yes.png", ITEM_STATUS_NUM, { 1,ITEM_STATUS_NUM });
		//�u�������v�u�͂��v�̑I�𑤂ɏo��e�摜�ǂݍ���
		m_confirmMenu.m_answerShadowTex = D3D12App::Instance()->GenerateTextureBuffer(menuTexDir + "shadow.png");
		//�u��낵���ł����H�v�摜
		m_confirmMenu.m_mindTex = D3D12App::Instance()->GenerateTextureBuffer(menuTexDir + "mind.png");

		//�I�񂾍��ڂɉ���������摜
		for (int itemIdx = 0; itemIdx < PAUSE_ITEM_NUM; ++itemIdx)
		{
			auto path = menuTexDir + "question/" + itemTexFileName[itemIdx];
			if (!ExistFile(path))continue;
			m_confirmMenu.m_questionTexArray[itemIdx] = D3D12App::Instance()->GenerateTextureBuffer(path);
		}
	}
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

	//����{�^��
	bool doneInput = OperationConfig::Instance()->GetOperationInput(OperationConfig::DONE, OperationConfig::ON_TRIGGER);
	//�L�����Z���{�^��
	bool cancelInput = OperationConfig::Instance()->GetOperationInput(OperationConfig::CANCEL, OperationConfig::ON_TRIGGER);
	//�����
	bool upInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_UP);
	//������
	bool downInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_DOWN);
	//������
	bool leftInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_LEFT);
	//�E����
	bool rightInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_RIGHT);

	//�ʏ탁�j���[
	if (m_menuStatus == DEFAULT_MENU)
	{
		//���ڂ̈ړ�
		auto oldItem = m_item;
		if (m_item < PAUSE_ITEM_NUM - 1 && downInput)m_item = PAUSE_ITEM(m_item + 1);		//����
		else if (0 < m_item && upInput)m_item = PAUSE_ITEM(m_item - 1);		//���

		//���ڂɕω���������
		if (oldItem != m_item)
		{
			//SE�Đ�
			SoundConfig::Instance()->Play(SoundConfig::SE_SELECT);

			//�Z�[�u�f�[�^���Ȃ��̂Ńt�@�X�g�g���x���o���Ȃ�
			//if (m_item == FAST_TRAVEL && !SaveDataManager::Instance()->LoadStageSaveData(nullptr, nullptr))
			//{
			//	if (m_item < oldItem)m_item = PAUSE_ITEM(FAST_TRAVEL - 1);
			//	else if (oldItem < m_item)m_item = PAUSE_ITEM(FAST_TRAVEL + 1);
			//}
		}

		//����{�^��
		if (doneInput)
		{
			switch (m_item)
			{
				//�Q�[�����ĊJ
				case RESUME:
					this->SetInverseActive();
					break;
				//���g���C
				case RETRY:
					//�Q�[���_���W�����p
					arg_gameScene->Retry();
					//�Q�[�������Ԃ��ĊJ
					TimeScaleMgr::s_inGame.Set(m_latestTimeScale);
					m_isActive = false;
					//���C���Q�[��������͎͂󂯕t���Ȃ��܂�

					/*
					m_menuStatus = CONFIRM_MENU;
					m_confirmMenu.m_confirmItem = ConfirmMenu::CONFIRM_ITEM::NONE;
					*/
					break;
					/*
				case FAST_TRAVEL:
					arg_gameScene->ActivateFastTravel();
					break;
					*/
				case STAGE_SELECT:
					arg_gameScene->GoStageSelect();
					break;
				case SETTING:
					arg_gameScene->ActivateSystemSetting();
					break;
				case RETURN_TO_TITLE:
					arg_gameScene->GoBackTitle();
					//�Q�[�������Ԃ��ĊJ
					TimeScaleMgr::s_inGame.Set(m_latestTimeScale);
					m_isActive = false;
					//���C���Q�[��������͎͂󂯕t���Ȃ��܂�

					/*
					m_menuStatus = CONFIRM_MENU;
					m_confirmMenu.m_confirmItem = ConfirmMenu::CONFIRM_ITEM::NONE;
					*/
					break;
				default:
					break;
			}
			//SE�Đ�
			SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
		}

		if (cancelInput)
		{
			//�|�[�Y�I��
			this->SetInverseActive();
			//SE�Đ�
			SoundConfig::Instance()->Play(SoundConfig::SE_CANCEL);
		}
	}
	else if (m_menuStatus == CONFIRM_MENU)
	{
		//���ڈړ�
		auto oldItem = m_confirmMenu.m_confirmItem;
		if (m_confirmMenu.m_confirmItem == ConfirmMenu::CONFIRM_ITEM::NONE)
		{
			if (rightInput || leftInput || upInput || downInput)
			{
				m_confirmMenu.m_confirmItem = ConfirmMenu::CONFIRM_ITEM::NO;
			}
		}
		else
		{
			if (rightInput && m_confirmMenu.m_confirmItem == ConfirmMenu::CONFIRM_ITEM::NO)m_confirmMenu.m_confirmItem = ConfirmMenu::CONFIRM_ITEM::YES;
			if (leftInput && m_confirmMenu.m_confirmItem == ConfirmMenu::CONFIRM_ITEM::YES)m_confirmMenu.m_confirmItem = ConfirmMenu::CONFIRM_ITEM::NO;

			//����
			if (doneInput)
			{
				if (m_confirmMenu.m_confirmItem == ConfirmMenu::CONFIRM_ITEM::YES)
				{
					if (m_item == RETRY)
					{
						arg_gameScene->Retry();
						//�Q�[�������Ԃ��ĊJ
						TimeScaleMgr::s_inGame.Set(m_latestTimeScale);
						m_isActive = false;
						//���C���Q�[��������͎͂󂯕t���Ȃ��܂�
					}
					else if (m_item == RETURN_TO_TITLE)
					{
						arg_gameScene->GoBackTitle();
						//�Q�[�������Ԃ��ĊJ
						TimeScaleMgr::s_inGame.Set(m_latestTimeScale);
						m_isActive = false;
						//���C���Q�[��������͎͂󂯕t���Ȃ��܂�
					}
				}
				else m_menuStatus = DEFAULT_MENU;

				SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
			}
			//�L�����Z��
			else if (cancelInput)
			{
				m_menuStatus = DEFAULT_MENU;
				SoundConfig::Instance()->Play(SoundConfig::SE_CANCEL);
			}
		}

		//���ڂɕω���������
		if (oldItem != m_confirmMenu.m_confirmItem)
		{
			SoundConfig::Instance()->Play(SoundConfig::SE_SELECT);
		}
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

	//�I������Ă��Ȃ����ڂ̃A���t�@
	const float NO_SELECT_ITEM_ALPHA = 0.8f;
	//�I�𒆍��ڂ̉�]�e�̃A���t�@
	const float SELECT_ITEM_SPIN_SHADOW_ALPHA = 0.4f;

	//���������l�p�w�i�`��
	static const float SQUARE_WIDTH_HALF = 284.0f;
	static const float SQUARE_ALPHA = 0.5f;
	DrawFunc2D::DrawBox2D(
		{ WIN_CENTER_X - SQUARE_WIDTH_HALF,0.0f }, { WIN_CENTER_X + SQUARE_WIDTH_HALF,WIN_SIZE.y },
		Color(0.0f, 0.0f, 0.0f, SQUARE_ALPHA), true);

	//�ʏ탁�j���[
	if (m_menuStatus == DEFAULT_MENU)
	{
		/*
		//�X�e�[�W���̒��S���W
		static const Vec2<float>STAGE_NAME_CENTER_POS = { WIN_CENTER_X,91.0f };
		//�X�e�[�W���`��
		auto stageNameTex = m_defaultMenu.m_stageNameDefaultTex;
		if (0 <= m_defaultMenu.m_stageNameIdx && m_defaultMenu.m_stageNameIdx < static_cast<int>(m_defaultMenu.m_stageNameTex.size()))stageNameTex = m_defaultMenu.m_stageNameTex[m_defaultMenu.m_stageNameIdx];
		DrawFunc2D::DrawRotaGraph2D(STAGE_NAME_CENTER_POS, { 1.0f,1.0f }, 0.0f, stageNameTex);

		//�X�e�[�W���̑��������`��
		static const Vec2<float>UNDER_LINE_CENTER_POS = { WIN_CENTER_X,132.0f };
		DrawFunc2D::DrawRotaGraph2D(UNDER_LINE_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_defaultMenu.m_underLineTex);
		*/

		//�Q�[���_���W�����p
		{
			//�X�e�[�W���̒��S���W
			static const Vec2<float>STAGE_NAME_CENTER_POS = { WIN_CENTER_X,91.0f };

			//�u�|�[�Y�v
			DrawFunc2D::DrawRotaGraph2D(STAGE_NAME_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_defaultMenu.m_pauseStrTex);

			//�X�e�[�W���̑��������`��
			static const Vec2<float>UNDER_LINE_CENTER_POS = { WIN_CENTER_X,132.0f };
			DrawFunc2D::DrawRotaGraph2D(UNDER_LINE_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_defaultMenu.m_underLineTex);
		}

		//��ԏ�̍��ڂ̒��S���W
		const Vec2<float>TOP_ITEM_CENTER_POS = { WIN_CENTER_X,215.0f };

		//���ڂ̍s��
		const float ITEM_SPACE_Y = 86.0f;

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
				DrawFunc2D::DrawRotaGraph2D(pos + m_selectItemShadowOffset, { 1.0f,1.0f }, m_selectItemShadowSpin, m_defaultMenu.m_selectItemShadowTex, SELECT_ITEM_SPIN_SHADOW_ALPHA);
				//��]����2
				DrawFunc2D::DrawRotaGraph2D(pos - m_selectItemShadowOffset, { 1.0f,1.0f }, -m_selectItemShadowSpin, m_defaultMenu.m_selectItemShadowTex, SELECT_ITEM_SPIN_SHADOW_ALPHA);
				//��]�Ȃ�
				DrawFunc2D::DrawRotaGraph2D(pos, { 1.0f,1.0f }, 0.0f, m_defaultMenu.m_selectItemShadowTex);
			}

			//�X�e�[�^�X
			ITEM_STATUS itemStatus = isSelected ? SELECT : DEFAULT;

			//�e�N�X�`������
			auto& tex = m_defaultMenu.m_itemTexArray[itemIdx][itemStatus];
			//�A���t�@����
			float alpha = isSelected ? 1.0f : NO_SELECT_ITEM_ALPHA;

			//�Z�[�u�f�[�^���Ȃ��̂Ńt�@�X�g�g���x���o���Ȃ�
			/*
			if (itemIdx == FAST_TRAVEL && !SaveDataManager::Instance()->LoadStageSaveData(nullptr, nullptr))
			{
				itemStatus = DEFAULT;
				alpha = 0.35f;
			}
			*/

			DrawFunc2D::DrawRotaGraph2D(pos, { 1.0f,1.0f }, 0.0f, tex, alpha);
		}

		/*
		//�ԕ`��
		const Vec2<float>FLOWER_CENTER_POS = { 1128.0f,86.0f };
		DrawFunc2D::DrawRotaGraph2D(FLOWER_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_defaultMenu.m_flowerTex);
		//�u x �v�`��
		const Vec2<float>FLOWER_MUL_CENTER_POS = { 1159.0f,127.0f };
		DrawFunc2D::DrawRotaGraph2D(FLOWER_MUL_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_defaultMenu.m_flowerMulTex);
		//�Ԃ̐��`��
		const Vec2<float>FLOWER_NUM_LEFT_UP_POS = { 1180.0f,102.0f };
		DrawFunc2D::DrawNumber2D(arg_totalGetFlowerNum, FLOWER_NUM_LEFT_UP_POS, m_defaultMenu.m_flowerNumTexArray.data());
		*/
	}
	else if (m_menuStatus == CONFIRM_MENU)
	{
		//�A�C�R���`��
		const Vec2<float>ICON_CENTER_POS = { WIN_CENTER_X,147.0f };
		DrawFunc2D::DrawRotaGraph2D(ICON_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_confirmMenu.m_iconTex);

		//����`��
		const Vec2<float>QUESTION_CENTER_POS = { WIN_CENTER_X,317.0f };
		DrawFunc2D::DrawRotaGraph2D(QUESTION_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_confirmMenu.m_questionTexArray[m_item]);

		//�u��낵���ł����H�v�`��
		const Vec2<float>MIND_CENTER_POS = { WIN_CENTER_X,457.0f };
		DrawFunc2D::DrawRotaGraph2D(MIND_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_confirmMenu.m_mindTex);

		//�u�͂��v�u�������v���S���W����̃I�t�Z�b�gX
		const float ANSWER_OFFSET_X = 130.0f;
		const Vec2<float>YES_CENTER_POS = { WIN_CENTER_X + ANSWER_OFFSET_X,525.0f };
		const Vec2<float>NO_CENTER_POS = { WIN_CENTER_X - ANSWER_OFFSET_X,525.0f };

		const Vec2<float>shadowPos = m_confirmMenu.m_confirmItem == ConfirmMenu::CONFIRM_ITEM::YES ? YES_CENTER_POS : NO_CENTER_POS;

		if (m_confirmMenu.m_confirmItem != ConfirmMenu::CONFIRM_ITEM::NONE)
		{
			//��]����1
			DrawFunc2D::DrawRotaGraph2D(shadowPos + m_selectItemShadowOffset, { 1.0f,1.0f }, m_selectItemShadowSpin, m_confirmMenu.m_answerShadowTex, SELECT_ITEM_SPIN_SHADOW_ALPHA);
			//��]����2
			DrawFunc2D::DrawRotaGraph2D(shadowPos - m_selectItemShadowOffset, { 1.0f,1.0f }, -m_selectItemShadowSpin, m_confirmMenu.m_answerShadowTex, SELECT_ITEM_SPIN_SHADOW_ALPHA);
			//��]�Ȃ�
			DrawFunc2D::DrawRotaGraph2D(shadowPos, { 1.0f,1.0f }, 0.0f, m_confirmMenu.m_answerShadowTex);
		}

		//�u�͂��v
		DrawFunc2D::DrawRotaGraph2D(YES_CENTER_POS, { 1.0f,1.0f }, 0.0f,
			m_confirmMenu.m_answerTexArray[ConfirmMenu::YES][m_confirmMenu.m_confirmItem == ConfirmMenu::CONFIRM_ITEM::YES ? SELECT : DEFAULT]);

		//�u�������v
		DrawFunc2D::DrawRotaGraph2D(NO_CENTER_POS, { 1.0f,1.0f }, 0.0f,
			m_confirmMenu.m_answerTexArray[ConfirmMenu::NO][m_confirmMenu.m_confirmItem == ConfirmMenu::CONFIRM_ITEM::NO ? SELECT : DEFAULT]);
	}
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
