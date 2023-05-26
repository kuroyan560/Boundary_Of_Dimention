#include "CameraModeUI.h"
#include"DirectX12/D3D12App.h"
#include"KuroEngine.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"

CameraModeUI::CameraModeUI()
{
	m_icon = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/in_game/operation/camera_mode.png");
}

void CameraModeUI::Update(float arg_timeRate)
{
	using namespace KuroEngine;

	static const Vec2<float>DEFAULT_POS = { 1112.0f,595.0f };
	static const Vec2<float>DISAPPEAR_OFFSET = { 248.0f,0.0f };
	static const float INTERVAL = 360.0f;

	if (!m_isActive)return;

	m_timer.UpdateTimer(arg_timeRate);
	m_lissajousAngle += Angle::ROUND() / INTERVAL;

	if (m_isAppear)
	{
		//èoåª
		m_pos = Math::Ease(Out, Back, m_timer.GetTimeRate(), DEFAULT_POS + DISAPPEAR_OFFSET, DEFAULT_POS);
		m_alpha = Math::Lerp(0.0f, 1.0f, m_timer.GetTimeRate(0.2f, 1.0f));
	}
	else
	{
		m_pos = Math::Ease(In, Back, m_timer.GetTimeRate(), DEFAULT_POS, DEFAULT_POS + DISAPPEAR_OFFSET);
		m_alpha = Math::Lerp(1.0f, 0.0f, m_timer.GetTimeRate(0.2f, 1.0f));
		//ëﬁèÍ
		if (m_timer.IsTimeUp())m_isActive = false;
	}
}

void CameraModeUI::Draw()
{
	using namespace KuroEngine;

	static const float LISSAJOUS_OFFSET_MAX = 4.0f;

	Vec2<float>lissajousOffset = { static_cast<float>(sin(2.0f * m_lissajousAngle)),static_cast<float>(sin(3.0f * m_lissajousAngle)) };
	DrawFunc2D::DrawRotaGraph2D(m_pos + lissajousOffset * LISSAJOUS_OFFSET_MAX, { 1.0f,1.0f }, 0.0f, m_icon, m_alpha);
}
