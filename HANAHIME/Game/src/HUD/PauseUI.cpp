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
	//ポーズ直前のタイムスケール格納
	m_latestTimeScale = TimeScaleMgr::s_inGame.GetTimeScale();
	//ゲーム内時間を止める
	TimeScaleMgr::s_inGame.Set(0.0f);

	//sinカーブレート初期化
	m_sinCurveRateT = 0.0f;

	//パーリンノイズレート初期化
	m_perlinNoiseRateT = 0.0f;
	//パーリンノイズシード初期化
	m_perlinNoiseSeed.x = KuroEngine::GetRand(100);
	m_perlinNoiseSeed.y = KuroEngine::GetRand(100);

	//項目リセット
	m_item = (PAUSE_ITEM)0;

	//SE
	SoundConfig::Instance()->Play(SoundConfig::SE_DONE);

	//メニューステータスリセット
	m_menuStatus = DEFAULT_MENU;
}

void PauseUI::OnNonActive()
{
	//ゲーム内時間を再開
	TimeScaleMgr::s_inGame.Set(m_latestTimeScale);
}

PauseUI::PauseUI()
{
	using namespace KuroEngine;

	//ポーズ画面の画像リソースのディレクトリ
	std::string pauseTexDir = "resource/user/tex/pause/";
	//項目ごとの画像ファイル名
	std::array<std::string, PAUSE_ITEM_NUM>itemTexFileName =
	{
		"resume.png",
		"retry.png",
		//"fast_travel.png",
		"stage_select.png",
		"setting.png",
		"return_to_title.png"
	};

	//メニュー
	{
		std::string menuTexDir = pauseTexDir + "menu/";

		//項目ごとの画像読み込み
		for (int itemIdx = 0; itemIdx < PAUSE_ITEM_NUM; ++itemIdx)
		{
			D3D12App::Instance()->GenerateTextureBuffer(m_defaultMenu.m_itemTexArray[itemIdx].data(),
				menuTexDir + itemTexFileName[itemIdx], ITEM_STATUS_NUM, { 1,ITEM_STATUS_NUM });
		}
		//選択中の項目にのみ出る影画像読み込み
		m_defaultMenu.m_selectItemShadowTex = D3D12App::Instance()->GenerateTextureBuffer(menuTexDir + "shadow.png");

		//ステージ関連のテクスチャのディレクトリ
		std::string stageTexDir = "resource/user/tex/stage/";
		//ステージ名テクスチャ
		int stageIdx = 0;
		while (1)
		{
			std::string path = stageTexDir + std::to_string(stageIdx++) + ".png";
			if (!ExistFile(path))break;

			m_defaultMenu.m_stageNameTex.emplace_back(D3D12App::Instance()->GenerateTextureBuffer(path));
		}
		m_defaultMenu.m_stageNameDefaultTex = D3D12App::Instance()->GenerateTextureBuffer(stageTexDir + "default.png");
		//ステージ名の装飾下線画像
		m_defaultMenu.m_underLineTex = D3D12App::Instance()->GenerateTextureBuffer(stageTexDir + "under_line.png");

		//花の画像読み込み
		m_defaultMenu.m_flowerTex = D3D12App::Instance()->GenerateTextureBuffer(pauseTexDir + "flower.png");
		//収集花の数テクスチャ読み込み
		D3D12App::Instance()->GenerateTextureBuffer(m_defaultMenu.m_flowerNumTexArray.data(),
			pauseTexDir + "flower_num.png", DefaultMenu::FLOWER_NUM_TEX_SIZE, { DefaultMenu::FLOWER_NUM_TEX_SIZE,1 });
		//収集花の「 x 」テクスチャ読み込み
		m_defaultMenu.m_flowerMulTex = D3D12App::Instance()->GenerateTextureBuffer(pauseTexDir + "mul.png");

		//「ポーズ」
		m_defaultMenu.m_pauseStrTex = D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/pause/pause_str.png");
	}

	//最終確認メニュー
	{
		std::string menuTexDir = pauseTexDir + "confirm/";

		//アイコン画像読み込み
		m_confirmMenu.m_iconTex = D3D12App::Instance()->GenerateTextureBuffer(menuTexDir + "icon.png");
		//「いいえ」画像読み込み
		D3D12App::Instance()->GenerateTextureBuffer(m_confirmMenu.m_answerTexArray[ConfirmMenu::NO].data(), menuTexDir + "no.png", ITEM_STATUS_NUM, { 1,ITEM_STATUS_NUM });
		//「はい」画像読み込み
		D3D12App::Instance()->GenerateTextureBuffer(m_confirmMenu.m_answerTexArray[ConfirmMenu::YES].data(), menuTexDir + "yes.png", ITEM_STATUS_NUM, { 1,ITEM_STATUS_NUM });
		//「いいえ」「はい」の選択側に出る影画像読み込み
		m_confirmMenu.m_answerShadowTex = D3D12App::Instance()->GenerateTextureBuffer(menuTexDir + "shadow.png");
		//「よろしいですか？」画像
		m_confirmMenu.m_mindTex = D3D12App::Instance()->GenerateTextureBuffer(menuTexDir + "mind.png");

		//選んだ項目に応じた質問画像
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
	//非アクティブ
	if (!m_isActive)return;

	using namespace KuroEngine;

	const float SIN_CURVE_INTERVAL = 60.0f;
	const float PERLIN_NOISE_INTERVAL = 90.0f;

	//sinカーブ更新
	m_sinCurveRateT += 1.0f / SIN_CURVE_INTERVAL * arg_timeScale;

	//パーリンノイズ更新
	m_perlinNoiseRateT += 1.0f / PERLIN_NOISE_INTERVAL * arg_timeScale;

	//選択中の影のパーリンノイズ回転
	const Angle SELECT_ITEM_SHADOW_SPIN_MAX = Angle(5);
	m_selectItemShadowSpin = sin(m_sinCurveRateT * Angle::ROUND()) * SELECT_ITEM_SHADOW_SPIN_MAX;

	//選択中の影のパーリンノイズオフセット
	const Vec2<float>SELECT_ITEM_SHADOW_OFFSET_MAX = { 32.0f,64.0f };
	m_selectItemShadowOffset.x = PerlinNoise::GetRand(m_perlinNoiseRateT, 0.0f, m_perlinNoiseSeed.x) * SELECT_ITEM_SHADOW_OFFSET_MAX.x;
	m_selectItemShadowOffset.y = PerlinNoise::GetRand(m_perlinNoiseRateT, 0.0f, m_perlinNoiseSeed.y) * SELECT_ITEM_SHADOW_OFFSET_MAX.y;

	//決定ボタン
	bool doneInput = OperationConfig::Instance()->GetOperationInput(OperationConfig::DONE, OperationConfig::ON_TRIGGER);
	//キャンセルボタン
	bool cancelInput = OperationConfig::Instance()->GetOperationInput(OperationConfig::CANCEL, OperationConfig::ON_TRIGGER);
	//上入力
	bool upInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_UP);
	//下入力
	bool downInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_DOWN);
	//左入力
	bool leftInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_LEFT);
	//右入力
	bool rightInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_RIGHT);

	//通常メニュー
	if (m_menuStatus == DEFAULT_MENU)
	{
		//項目の移動
		auto oldItem = m_item;
		if (m_item < PAUSE_ITEM_NUM - 1 && downInput)m_item = PAUSE_ITEM(m_item + 1);		//下へ
		else if (0 < m_item && upInput)m_item = PAUSE_ITEM(m_item - 1);		//上へ

		//項目に変化があった
		if (oldItem != m_item)
		{
			//SE再生
			SoundConfig::Instance()->Play(SoundConfig::SE_SELECT);

			//セーブデータがないのでファストトラベル出来ない
			//if (m_item == FAST_TRAVEL && !SaveDataManager::Instance()->LoadStageSaveData(nullptr, nullptr))
			//{
			//	if (m_item < oldItem)m_item = PAUSE_ITEM(FAST_TRAVEL - 1);
			//	else if (oldItem < m_item)m_item = PAUSE_ITEM(FAST_TRAVEL + 1);
			//}
		}

		//決定ボタン
		if (doneInput)
		{
			switch (m_item)
			{
				//ゲームを再開
				case RESUME:
					this->SetInverseActive();
					break;
				//リトライ
				case RETRY:
					//ゲームダンジョン用
					arg_gameScene->Retry();
					//ゲーム内時間を再開
					TimeScaleMgr::s_inGame.Set(m_latestTimeScale);
					m_isActive = false;
					//※インゲーム操作入力は受け付けないまま

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
					//ゲーム内時間を再開
					TimeScaleMgr::s_inGame.Set(m_latestTimeScale);
					m_isActive = false;
					//※インゲーム操作入力は受け付けないまま

					/*
					m_menuStatus = CONFIRM_MENU;
					m_confirmMenu.m_confirmItem = ConfirmMenu::CONFIRM_ITEM::NONE;
					*/
					break;
				default:
					break;
			}
			//SE再生
			SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
		}

		if (cancelInput)
		{
			//ポーズ終了
			this->SetInverseActive();
			//SE再生
			SoundConfig::Instance()->Play(SoundConfig::SE_CANCEL);
		}
	}
	else if (m_menuStatus == CONFIRM_MENU)
	{
		//項目移動
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

			//決定
			if (doneInput)
			{
				if (m_confirmMenu.m_confirmItem == ConfirmMenu::CONFIRM_ITEM::YES)
				{
					if (m_item == RETRY)
					{
						arg_gameScene->Retry();
						//ゲーム内時間を再開
						TimeScaleMgr::s_inGame.Set(m_latestTimeScale);
						m_isActive = false;
						//※インゲーム操作入力は受け付けないまま
					}
					else if (m_item == RETURN_TO_TITLE)
					{
						arg_gameScene->GoBackTitle();
						//ゲーム内時間を再開
						TimeScaleMgr::s_inGame.Set(m_latestTimeScale);
						m_isActive = false;
						//※インゲーム操作入力は受け付けないまま
					}
				}
				else m_menuStatus = DEFAULT_MENU;

				SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
			}
			//キャンセル
			else if (cancelInput)
			{
				m_menuStatus = DEFAULT_MENU;
				SoundConfig::Instance()->Play(SoundConfig::SE_CANCEL);
			}
		}

		//項目に変化があった
		if (oldItem != m_confirmMenu.m_confirmItem)
		{
			SoundConfig::Instance()->Play(SoundConfig::SE_SELECT);
		}
	}
}

