#pragma once
#include"../Movie/MovieCamera.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"PazzleStageSelect.h"
#include"../SoundConfig.h"

/// <summary>
/// �^�C�g����ʌ����̏���
/// </summary>
class Title
{
public:
	Title();
	void Init();
	void Update(KuroEngine::Transform *player_camera, std::shared_ptr<KuroEngine::Camera> arg_cam);
	void Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr);

	bool IsStartOP()
	{
		return m_startOPFlag;
	}
	bool IsFinish()
	{
		return m_isFinishFlag;
	}
	void FinishTitle()
	{
		m_isFinishFlag = true;
	}
	std::weak_ptr<KuroEngine::Camera>GetCamera()
	{
		if (!m_startPazzleFlag)
		{
			return m_camera.GetCamera();
		}
		else
		{
			return m_stageSelect.GetCamera();
		}
	}
	int GetStageNum()
	{
		if (!m_startPazzleFlag)
		{
			return -1;
		}
		//�A�����͂�h�����߃X�L�b�v����
		if (!m_delayInputFlag)
		{
			return -1;
		}

		//���́�����B�X�e�[�W�ԍ���n��
		bool inputFlag = KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_SPACE) || KuroEngine::UsersInput::Instance()->ControllerOnTrigger(0, KuroEngine::A);
		if (inputFlag && m_stageSelect.IsEnableToDone() && !m_doneFlag)
		{
			if (m_stageSelect.IsEnableToSelect())
			{
				m_doneFlag = true;
				m_stageSelect.Stop();
				return m_stageSelect.GetNumber();
			}
			else
			{
				return -1;
			}
		}
		return -1;
	}

	void Clear()
	{
		m_stageSelect.Clear();
	};

	PazzleStageSelect m_stageSelect;
private:
	//�Q�[���J�n�t���O
	bool m_startGameFlag;
	bool m_startPazzleFlag;
	//OP�t���O
	bool m_startOPFlag;
	bool m_generateCameraMoveDataFlag;

	bool m_doneFlag;

	//�I���t���O
	bool m_isFinishFlag;

	MovieCamera m_camera;

	//�X�e�[�W�I�����
	//�p�Y�����[�h���͎��Ɍ����1F���炷���ŘA���œ��͂���邱�Ƃ�h��
	bool m_delayInputFlag;
	KuroEngine::Timer m_delayTime;

	KuroEngine::Vec2<float> m_pazzleModeLogoPos, m_storyModeLogoPos;

	//�V�������==============================================================
	enum TITLE_MENU_ITEM { CONTINUE, NEW_GAME, SETTING, QUIT, TITLE_MENU_ITEM_NUM }m_nowItem;
	struct Item
	{
		std::shared_ptr<KuroEngine::TextureBuffer>m_tex;
		KuroEngine::Vec2<float>m_offsetPos;
	};
	//���ڔz��
	std::array<Item, TITLE_MENU_ITEM_NUM>m_itemArray;

	//�^�C�g�����S�e�N�X�`��
	std::shared_ptr<KuroEngine::TextureBuffer>m_titleLogoTex;
	//�I����e�N�X�`��
	std::shared_ptr<KuroEngine::TextureBuffer>m_selectArrowTex;
};

