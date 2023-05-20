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
		m_appearTimer.Reset(60);
		m_hpUiStatus = HP_UI_APPEAR;
	}
	else if (arg_status == HP_UI_DRAW)
	{
		m_appearTimer.Reset(300);
		m_hpUiStatus = HP_UI_DRAW;
	}
	else if (arg_status == HP_UI_DISAPPEAR && m_hpUiStatus == HP_UI_DRAW)
	{
		m_appearTimer.Reset(60);
		m_hpUiStatus = HP_UI_DISAPPEAR;
	}
	else if (arg_status == HP_UI_DAMAGE)
	{
		m_appearTimer.Reset(30);
		m_hpUiStatus = HP_UI_DAMAGE;
		m_hpTexExpand = 1.0f;
		m_hpCenterOffset = { 0,0 };
		m_hpRadiusExpand = 1.0f;
		m_leafSpin = Angle(0);
		m_impactShake.Shake(30.0f, 1.0f, 32.0f, 64.0f);
	}
}

PlayerHpUI::PlayerHpUI()
	:m_impactShake({ 1.0f,1.0f,1.0f })
{
	using namespace KuroEngine;

	std::string dir = "resource/user/tex/in_game/hp/";

	for (int leafIdx = 0; leafIdx < LEAF_NUM; ++leafIdx)
	{
		m_leafTexArray[leafIdx] = D3D12App::Instance()->GenerateTextureBuffer(dir + "leaf_" + std::to_string(leafIdx) + ".png");
	}

	D3D12App::Instance()->GenerateTextureBuffer(
		m_numTexArray.data(), dir + "num_until_" + std::to_string(LEAF_NUM) + ".png", NUM_TEX, { NUM_TEX,1 });

	m_hpStrTex = D3D12App::Instance()->GenerateTextureBuffer(dir + "str.png");
}

void PlayerHpUI::Init()
{
	//HPのUI初期化
	m_impactShake.Init();
	SetHpUIStatus(HP_UI_APPEAR);
	m_beatTimer.Reset(0.0f);
	m_damageFlash = true;
}

void PlayerHpUI::Update(float arg_timeScale, int arg_defaultHp, int arg_nowHp, const KuroEngine::Timer& arg_noDamageTimer)
{
	using namespace KuroEngine;

	const float OFFSET_X_MAX = -300.0f;

	m_appearTimer.UpdateTimer(arg_timeScale);

	if (m_hpUiStatus == HP_UI_APPEAR)
	{
		m_hpRadiusExpand = Math::Ease(Out, Quart, m_appearTimer.GetTimeRate(), 0.1f, 1.0f);
		m_hpTexExpand = Math::Ease(Out, Quart, m_appearTimer.GetTimeRate(0.7f), 0.0f, 1.0f);
		m_leafSpin = Math::Ease(Out, Quart, m_appearTimer.GetTimeRate(), Angle(-360 * 2), 0.0f);
		m_hpCenterOffset = Math::Ease(Out, Exp, m_appearTimer.GetTimeRate(0.8f), { OFFSET_X_MAX,0.0f }, { 0.0f,0.0f });
		m_strAlpha = Math::Lerp(0.0f, 1.0f, m_appearTimer.GetTimeRate());
		m_strOffsetX = Math::Ease(Out, Back, m_appearTimer.GetTimeRate(0.8f), OFFSET_X_MAX, 0.0f);
		if (m_appearTimer.IsTimeUp())
		{
			SetHpUIStatus(HP_UI_DRAW);
		}
	}
	else if (m_hpUiStatus == HP_UI_DAMAGE)
	{
		m_impactShake.Update(arg_timeScale);
		if (m_appearTimer.IsTimeUp())
		{
			SetHpUIStatus(HP_UI_DRAW);
		}
	}
	else if (m_hpUiStatus == HP_UI_DRAW)
	{
		//HPがMAXのときは消える
		if (m_appearTimer.IsTimeUp() && arg_defaultHp <= arg_nowHp)
		{
			SetHpUIStatus(HP_UI_DISAPPEAR);
		}
	}
	else if (m_hpUiStatus == HP_UI_DISAPPEAR)
	{
		m_hpRadiusExpand = Math::Ease(In, Quart, m_appearTimer.GetTimeRate(), 1.0f, 0.1f);
		m_hpTexExpand = Math::Ease(In, Quart, m_appearTimer.GetTimeRate(0.7f), 1.0f, 0.0f);
		m_leafSpin = Math::Ease(In, Quart, m_appearTimer.GetTimeRate(), 0.0f, Angle(-360 * 2));
		m_hpCenterOffset = Math::Ease(In, Quart, m_appearTimer.GetTimeRate(0.8f), { 0.0f,0.0f }, { -300.0f,0.0f });
		m_strAlpha = Math::Lerp(1.0f, 0.0f, m_appearTimer.GetTimeRate());
		m_strOffsetX = Math::Ease(In, Back, m_appearTimer.GetTimeRate(0.8f), 0.0f, OFFSET_X_MAX);
	}

	//心拍演出
	if (m_beatTimer.UpdateTimer(arg_timeScale))
	{
		m_beatTimer.Reset(Math::Ease(InOut, Cubic, static_cast<float>(arg_nowHp - 1) / (arg_defaultHp - 1), 45.0f, 100.0f));
	}

	//無敵時間中のダメージ葉の点滅
	if (m_damageFlashTimer.UpdateTimer(arg_timeScale) || (!m_isNoDamageTime && !arg_noDamageTimer.IsTimeUp()))
	{
		m_damageFlashTimer.Reset(10.0f);

		if ((!m_isNoDamageTime && !arg_noDamageTimer.IsTimeUp()))
		{
			m_damageFlash = true;
		}
		else
		{
			m_damageFlash = !m_damageFlash;
		}
	}
	m_isNoDamageTime = !arg_noDamageTimer.IsTimeUp();
}

