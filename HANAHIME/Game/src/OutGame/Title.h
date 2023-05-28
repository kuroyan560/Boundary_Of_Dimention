#pragma once
#include"../Movie/MovieCamera.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"PazzleStageSelect.h"
#include"../SoundConfig.h"

class GameScene;

/// <summary>
/// �^�C�g����ʌ����̏���
/// </summary>
class Title
{
	//�ʏ�̃��j���[
	void MenuUpdate(bool arg_inputUp, bool arg_inputDown, bool arg_inputDone, GameScene* arg_gameScene);
	void MenuDraw();

	void ConfirmNewGameUpdate(bool arg_inputLeft, bool arg_inputRight, bool arg_inputDone, GameScene* arg_gameScene);
	void ConfirmNewGameDraw();

public:
	Title();
	void Init();
	void Update(KuroEngine::Transform* player_camera, std::shared_ptr<KuroEngine::Camera> arg_cam, GameScene* arg_gameScene);
	void Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr);

	//bool IsStartOP()
	//{
		//return m_startOPFlag;
	//}
	/*bool IsFinish()
	{
		return m_isFinishFlag;
	}
	void FinishTitle()
	{
		m_isFinishFlag = true;
	}*/
	std::weak_ptr<KuroEngine::Camera>GetCamera()
	{
		/*if (!m_startPazzleFlag)
		{*/
			return m_camera.GetCamera();
		/*}
		else
		{
			return m_stageSelect.GetCamera();
		}*/
	}
	//int GetStageNum()
	//{
	//	if (!m_startPazzleFlag)
	//	{
	//		return -1;
	//	}
	//	//�A�����͂�h�����߃X�L�b�v����
	//	if (!m_delayInputFlag)
	//	{
	//		return -1;
	//	}

	//	//���́�����B�X�e�[�W�ԍ���n��
	//	bool inputFlag = KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_SPACE) || KuroEngine::UsersInput::Instance()->ControllerOnTrigger(0, KuroEngine::A);
	//	if (inputFlag && m_stageSelect.IsEnableToDone() && !m_doneFlag)
	//	{
	//		if (m_stageSelect.IsEnableToSelect())
	//		{
	//			m_doneFlag = true;
	//			m_stageSelect.Stop();
	//			return m_stageSelect.GetNumber();
	//		}
	//		else
	//		{
	//			return -1;
	//		}
	//	}
	//	return -1;
	//}

	/*void Clear()
	{
		m_stageSelect.Clear();
	};*/

	//PazzleStageSelect m_stageSelect;
private:
	//�Q�[���J�n�t���O
	//bool m_startGameFlag;
	//bool m_startPazzleFlag;
	//OP�t���O
	//bool m_startOPFlag;
	//bool m_generateCameraMoveDataFlag;

	//bool m_doneFlag;

	//�I���t���O
	//bool m_isFinishFlag;

	MovieCamera m_camera;

	//�X�e�[�W�I�����
	//�p�Y�����[�h���͎��Ɍ����1F���炷���ŘA���œ��͂���邱�Ƃ�h��
	//bool m_delayInputFlag;
	//KuroEngine::Timer m_delayTime;

	//KuroEngine::Vec2<float> m_pazzleModeLogoPos, m_storyModeLogoPos;

	//�V�������==============================================================
	enum TITLE_MODE { MODE_MENU, MODE_CONFIRM_NEW_GAME }m_mode;
	enum TITLE_MENU_ITEM { CONTINUE, NEW_GAME, SETTING, QUIT, TITLE_MENU_ITEM_NUM }m_nowItem;
	enum ITEM_STATUS { DEFAULT, SELECT, ITEM_STATUS_NUM };
	struct Item
	{
		std::array<std::shared_ptr<KuroEngine::TextureBuffer>, ITEM_STATUS_NUM>m_texArray;
		KuroEngine::Vec2<float>m_offsetPos;
		ITEM_STATUS m_status;
		std::shared_ptr<KuroEngine::TextureBuffer>GetTex() { return m_texArray[m_status]; }
	};
	//���ڔz��
	std::array<Item, TITLE_MENU_ITEM_NUM>m_itemArray;

	//�^�C�g�����S�e�N�X�`��
	std::shared_ptr<KuroEngine::TextureBuffer>m_titleLogoTex;
	//�I���e�e�N�X�`��
	std::shared_ptr<KuroEngine::TextureBuffer>m_selectShadowTex;

	//�u�͂��߂���v�̍ŏI�m�F
	struct ConfirmNewGame
	{
		bool m_isNo = true;
		//�A�C�R��
		std::shared_ptr<KuroEngine::TextureBuffer>m_iconTex;
		//����
		std::shared_ptr<KuroEngine::TextureBuffer>m_strTex;
		//�͂�
		std::array<std::shared_ptr<KuroEngine::TextureBuffer>, ITEM_STATUS_NUM>m_yesTex;
		//������
		std::array<std::shared_ptr<KuroEngine::TextureBuffer>, ITEM_STATUS_NUM>m_noTex;
		//�I�𒆂̉e
		std::shared_ptr<KuroEngine::TextureBuffer>m_shadowTex;

		std::shared_ptr<KuroEngine::TextureBuffer>GetYesTex()
		{
			return m_yesTex[m_isNo ? DEFAULT : SELECT];
		}
		std::shared_ptr<KuroEngine::TextureBuffer>GetNoTex()
		{
			return m_noTex[m_isNo ? SELECT : DEFAULT];
		}

	}m_confirmNewGame;
};

