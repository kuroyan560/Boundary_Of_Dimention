#pragma once
#include<array>
#include<memory>
#include"Common/Vec.h"
#include"Common/Angle.h"
#include"ForUser/Timer.h"
#include"ForUser/ImpactShake.h"

namespace KuroEngine
{
	class TextureBuffer;
}

class PlayerHpUI
{
	//HPUIの演出ステータス
	enum HP_UI_STATUS { HP_UI_APPEAR, HP_UI_DRAW, HP_UI_DISAPPEAR, HP_UI_DAMAGE }m_hpUiStatus;

	//HPリーフの数
	static const int LEAF_NUM = 5;
	//葉っぱテクスチャ
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, LEAF_NUM>m_leafTexArray;
	//数字テクスチャ
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, LEAF_NUM>m_numTexArray;
	//「HP」テクスチャ
	std::shared_ptr<KuroEngine::TextureBuffer>m_hpStrTex;

	//HPUIの中心座標オフセット
	KuroEngine::Vec2<float>m_hpCenterOffset;
	//HPUIの半径拡大率
	float m_hpRadiusExpand = 1.0f;
	//HPUIの画像の拡大率
	float m_hpTexExpand = 1.0f;
	//HPUIの回転角度
	KuroEngine::Angle m_hpAngle = KuroEngine::Angle(0);
	//HPUIの登場演出タイマー
	KuroEngine::Timer m_hpUiTimer;
	//HPUIの振動
	KuroEngine::ImpactShake m_hpUiShake;
	//HPUIの心拍演出タイマー
	KuroEngine::Timer m_hpUiBeatTimer;

	//ダメージ時のフラッシュ
	bool m_isDamageFlash;
	KuroEngine::Timer m_damageFlashTimer;
	bool m_damageFlash;

	void SetHpUIStatus(HP_UI_STATUS arg_status);

public:
	PlayerHpUI();
	void Init();
	void Update(float arg_timeScale, int arg_defaultHp, int arg_nowHp, const KuroEngine::Timer& arg_noDamageTimer);
	void Draw(int arg_defaultHp, int arg_nowHp, bool arg_isHitStop);

	void OnDamage()
	{
		SetHpUIStatus(HP_UI_DAMAGE);
	}
};