void PauseUI::Draw(int arg_totalGetFlowerNum)
{
	//非アクティブ
	if (!m_isActive)return;

	using namespace KuroEngine;

	//ウィンドウの中心座標X
	static const float WIN_CENTER_X = WinApp::Instance()->GetExpandWinCenter().x;
	//ウィンドウサイズ
	static const auto WIN_SIZE = WinApp::Instance()->GetExpandWinSize();

	//選択されていない項目のアルファ
	const float NO_SELECT_ITEM_ALPHA = 0.8f;
	//選択中項目の回転影のアルファ
	const float SELECT_ITEM_SPIN_SHADOW_ALPHA = 0.4f;

	//半透明黒四角背景描画
	static const float SQUARE_WIDTH_HALF = 284.0f;
	static const float SQUARE_ALPHA = 0.5f;
	DrawFunc2D::DrawBox2D(
		{ WIN_CENTER_X - SQUARE_WIDTH_HALF,0.0f }, { WIN_CENTER_X + SQUARE_WIDTH_HALF,WIN_SIZE.y },
		Color(0.0f, 0.0f, 0.0f, SQUARE_ALPHA), true);

	//通常メニュー
	if (m_menuStatus == DEFAULT_MENU)
	{
		/*
		//ステージ名の中心座標
		static const Vec2<float>STAGE_NAME_CENTER_POS = { WIN_CENTER_X,91.0f };
		//ステージ名描画
		auto stageNameTex = m_defaultMenu.m_stageNameDefaultTex;
		if (0 <= m_defaultMenu.m_stageNameIdx && m_defaultMenu.m_stageNameIdx < static_cast<int>(m_defaultMenu.m_stageNameTex.size()))stageNameTex = m_defaultMenu.m_stageNameTex[m_defaultMenu.m_stageNameIdx];
		DrawFunc2D::DrawRotaGraph2D(STAGE_NAME_CENTER_POS, { 1.0f,1.0f }, 0.0f, stageNameTex);

		//ステージ名の装飾下線描画
		static const Vec2<float>UNDER_LINE_CENTER_POS = { WIN_CENTER_X,132.0f };
		DrawFunc2D::DrawRotaGraph2D(UNDER_LINE_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_defaultMenu.m_underLineTex);
		*/

		//ゲームダンジョン用
		{
			//ステージ名の中心座標
			static const Vec2<float>STAGE_NAME_CENTER_POS = { WIN_CENTER_X,91.0f };

			//「ポーズ」
			DrawFunc2D::DrawRotaGraph2D(STAGE_NAME_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_defaultMenu.m_pauseStrTex);

			//ステージ名の装飾下線描画
			static const Vec2<float>UNDER_LINE_CENTER_POS = { WIN_CENTER_X,132.0f };
			DrawFunc2D::DrawRotaGraph2D(UNDER_LINE_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_defaultMenu.m_underLineTex);
		}

		//一番上の項目の中心座標
		const Vec2<float>TOP_ITEM_CENTER_POS = { WIN_CENTER_X,215.0f };

		//項目の行間
		const float ITEM_SPACE_Y = 86.0f;

		//項目の描画
		for (int itemIdx = 0; itemIdx < PAUSE_ITEM_NUM; ++itemIdx)
		{
			//座標計算
			const auto pos = TOP_ITEM_CENTER_POS + Vec2<float>(0.0f, ITEM_SPACE_Y * itemIdx);
			//項目が選択中か
			bool isSelected = (PAUSE_ITEM)itemIdx == m_item;

			//選択中なら影描画
			if (isSelected)
			{
				//回転あり1
				DrawFunc2D::DrawRotaGraph2D(pos + m_selectItemShadowOffset, { 1.0f,1.0f }, m_selectItemShadowSpin, m_defaultMenu.m_selectItemShadowTex, SELECT_ITEM_SPIN_SHADOW_ALPHA);
				//回転あり2
				DrawFunc2D::DrawRotaGraph2D(pos - m_selectItemShadowOffset, { 1.0f,1.0f }, -m_selectItemShadowSpin, m_defaultMenu.m_selectItemShadowTex, SELECT_ITEM_SPIN_SHADOW_ALPHA);
				//回転なし
				DrawFunc2D::DrawRotaGraph2D(pos, { 1.0f,1.0f }, 0.0f, m_defaultMenu.m_selectItemShadowTex);
			}

			//ステータス
			ITEM_STATUS itemStatus = isSelected ? SELECT : DEFAULT;

			//テクスチャ決定
			auto& tex = m_defaultMenu.m_itemTexArray[itemIdx][itemStatus];
			//アルファ決定
			float alpha = isSelected ? 1.0f : NO_SELECT_ITEM_ALPHA;

			//セーブデータがないのでファストトラベル出来ない
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
		//花描画
		const Vec2<float>FLOWER_CENTER_POS = { 1128.0f,86.0f };
		DrawFunc2D::DrawRotaGraph2D(FLOWER_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_defaultMenu.m_flowerTex);
		//「 x 」描画
		const Vec2<float>FLOWER_MUL_CENTER_POS = { 1159.0f,127.0f };
		DrawFunc2D::DrawRotaGraph2D(FLOWER_MUL_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_defaultMenu.m_flowerMulTex);
		//花の数描画
		const Vec2<float>FLOWER_NUM_LEFT_UP_POS = { 1180.0f,102.0f };
		DrawFunc2D::DrawNumber2D(arg_totalGetFlowerNum, FLOWER_NUM_LEFT_UP_POS, m_defaultMenu.m_flowerNumTexArray.data());
		*/
	}
	else if (m_menuStatus == CONFIRM_MENU)
	{
		//アイコン描画
		const Vec2<float>ICON_CENTER_POS = { WIN_CENTER_X,147.0f };
		DrawFunc2D::DrawRotaGraph2D(ICON_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_confirmMenu.m_iconTex);

		//質問描画
		const Vec2<float>QUESTION_CENTER_POS = { WIN_CENTER_X,317.0f };
		DrawFunc2D::DrawRotaGraph2D(QUESTION_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_confirmMenu.m_questionTexArray[m_item]);

		//「よろしいですか？」描画
		const Vec2<float>MIND_CENTER_POS = { WIN_CENTER_X,457.0f };
		DrawFunc2D::DrawRotaGraph2D(MIND_CENTER_POS, { 1.0f,1.0f }, 0.0f, m_confirmMenu.m_mindTex);

		//「はい」「いいえ」中心座標からのオフセットX
		const float ANSWER_OFFSET_X = 130.0f;
		const Vec2<float>YES_CENTER_POS = { WIN_CENTER_X + ANSWER_OFFSET_X,525.0f };
		const Vec2<float>NO_CENTER_POS = { WIN_CENTER_X - ANSWER_OFFSET_X,525.0f };

		const Vec2<float>shadowPos = m_confirmMenu.m_confirmItem == ConfirmMenu::CONFIRM_ITEM::YES ? YES_CENTER_POS : NO_CENTER_POS;

		if (m_confirmMenu.m_confirmItem != ConfirmMenu::CONFIRM_ITEM::NONE)
		{
			//回転あり1
			DrawFunc2D::DrawRotaGraph2D(shadowPos + m_selectItemShadowOffset, { 1.0f,1.0f }, m_selectItemShadowSpin, m_confirmMenu.m_answerShadowTex, SELECT_ITEM_SPIN_SHADOW_ALPHA);
			//回転あり2
			DrawFunc2D::DrawRotaGraph2D(shadowPos - m_selectItemShadowOffset, { 1.0f,1.0f }, -m_selectItemShadowSpin, m_confirmMenu.m_answerShadowTex, SELECT_ITEM_SPIN_SHADOW_ALPHA);
			//回転なし
			DrawFunc2D::DrawRotaGraph2D(shadowPos, { 1.0f,1.0f }, 0.0f, m_confirmMenu.m_answerShadowTex);
		}

		//「はい」
		DrawFunc2D::DrawRotaGraph2D(YES_CENTER_POS, { 1.0f,1.0f }, 0.0f,
			m_confirmMenu.m_answerTexArray[ConfirmMenu::YES][m_confirmMenu.m_confirmItem == ConfirmMenu::CONFIRM_ITEM::YES ? SELECT : DEFAULT]);

		//「いいえ」
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

	//インゲームの操作入力受付切り替え
	OperationConfig::Instance()->SetInGameOperationActive(!m_isActive);
}
