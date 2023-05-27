#pragma once
#include"../Movie/MovieCamera.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"PazzleStageSelect.h"
#include"../SoundConfig.h"

/// <summary>
/// タイトル画面向けの処理
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
		//連続入力を防ぐためスキップする
		if (!m_delayInputFlag)
		{
			return -1;
		}

		//入力＝決定。ステージ番号を渡す
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
	//ゲーム開始フラグ
	bool m_startGameFlag;
	bool m_startPazzleFlag;
	//OPフラグ
	bool m_startOPFlag;
	bool m_generateCameraMoveDataFlag;

	bool m_doneFlag;

	//終了フラグ
	bool m_isFinishFlag;

	MovieCamera m_camera;

	//ステージ選択画面
	//パズルモード入力時に決定を1Fずらす事で連続で入力されることを防ぐ
	bool m_delayInputFlag;
	KuroEngine::Timer m_delayTime;

	KuroEngine::Vec2<float> m_pazzleModeLogoPos, m_storyModeLogoPos;

	//新しいやつ↓==============================================================
	enum TITLE_MENU_ITEM { CONTINUE, NEW_GAME, SETTING, QUIT, TITLE_MENU_ITEM_NUM }m_nowItem;
	struct Item
	{
		std::shared_ptr<KuroEngine::TextureBuffer>m_tex;
		KuroEngine::Vec2<float>m_offsetPos;
	};
	//項目配列
	std::array<Item, TITLE_MENU_ITEM_NUM>m_itemArray;

	//タイトルロゴテクスチャ
	std::shared_ptr<KuroEngine::TextureBuffer>m_titleLogoTex;
	//選択矢印テクスチャ
	std::shared_ptr<KuroEngine::TextureBuffer>m_selectArrowTex;
};