void PlayerHpUI::Draw(int arg_defaultHp, int arg_nowHp, bool arg_isHitStop)
{
	using namespace KuroEngine;

	//HPが０なら非表示
	if (arg_nowHp <= 0)return;
	
	//葉全体の中心座標
	static const Vec2<float>LEAF_CENTER_POS = { 160.0f,178.0f };

	//各葉の中心座標
	static const std::array<Vec2<float>,LEAF_NUM>EACH_LEAF_CENTER_POS_ARRAY = 
	{
		Vec2<float>(173.0f,124.0f),
		Vec2<float>(214.0f,157.0f),
		Vec2<float>(177.0f,229.0f),
		Vec2<float>(118.0f,229.0f),
		Vec2<float>(122.0f,167.0f),
	};

	//各葉の外側に向かうベクトル
	static const std::array<Vec2<float>, LEAF_NUM>EACH_LEAF_OUTER_VEC =
	{
		EACH_LEAF_CENTER_POS_ARRAY[0] - LEAF_CENTER_POS,
		EACH_LEAF_CENTER_POS_ARRAY[1] - LEAF_CENTER_POS,
		EACH_LEAF_CENTER_POS_ARRAY[2] - LEAF_CENTER_POS,
		EACH_LEAF_CENTER_POS_ARRAY[3] - LEAF_CENTER_POS,
		EACH_LEAF_CENTER_POS_ARRAY[4] - LEAF_CENTER_POS,
	};

	//各葉の通常時の半径
	static const std::array<float, LEAF_NUM>EACH_LEAF_DEFAULT_RADIUS =
	{
		EACH_LEAF_OUTER_VEC[0].Length(),
		EACH_LEAF_OUTER_VEC[1].Length(),
		EACH_LEAF_OUTER_VEC[2].Length(),
		EACH_LEAF_OUTER_VEC[3].Length(),
		EACH_LEAF_OUTER_VEC[4].Length(),
	};

	//通常時のアルファ
	static const float DEFAULT_ALPHA = 1.0f;
	//非アクティブの葉のアルファ
	static const float LOW_ALPHA = 0.5f;

	//削られたHP葉のスケール
	static const float DAMAGE_LEAF_EXPAND = 0.9f;
	//削られたHP葉のオフセットY
	static const float DAMAGE_LEAF_OFFSET_Y = 6.0f;

	//シェイク
	const auto shake = (!arg_isHitStop ? Vec2<float>(m_impactShake.GetOffset().x, m_impactShake.GetOffset().y) : Vec2<float>(0, 0));
	
	//HPUIの中心座標
	const auto hpCenterPos = LEAF_CENTER_POS + m_hpCenterOffset + shake;

	//HPUI心拍演出の状態
	const auto hpBeat = Math::Ease(Out, Elastic, m_beatTimer.GetTimeRate(0.9f), 0.9f, 1.0f);

	//HPUI画像の拡大率
	const Vec2<float>hpTexExpand = Vec2<float>(1.0f, 1.0f) * m_hpTexExpand * hpBeat;

	//HPの葉描画
	for (int hpIdx = arg_defaultHp - 1; 0 <= hpIdx; --hpIdx)
	{
		auto pos = hpCenterPos;
		auto texExpand = hpTexExpand;

		//ベクトル回転
		auto vec = Math::RotateVec2(EACH_LEAF_OUTER_VEC[hpIdx], m_leafSpin).GetNormal();
		pos += vec * (EACH_LEAF_DEFAULT_RADIUS[hpIdx] * m_hpRadiusExpand * hpBeat);

		float alpha = DEFAULT_ALPHA;
		//削られたHP
		if (arg_nowHp - 1 < hpIdx)
		{
			alpha = LOW_ALPHA;
			texExpand *= DAMAGE_LEAF_EXPAND;
			pos.y += DAMAGE_LEAF_OFFSET_Y;
		}
		//たった今削れたHPは点滅
		if (m_isNoDamageTime && arg_nowHp == hpIdx)
		{
			alpha = m_damageFlash ? LOW_ALPHA : DEFAULT_ALPHA;
		}

		DrawFunc2D::DrawRotaGraph2D(pos, texExpand, m_leafSpin, m_leafTexArray[hpIdx], alpha);
	}

	//「HP」描画
	static const Vec2<float>HP_STR_POS = { 242.0f,202.0f };
	DrawFunc2D::DrawRotaGraph2D(HP_STR_POS + Vec2<float>(m_strOffsetX, 0.0f) + shake, { 1.0f,1.0f }, 0.0f, m_hpStrTex, m_strAlpha);

	//HP数字描画
	static const Vec2<float>HP_NUM_POS = { 238.0f,246.0f };
	DrawFunc2D::DrawRotaGraph2D(HP_NUM_POS + Vec2<float>(m_strOffsetX, 0.0f) + shake, { 1.0f,1.0f }, 0.0f, m_numTexArray[arg_nowHp], m_strAlpha);
}
