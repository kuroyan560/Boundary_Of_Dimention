#include "OperationInfoUI.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"../OperationConfig.h"

void OperationInfoUI::SetUIStatus(STATUS arg_status)
{
	using namespace KuroEngine;

	//退場演出は通常描画ステータスのときのみ
	if (arg_status == DISAPPEAR && m_status != DRAW)return;

	 static const std::array<float, STATUS_NUM>INTERVALS =
	{
		45.0f,	//登場
		300.0f,	//通常描画
		45.0f,	//退場
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

	std::array<std::string, BUTTON_NUM>BUTTON_FILE_NAME = { "X","LT","RT" };
	for (int buttonIdx = 0; buttonIdx < BUTTON_NUM; ++buttonIdx)
	{
		D3D12App::Instance()->GenerateTextureBuffer(
			m_opeButtonTexArray[buttonIdx].data(),
			dir + BUTTON_FILE_NAME[buttonIdx] + ".png",
			INPUT_STATUS_NUM, 
			{ INPUT_STATUS_NUM,1 });
	}
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
		if (m_timer.IsTimeUp())
		{
			SetUIStatus(DISAPPEAR);
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

	const Vec2<float>opeButtonOffset = { m_opeButtonOffsetX,0.0f };

	//入力方法画像のベース
	static const Vec2<float>OPE_BASE_POS = { 151.0f,504.0f };
	DrawFunc2D::DrawRotaGraph2D(OPE_BASE_POS + opeButtonOffset, { 1.0f,1.0f }, 0.0f, m_opeBaseTex, m_opeButtonAlpha);

	//Xボタン
	static const Vec2<float>X_BUTTON_POS = { 124.0f,428.0f };
	DrawFunc2D::DrawRotaGraph2D(X_BUTTON_POS + opeButtonOffset, { 1.0f,1.0f }, 0.0f,
		m_opeButtonTexArray[X][OperationConfig::Instance()->InputCamDistModeChange(OperationConfig::HOLD) ? OFF : ON], m_opeButtonAlpha);

	//LTボタン
	static const Vec2<float>LT_BUTTON_POS = { 110.0f,528.0f };
	DrawFunc2D::DrawRotaGraph2D(LT_BUTTON_POS + opeButtonOffset, { 1.0f,1.0f }, 0.0f,
		m_opeButtonTexArray[LT][OperationConfig::Instance()->InputCamReset(OperationConfig::HOLD) ? OFF : ON], m_opeButtonAlpha);

	//RTボタン
	static const Vec2<float>RT_BUTTON_POS = { 110.0f,604.0f };
	DrawFunc2D::DrawRotaGraph2D(RT_BUTTON_POS + opeButtonOffset, { 1.0f,1.0f }, 0.0f,
		m_opeButtonTexArray[RT][OperationConfig::Instance()->InputSink(OperationConfig::HOLD) ? OFF : ON], m_opeButtonAlpha);
}
