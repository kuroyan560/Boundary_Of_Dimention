#include "SystemSetting.h"
#include"DirectX12/D3D12App.h"
#include"../OperationConfig.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"FrameWork/WinApp.h"
#include"../SoundConfig.h"
#include"SaveDataManager.h"

std::string SystemSetting::TEX_DIR = "resource/user/tex/setting/";

void SystemSetting::SelectItem::LoadTex(std::string arg_path)
{
	KuroEngine::D3D12App::Instance()->GenerateTextureBuffer(m_tex.data(), arg_path, ITEM_STATUS_NUM, { 1,ITEM_STATUS_NUM });
}

SystemSetting::MainMenuGroup::MainMenuGroup()
{
	using namespace KuroEngine;
	//見出し
	m_headTex = D3D12App::Instance()->GenerateTextureBuffer(TEX_DIR + "head.png");
	//「音量」
	m_items[ITEM_SOUND].LoadTex(TEX_DIR + "sound.png");
	//「操作」
	m_items[ITEM_OPE].LoadTex(TEX_DIR + "ope.png");
	//「戻る」
	m_items[ITEM_BACK].LoadTex(TEX_DIR + "back.png");
}

void SystemSetting::MainMenuGroup::Update(SystemSetting* arg_parent)
{
	bool upInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_UP);
	bool downInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_DOWN);
	bool doneInput = OperationConfig::Instance()->GetOperationInput(OperationConfig::DONE, OperationConfig::ON_TRIGGER);
	bool cancelInput = OperationConfig::Instance()->GetOperationInput(OperationConfig::CANCEL, OperationConfig::ON_TRIGGER);

	//変化前の項目
	auto oldItem = m_nowItem;

	//項目上へ
	if (0 < m_nowItem && upInput)m_nowItem = (ITEM)(m_nowItem - 1);
	//項目下へ
	if (m_nowItem < ITEM_NUM - 1 && downInput)m_nowItem = (ITEM)(m_nowItem + 1);

	//項目変化あり
	if (oldItem != m_nowItem)SoundConfig::Instance()->Play(SoundConfig::SE_SELECT);

	//決定
	if (doneInput)
	{
		//音量設定
		if (m_nowItem == ITEM_SOUND)arg_parent->m_mode = MODE_SOUND;
		//操作設定
		else if (m_nowItem == ITEM_OPE)arg_parent->m_mode = MODE_OPERATION;
		//戻る
		else arg_parent->m_isActive = false;

		SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
	}

	//キャンセル
	if (cancelInput)
	{
		SoundConfig::Instance()->Play(SoundConfig::SE_CANCEL);
		arg_parent->m_isActive = false;
	}
}

void SystemSetting::MainMenuGroup::Draw()
{
	using namespace KuroEngine;

	//ウィンドウセンター
	const auto winCenter = WinApp::Instance()->GetExpandWinCenter();

	//見出し
	DrawFunc2D::DrawRotaGraph2D({ winCenter.x,171.0f }, { 1.0f,1.0f }, 0.0f, m_headTex);

	//項目の描画
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

const float SystemSetting::SoundMenuGroup::VOL_CHANGE = 1.0f / static_cast<float>(VOL_STAGE_NUM);

SystemSetting::SoundMenuGroup::SoundMenuGroup()
{
	using namespace KuroEngine;

	const std::string dir = TEX_DIR + "soundMenu/";
	//見出し
	m_headTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "head.png");
	//「マスター音量」
	m_items[ITEM_MASTER].LoadTex(dir + "master.png");
	//「サウンドエフェクト」
	m_items[ITEM_SE].LoadTex(dir + "se.png");
	//「背景音楽」
	m_items[ITEM_BGM].LoadTex(dir + "bgm.png");
	//「戻る」
	m_items[ITEM_BACK].LoadTex(TEX_DIR + "back.png");
	//地面ゲージ
	m_groundLineTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "ground_line.png");
	//ゲージのパターン数
	D3D12App::Instance()->GenerateTextureBuffer(m_gagePatternTex.data(), dir + "pattern.png", GAGE_PATTERN_NUM, { GAGE_PATTERN_NUM ,1 });

	for (auto& array : m_gageParam)array.reserve(VOL_STAGE_NUM);

	//サウンドボリュームデータ参照取得
	auto& soundVolData = SaveDataManager::Instance()->m_saveData.m_soundVol;
	m_gageParam[ITEM_MASTER].resize(static_cast<int>(soundVolData.m_masterVolume / VOL_CHANGE));
	m_gageParam[ITEM_SE].resize(soundVolData.m_seVolume / VOL_CHANGE);
	m_gageParam[ITEM_BGM].resize(static_cast<int>(soundVolData.m_bgmVolume / VOL_CHANGE));
	//ゲージのパターンの決定
	for (auto& patternArray : m_gageParam)
	{
		for (auto& patternIdx : patternArray)patternIdx = GetPattern();
	}
}

