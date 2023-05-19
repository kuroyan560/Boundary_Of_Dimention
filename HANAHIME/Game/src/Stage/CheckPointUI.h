#pragma once
#include"ForUser/Timer.h"
#include"Common/Vec.h"
#include<memory>
namespace KuroEngine
{
	class TextureBuffer;
};

class CheckPointUI
{
	//「チェックポイントを解放しました」画像
	std::shared_ptr<KuroEngine::TextureBuffer>m_unlockStrTex;
	//「チェックポイントを解放しました」描画座標
	KuroEngine::Vec2<float>m_unlockStrPos;

	//修飾アンダーライン画像
	std::shared_ptr<KuroEngine::TextureBuffer>m_accUnderLineTex;
	//修飾アンダーライン描画座標
	KuroEngine::Vec2<float>m_accUnderLinePos;

	//演出のトータル時間
	static const int STAGING_TOTAL_TIME = 120;

	//演出タイマー
	KuroEngine::Timer m_timer;

	//描画アルファ値
	float m_alpha = 1.0f;

public:
	CheckPointUI();
	void Init();
	void Update();
	void Draw();

	void Start() { m_timer.Reset(STAGING_TOTAL_TIME); }
};

