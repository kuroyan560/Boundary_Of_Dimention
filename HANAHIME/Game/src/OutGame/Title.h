#pragma once
#include"../Movie/MovieCamera.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"PazzleStageSelect.h"

/// <summary>
/// タイトル画面向けの処理
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
		//連続入力を防ぐためスキップする
		if (!m_delayInputFlag)
		{
			return -1;
		}

		//入力＝決定。ステージ番号を渡す
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
	//ゲーム開始フラグ
	bool m_startGameFlag;
	bool m_startPazzleFlag;
	bool m_isPazzleModeFlag;
	//OPフラグ
	bool m_startOPFlag;
	bool m_generateCameraMoveDataFlag;

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
	PazzleStageSelect m_stageSelect;
	std::shared_ptr<KuroEngine::TextureBuffer> m_pazzleModeTexBuff;
	std::shared_ptr<KuroEngine::TextureBuffer> m_storyModeTexBuff;
};