void SystemSetting::SoundMenuGroup::Init()
{
	m_nowItem = ITEM_MASTER;
}

void SystemSetting::SoundMenuGroup::Update(SystemSetting* arg_parent)
{
	bool upInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_UP);
	bool downInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_DOWN);
	bool leftInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_LEFT);
	bool rightInput = OperationConfig::Instance()->GetSelectVec(OperationConfig::SELECT_VEC_RIGHT);
	bool doneInput = OperationConfig::Instance()->GetOperationInput(OperationConfig::DONE, OperationConfig::ON_TRIGGER);
	bool cancelInput = OperationConfig::Instance()->GetOperationInput(OperationConfig::CANCEL, OperationConfig::ON_TRIGGER);

	//変化前の項目
	auto oldItem = m_nowItem;
	//項目上へ
	if (0 < m_nowItem && upInput)m_nowItem = (ITEM)(m_nowItem - 1);
	//項目下へ
	if (m_nowItem < ITEM_NUM - 1 && downInput)m_nowItem = (ITEM)(m_nowItem + 1);
	//項目変化あり
	if (oldItem != m_nowItem)SoundConfig::Instance()->Play(SoundConfig::SE_SELECT);

	//サウンドボリュームデータ参照取得
	auto& soundVolData = SaveDataManager::Instance()->m_saveData.m_soundVol;

	//音量上げ下げ
	bool volChange = false;
	if (leftInput)
	{
		if (m_nowItem == ITEM_MASTER)soundVolData.m_masterVolume = std::clamp(soundVolData.m_masterVolume - VOL_CHANGE, 0.0f, 1.0f);
		else if (m_nowItem == ITEM_SE)soundVolData.m_seVolume = std::clamp(soundVolData.m_seVolume - VOL_CHANGE, 0.0f, 1.0f);
		else if (m_nowItem == ITEM_BGM)soundVolData.m_bgmVolume = std::clamp(soundVolData.m_bgmVolume - VOL_CHANGE, 0.0f, 1.0f);

		if (!m_gageParam[m_nowItem].empty())
		{
			m_gageParam[m_nowItem].pop_back();
			volChange = true;
		}
	}
	else if (rightInput)
	{
		if (m_nowItem == ITEM_MASTER)soundVolData.m_masterVolume = std::clamp(soundVolData.m_masterVolume + VOL_CHANGE, 0.0f, 1.0f);
		else if (m_nowItem == ITEM_SE)soundVolData.m_seVolume = std::clamp(soundVolData.m_seVolume + VOL_CHANGE, 0.0f, 1.0f);
		else if (m_nowItem == ITEM_BGM)soundVolData.m_bgmVolume = std::clamp(soundVolData.m_bgmVolume + VOL_CHANGE, 0.0f, 1.0f);

		if (static_cast<int>(m_gageParam[m_nowItem].size()) < VOL_STAGE_NUM)
		{
			m_gageParam[m_nowItem].emplace_back(GetPattern());
			volChange = true;
		}
	}
	//音量に変化あり
	if (volChange)
	{
		SoundConfig::Instance()->UpdateIndividualVolume();
		SoundConfig::Instance()->Play(SoundConfig::SE_GRASS);
	}

	//決定「戻る」
	if (doneInput && m_nowItem == ITEM_BACK)
	{
		arg_parent->m_mode = MODE_MAIN_MENU;
		SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
	}
	//キャンセル＝「戻る」
	if (cancelInput)
	{
		arg_parent->m_mode = MODE_MAIN_MENU;
		SoundConfig::Instance()->Play(SoundConfig::SE_CANCEL);
	}
}

