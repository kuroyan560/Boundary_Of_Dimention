#pragma once
#include"../Movie/MovieCamera.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"PazzleStageSelect.h"
#include"../SoundConfig.h"

enum TitleMode
{
	TITLE_SELECT,//�I����ʂɖ߂肽����
	TITLE_PAZZLE //�p�Y����ʂɖ߂肽����
};

/// <summary>
/// �^�C�g����ʌ����̏���
/// </summary>
class Title
{
public:
	Title();
	void Init(TitleMode title_mode);
	void Update(KuroEngine::Transform *player_camera);
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
			return m_stageSelect.m_camera.GetCamera();
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
		if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_SPACE) || KuroEngine::UsersInput::Instance()->ControllerOnTrigger(0,KuroEngine::A))
		{
			if (m_stageSelect.IsEnableToSelect())
			{
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
	bool m_isPazzleModeFlag;
	bool m_prevIsPazzleModeFlag;
	//OP�t���O
	bool m_startOPFlag;
	bool m_generateCameraMoveDataFlag;

	//���̓t���O
	bool m_isPrevInputControllerRight;
	bool m_isPrevInputControllerLeft;

	//�I���t���O
	bool m_isFinishFlag;

	MovieCamera m_camera;

	//�^�C�g�����S
	KuroEngine::Vec2<float> m_titlePos, m_titleLogoSize;
	std::shared_ptr<KuroEngine::TextureBuffer> m_titleTexBuff;
	KuroEngine::Timer m_alphaRate;

	//�X�e�[�W�I�����
	//�p�Y�����[�h���͎��Ɍ����1F���炷���ŘA���œ��͂���邱�Ƃ�h��
	bool m_delayInputFlag;
	KuroEngine::Timer m_delayTime;

	KuroEngine::Vec2<float> m_pazzleModeLogoPos, m_storyModeLogoPos;

	std::shared_ptr<KuroEngine::TextureBuffer> m_pazzleModeTexBuff;
	std::shared_ptr<KuroEngine::TextureBuffer> m_storyModeTexBuff;
};

