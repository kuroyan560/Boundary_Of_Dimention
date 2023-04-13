#pragma once
#include"../Movie/MovieCamera.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"PazzleStageSelect.h"

/// <summary>
/// �^�C�g����ʌ����̏���
/// </summary>
class Title
{
public:
	Title();
	void Init();
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
		return m_camera.GetCamera();
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
		if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_SPACE))
		{
			if (m_stageSelect.IsEnableToSelect())
			{
				return m_stageSelect.GetNumber();
			}
			else
			{
				return -1;
			}
		}
		return -1;
	}

private:
	//�Q�[���J�n�t���O
	bool m_startGameFlag;
	bool m_startPazzleFlag;
	bool m_isPazzleModeFlag;
	//OP�t���O
	bool m_startOPFlag;
	bool m_generateCameraMoveDataFlag;

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
	PazzleStageSelect m_stageSelect;
	std::shared_ptr<KuroEngine::TextureBuffer> m_pazzleModeTexBuff;
	std::shared_ptr<KuroEngine::TextureBuffer> m_storyModeTexBuff;
};

