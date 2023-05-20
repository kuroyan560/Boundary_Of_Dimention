#include "PlayerHpUI.h"
#include"KuroEngine.h"
#include"DirectX12/D3D12App.h"
#include"FrameWork/WinApp.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"

void PlayerHpUI::SetHpUIStatus(HP_UI_STATUS arg_status)
{
	using namespace KuroEngine;

	if (arg_status == HP_UI_APPEAR)
	{
		m_hpUiTimer.Reset(60);
		m_hpUiStatus = HP_UI_APPEAR;
	}
	else if (arg_status == HP_UI_DRAW)
	{
		m_hpUiTimer.Reset(300);
		m_hpUiStatus = HP_UI_DRAW;
	}
	else if (arg_status == HP_UI_DISAPPEAR && m_hpUiStatus == HP_UI_DRAW)
	{
		m_hpUiTimer.Reset(60);
		m_hpUiStatus = HP_UI_DISAPPEAR;
	}
	else if (arg_status == HP_UI_DAMAGE)
	{
		m_hpUiTimer.Reset(30);
		m_hpUiStatus = HP_UI_DAMAGE;
		m_hpTexExpand = 1.0f;
		m_hpCenterOffset = { 0,0 };
		m_hpRadiusExpand = 1.0f;
		m_hpAngle = Angle(0);
		m_hpUiShake.Shake(30.0f, 1.0f, 32.0f, 64.0f);
	}
}

PlayerHpUI::PlayerHpUI()
	:m_hpUiShake({ 1.0f,1.0f,1.0f })
{
	using namespace KuroEngine;
	m_hpTex = D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/in_game/hp_leaf.png");
	m_hpDamageTex = D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/in_game/hp_leaf_damage.png");
}

void PlayerHpUI::Init()
{
	//HPのUI初期化
	m_hpUiShake.Init();
	SetHpUIStatus(HP_UI_APPEAR);
	m_hpUiBeatTimer.Reset(0.0f);
}

void PlayerHpUI::Update(float arg_timeScale, int arg_defaultHp, int arg_nowHp)
{
	using namespace KuroEngine;

	m_hpUiTimer.UpdateTimer(arg_timeScale);

	if (m_hpUiStatus == HP_UI_APPEAR)
	{
		m_hpRadiusExpand = Math::Ease(Out, Quart, m_hpUiTimer.GetTimeRate(), 0.5f, 1.0f);
		m_hpTexExpand = Math::Ease(Out, Quart, m_hpUiTimer.GetTimeRate(0.7f), 0.0f, 1.0f);
		m_hpAngle = Math::Ease(Out, Quart, m_hpUiTimer.GetTimeRate(), Angle(-360 * 2), 0.0f);
		m_hpCenterOffset = Math::Ease(Out, Exp, m_hpUiTimer.GetTimeRate(0.8f), { -300.0f,0.0f }, { 0.0f,0.0f });
		if (m_hpUiTimer.IsTimeUp())
		{
			SetHpUIStatus(HP_UI_DRAW);
		}
	}
	else if (m_hpUiStatus == HP_UI_DAMAGE)
	{
		m_hpUiShake.Update(arg_timeScale);
		if (m_hpUiTimer.IsTimeUp())
		{
			SetHpUIStatus(HP_UI_DRAW);
		}
	}
	else if (m_hpUiStatus == HP_UI_DRAW)
	{
		//HPがMAXのときは消える
		if (m_hpUiTimer.IsTimeUp() && arg_defaultHp <= arg_nowHp)
		{
			SetHpUIStatus(HP_UI_DISAPPEAR);
		}
	}
	else if (m_hpUiStatus == HP_UI_DISAPPEAR)
	{
		m_hpRadiusExpand = Math::Ease(In, Quart, m_hpUiTimer.GetTimeRate(), 1.0f, 0.5f);
		m_hpTexExpand = Math::Ease(In, Quart, m_hpUiTimer.GetTimeRate(0.7f), 1.0f, 0.0f);
		m_hpAngle = Math::Ease(In, Quart, m_hpUiTimer.GetTimeRate(), 0.0f, Angle(-360 * 2));
		m_hpCenterOffset = Math::Ease(In, Quart, m_hpUiTimer.GetTimeRate(0.8f), { 0.0f,0.0f }, { -300.0f,0.0f });
	}

	//心拍演出
	if (m_hpUiBeatTimer.UpdateTimer(arg_timeScale))
	{
		m_hpUiBeatTimer.Reset(Math::Ease(InOut, Cubic, static_cast<float>(arg_nowHp - 1) / (arg_defaultHp - 1), 45.0f, 100.0f));
	}
}

void PlayerHpUI::Draw(int arg_defaultHp, int arg_nowHp, bool arg_isHitStop, const KuroEngine::Timer& arg_noDamageTimer)
{
	using namespace KuroEngine;
	
	//最大HPから配置の角度オフセットを求める
	const Angle angleOffset = Angle::ROUND() / arg_defaultHp;

	//ウィンドウのサイズ取得
	const auto winSize = WinApp::Instance()->GetExpandWinSize();

	//HPUIの中心座標
	const auto hpCenterPos = m_hpCenterPos + m_hpCenterOffset + (!arg_isHitStop ? Vec2<float>(m_hpUiShake.GetOffset().x, m_hpUiShake.GetOffset().y) : Vec2<float>(0, 0));

	//HPUI心拍演出の状態
	const auto hpBeat = Math::Ease(Out, Elastic, m_hpUiBeatTimer.GetTimeRate(0.9f), 0.9f, 1.0f);

	//HPUI画像の拡大率
	const Vec2<float>hpTexExpand = Vec2<float>(1.2f, 1.2f) * m_hpTexExpand * hpBeat;

	//HPUI円の半径
	const auto hpRadius = m_hpTex->GetGraphSize().y * 0.5f * hpTexExpand.y * m_hpRadiusExpand * hpBeat;

	//プレイヤーの２D座標
	//HPのUI描画
	for (int hpIdx = arg_nowHp - 1; 0 <= hpIdx; --hpIdx)
	{
		auto pos = hpCenterPos;
		Angle angle = angleOffset * hpIdx - Angle::ConvertToRadian(90) + m_hpAngle;
		pos.x += cos(angle) * hpRadius;
		pos.y += sin(angle) * hpRadius;
		DrawFunc2D::DrawRotaGraph2D(pos, hpTexExpand, angle + Angle(90), m_hpTex);
	}

	//ダメージで減ったHP
	if (!arg_noDamageTimer.IsTimeUp())
	{
		int hpIdx = arg_nowHp;
		auto pos = hpCenterPos;
		Angle angle = angleOffset * hpIdx - Angle::ConvertToRadian(90) + m_hpAngle;
		auto damageHpRadius = hpRadius * Math::Lerp(1.0f, 0.8f, arg_noDamageTimer.GetTimeRate());
		pos.x += cos(angle) * damageHpRadius;
		pos.y += sin(angle) * damageHpRadius;
		DrawFunc2D::DrawRotaGraph2D(pos,
			hpTexExpand * Math::Ease(Out, Circ, arg_noDamageTimer.GetTimeRate(0.8f), 1.0f, 0.8f),
			angle + Angle(90),
			m_hpDamageTex,
			Math::Ease(In, Circ, arg_noDamageTimer.GetTimeRate(0.7f), 1.0f, 0.0f));
	}
}
