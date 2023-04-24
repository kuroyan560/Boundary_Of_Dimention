#pragma once
#include"../Movie/MovieCamera.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"PazzleStageSelect.h"
#include"../SoundConfig.h"

enum TitleMode
{
	TITLE_SELECT,//選択画面に戻りたい時
	TITLE_PAZZLE //パズル画面に戻りたい時
};

/// <summary>
/// タイトル画面向けの処理
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
		//連続入力を防ぐためスキップする
		if (!m_delayInputFlag)
		{
			return -1;
		}

		//入力＝決定。ステージ番号を渡す
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
	//ゲーム開始フラグ
	bool m_startGameFlag;
	bool m_startPazzleFlag;
	bool m_isPazzleModeFlag;
	bool m_prevIsPazzleModeFlag;
	//OPフラグ
	bool m_startOPFlag;
	bool m_generateCameraMoveDataFlag;

	//入力フラグ
	bool m_isPrevInputControllerRight;
	bool m_isPrevInputControllerLeft;

	//終了フラグ
	bool m_isFinishFlag;

	MovieCamera m_camera;

	//タイトルロゴ
	KuroEngine::Vec2<float> m_titlePos, m_titleLogoSize;
	std::shared_ptr<KuroEngine::TextureBuffer> m_titleTexBuff;
	KuroEngine::Timer m_alphaRate;

	//ステージ選択画面
	//パズルモード入力時に決定を1Fずらす事で連続で入力されることを防ぐ
	bool m_delayInputFlag;
	KuroEngine::Timer m_delayTime;

	KuroEngine::Vec2<float> m_pazzleModeLogoPos, m_storyModeLogoPos;

	std::shared_ptr<KuroEngine::TextureBuffer> m_pazzleModeTexBuff;
	std::shared_ptr<KuroEngine::TextureBuffer> m_storyModeTexBuff;
};

