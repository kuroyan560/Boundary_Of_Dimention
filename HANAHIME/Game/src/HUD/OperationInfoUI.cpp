#include "OperationInfoUI.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"../OperationConfig.h"
#include"../Stage/StageManager.h"

void OperationInfoUI::SetUIStatus(STATUS arg_status)
{
	using namespace KuroEngine;

	 static const std::array<float, STATUS_NUM>INTERVALS =
	{
		35.0f,	//登場
		250.0f,	//通常描画
		35.0f,	//退場
	};

	//演出時間セット
	m_timer.Reset(INTERVALS[arg_status]);
	//ステータス更新
	m_status = arg_status;
}

OperationInfoUI::OperationInfoUI()
{
	using namespace KuroEngine;

	std::string dir = "resource/user/tex/in_game/operation/";
	m_opeBaseTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "base.png");

	//std::array<std::string, BUTTON_NUM>BUTTON_FILE_NAME = { "X","LT","RT" };
	std::array<std::string, BUTTON_NUM>BUTTON_FILE_NAME = { "X","L_stick","RT" };
	for (int buttonIdx = 0; buttonIdx < BUTTON_NUM; ++buttonIdx)
	{
		D3D12App::Instance()->GenerateTextureBuffer(
			m_opeButtonTexArray[buttonIdx].data(),
			dir + BUTTON_FILE_NAME[buttonIdx] + ".png",
			INPUT_STATUS_NUM, 
			{ INPUT_STATUS_NUM,1 });
	}

	//出現後絶対に表示させる時間
	static const float APPEAR_IDLE_INTERVAL = 300.0f;
	m_idleTimer.Reset(APPEAR_IDLE_INTERVAL);
}

void OperationInfoUI::Init()
{
	//UI初期化
	SetUIStatus(APPEAR);
}

void OperationInfoUI::Update(float arg_timeScale)
{
	using namespace KuroEngine;

	static const float OPE_BUTTON_OFFSET_X_MAX = -300.0f;

	m_timer.UpdateTimer(arg_timeScale);

	if (m_status == APPEAR)
	{
		m_opeButtonOffsetX = Math::Ease(Out, Back, m_timer.GetTimeRate(), OPE_BUTTON_OFFSET_X_MAX, 0.0f);
		m_opeButtonAlpha = Math::Lerp(0.0f, 1.0f, m_timer.GetTimeRate());
		if (m_timer.IsTimeUp())
		{
			SetUIStatus(DRAW);
		}
	}
	else if (m_status == DRAW)
	{
		/*if (m_timer.IsTimeUp())
		{
			SetUIStatus(DISAPPEAR);
		}*/
		if (m_idleTimer.UpdateTimer(arg_timeScale) && m_disappearCall)
		{
			//予約された退場の実行
			Disappear();
		}
	}
	else if (m_status == DISAPPEAR)
	{
		m_opeButtonOffsetX = Math::Ease(In, Back, m_timer.GetTimeRate(), 0.0f, OPE_BUTTON_OFFSET_X_MAX);
		m_opeButtonAlpha = Math::Lerp(1.0f, 0.0f, m_timer.GetTimeRate());
	}
}

void OperationInfoUI::Draw()
{
	using namespace KuroEngine;

	//const Vec2<float>opeButtonOffset = { m_opeButtonOffsetX,0.0f };
	const Vec2<float>opeButtonOffset = { -48.0f,40.0f };

	//入力方法画像のベース
	static const Vec2<float>OPE_BASE_POS = { 151.0f,504.0f };
	DrawFunc2D::DrawRotaGraph2D(OPE_BASE_POS + opeButtonOffset, { 1.0f,1.0f }, 0.0f, m_opeBaseTex, m_opeButtonAlpha);

	//Xボタン
	static const Vec2<float>X_BUTTON_POS = { 124.0f,428.0f };
	DrawFunc2D::DrawRotaGraph2D(X_BUTTON_POS + opeButtonOffset, { 1.0f,1.0f }, 0.0f,
		m_opeButtonTexArray[X][OperationConfig::Instance()->GetOperationInput(OperationConfig::CAM_DIST_MODE_CHANGE_LOOK_AROUND, OperationConfig::HOLD) ? OFF : ON], m_opeButtonAlpha);

	//LTボタン
	/*
	static const Vec2<float>LT_BUTTON_POS = { 110.0f,528.0f };
	DrawFunc2D::DrawRotaGraph2D(LT_BUTTON_POS + opeButtonOffset, { 1.0f,1.0f }, 0.0f,
		m_opeButtonTexArray[LT][OperationConfig::Instance()->GetOperationInput(OperationConfig::CAM_RESET, OperationConfig::HOLD) ? OFF : ON], m_opeButtonAlpha);
	*/

	//Lスティック
	const Vec2<float>L_STICK_POS = { 110.0f,528.0f };
	DrawFunc2D::DrawRotaGraph2D(L_STICK_POS + opeButtonOffset, { 1.0f,1.0f }, 0.0f,
		m_opeButtonTexArray[L_STICK][!OperationConfig::Instance()->GetMoveVec(XMQuaternionIdentity()).IsZero() ? OFF : ON], m_opeButtonAlpha);

	//RTボタン
	static const Vec2<float>RT_BUTTON_POS = { 110.0f,604.0f };
	DrawFunc2D::DrawRotaGraph2D(RT_BUTTON_POS + opeButtonOffset, { 1.0f,1.0f }, 0.0f,
		m_opeButtonTexArray[RT][OperationConfig::Instance()->GetOperationInput(OperationConfig::SINK_GROUND, OperationConfig::HOLD) ? OFF : ON], m_opeButtonAlpha);
}

void OperationInfoUI::Appear()
{
	if (m_status != DISAPPEAR)return;
	SetUIStatus(APPEAR);
	m_idleTimer.Reset();
}

void OperationInfoUI::Disappear()
{
	if (m_status != DRAW)return;
	if (StageManager::Instance()->GetNowStageIdx() <= 0)return;

	//出現後の表示アイドル時間が終わっていないなら
	if (!m_idleTimer.IsTimeUp())
	{
		//予約
		m_disappearCall = true;
		return;
	}

	SetUIStatus(DISAPPEAR);
	m_disappearCall = false;
}