void SystemSetting::SoundMenuGroup::Draw()
{
	using namespace KuroEngine;

	//見出し
	DrawFunc2D::DrawGraph({ 0.0f,32.0f }, m_headTex);

	//地面ゲージのX
	const float GROUND_GAGE_LEFT_X = 588.0f;
	const float GROUND_GAGE_RIGHT_X = GROUND_GAGE_LEFT_X + m_groundLineTex->GetGraphSize().x;
	const float GROUND_GAGE_OFFSEY_Y = 16.0f;

	//パターンごとの画像の高さ
	const std::array<float, GAGE_PATTERN_NUM>GAGE_PATTERN_TEX_HEGIHT =
	{
		m_gagePatternTex[0]->GetGraphSize().y,
		m_gagePatternTex[1]->GetGraphSize().y,
		m_gagePatternTex[2]->GetGraphSize().y,
	};
	//ゲージの間隔X
	const float GAGE_SPACE_X = m_groundLineTex->GetGraphSize().x / static_cast<float>(VOL_STAGE_NUM);

	//項目の描画
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

		//地面ゲージ
		if (itemIdx != ITEM_BACK)
		{
			Vec2<float>groundPos = { GROUND_GAGE_LEFT_X,ITEM_CENTER_POS[itemIdx].y + GROUND_GAGE_OFFSEY_Y };
			DrawFunc2D::DrawGraph(groundPos, m_groundLineTex);

			//ゲージの描画
			for (int gageIdx = 0; gageIdx < static_cast<int>(m_gageParam[itemIdx].size()); ++gageIdx)
			{
				int patternIdx = m_gageParam[itemIdx][gageIdx];
				Vec2<float>leftUpPos = { GROUND_GAGE_LEFT_X + GAGE_SPACE_X * gageIdx, groundPos.y - GAGE_PATTERN_TEX_HEGIHT[patternIdx] };
				DrawFunc2D::DrawGraph(leftUpPos, m_gagePatternTex[patternIdx]);
			}
		}
	}
}

SystemSetting::OpeMenuGroup::OpeMenuGroup()
{
	using namespace KuroEngine;

	const std::string dir = TEX_DIR + "operationMenu/";
	//見出し
	m_headTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "head.png");
	
	//「カメラミラー」
	m_items[ITEM_MIRROR].LoadTex(dir + "cam_mirror.png");
	//「カメラ感度」
	m_items[ITEM_SENSITIVITY].LoadTex(dir + "cam_sensitivity.png");
	//「戻る」
	m_items[ITEM_BACK].LoadTex(TEX_DIR + "back.png");

	//矢印
	m_arrowTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "arrow.png");
	//カメラのミラーのチェックボックス
	m_camMirrorCheckBoxTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "check_box.png");
	//チェックマーク
	m_checkMarkTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "check_box.png");
	//地面ゲージ
	m_groundLineTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "ground.png");
	//地面ゲージのインデックスの葉
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

	//変化前の項目
	auto oldItem = m_nowItem;

	//項目上へ
	if (0 < m_nowItem && upInput)m_nowItem = (ITEM)(m_nowItem - 1);
	//項目下へ
	if (m_nowItem < ITEM_NUM - 1 && downInput)m_nowItem = (ITEM)(m_nowItem + 1);

	//項目変化あり
	if (oldItem != m_nowItem)SoundConfig::Instance()->Play(SoundConfig::SE_SELECT);

	//決定「戻る」
	if (doneInput && m_nowItem == ITEM_BACK)
	{
		arg_parent->m_mode = MODE_MAIN_MENU;
		SoundConfig::Instance()->Play(SoundConfig::SE_DONE);
	}
	//キャンセル=「戻る」
	if (cancelInput)
	{
		arg_parent->m_mode = MODE_MAIN_MENU;
		SoundConfig::Instance()->Play(SoundConfig::SE_CANCEL);
	}
}

void SystemSetting::OpeMenuGroup::Draw()
{
	using namespace KuroEngine;

	//見出し
	DrawFunc2D::DrawGraph({ 0.0f,32.0f }, m_headTex);

	//項目の描画
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

	//チェックボックス
	DrawFunc2D::DrawRotaGraph2D({ 921.0f,ITEM_CENTER_POS[ITEM_MIRROR].y }, { 1.0f,1.0f }, 0.0f, m_camMirrorCheckBoxTex);

	//地面ゲージ
	DrawFunc2D::DrawGraph({ 504.0f,390.0f }, m_groundLineTex);
}

void SystemSetting::Update()
{
	if (!m_isActive)return;

	auto oldMode = m_mode;

	//更新
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

	//設定メニューモード変化あり
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

	//背景色描画
	DrawFunc2D::DrawBox2D({ 0,0 }, WinApp::Instance()->GetExpandWinSize(),
		Color(221, 236, 212, 255), true, AlphaBlendMode_None);

	//描画
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