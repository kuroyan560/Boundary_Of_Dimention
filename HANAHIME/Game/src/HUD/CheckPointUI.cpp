#include "CheckPointUI.h"
#include"DirectX12/D3D12App.h"
#include"FrameWork/WinApp.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"

CheckPointUI::CheckPointUI()
{
	using namespace KuroEngine;

	std::string dir = "resource/user/tex/in_game/check_point/";
	m_unlockStrTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "open_str.png");
	m_accUnderLineTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "acc_under_line.png");
}

void CheckPointUI::Init()
{
	m_timer.Reset(0.0f);
}

void CheckPointUI::Update()
{
	using namespace KuroEngine;

	//修飾アンダーラインのY座標
	const float ACC_UNDER_LINE_Y = 222.0f;
	//「チェックポイントを解放しました」と下線との隙間オフセット
	const float SPACE_BETWEEN_STR_AND_LINE = 8.0f;
	//「チェックポイントを解放しました」のY座標
	const float STR_Y = ACC_UNDER_LINE_Y - m_accUnderLineTex->GetGraphSize().y * 0.5f - m_unlockStrTex->GetGraphSize().y * 0.5f - SPACE_BETWEEN_STR_AND_LINE;

	//演出時のY座標オフセット最大
	const float STR_OFFSET_Y_MAX = 32.0f;

	//登場時間割合
	const float APPEAR_TIME_RATE = 0.2f;
	//退場時間割合
	const float DISAPPEAR_TIME_RATE = 0.8f;

	//非アクティブ
	if (m_timer.IsTimeUp())return;

	//タイマー更新
	m_timer.UpdateTimer();

	float strOffsetY = 0.0f;

	//登場
	if (m_timer.GetTimeRate() < APPEAR_TIME_RATE)
	{
		float easeRate = m_timer.GetTimeRate(0.0f, APPEAR_TIME_RATE);
		m_alpha = Math::Lerp(0.0f, 1.0f, easeRate);
		strOffsetY = Math::Ease(Out, Quint, easeRate, -STR_OFFSET_Y_MAX, 0.0f);
	}
	//退場
	else if (DISAPPEAR_TIME_RATE < m_timer.GetTimeRate())
	{
		float easeRate = m_timer.GetTimeRate(DISAPPEAR_TIME_RATE, 1.0f);
		m_alpha = Math::Lerp(1.0f, 0.0f, easeRate);
		strOffsetY = Math::Ease(In, Back, easeRate, 0.0f, STR_OFFSET_Y_MAX);
	}

	//ウィンドウのX座標中心
	const auto winCenterX = WinApp::Instance()->GetExpandWinCenter().x;

	//修飾アンダーラインの座標固定
	m_accUnderLinePos = { winCenterX,ACC_UNDER_LINE_Y };
	//「チェックポイントを解放しました」の座標
	m_unlockStrPos = { winCenterX,STR_Y + strOffsetY };
}

void CheckPointUI::Draw()
{
	return;

	using namespace KuroEngine;

	//非アクティブ
	if (m_timer.IsTimeUp())return;

	//修飾アンダーライン
	DrawFunc2D::DrawRotaGraph2D(m_accUnderLinePos, { 1.0f,1.0f }, 0.0f, m_accUnderLineTex, m_alpha);
	//「チェックポイントを解放しました」
	DrawFunc2D::DrawRotaGraph2D(m_unlockStrPos, { 1.0f,1.0f }, 0.0f, m_unlockStrTex, m_alpha);
}
