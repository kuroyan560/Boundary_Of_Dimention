#pragma once
#include<vector>
#include<array>
#include<memory>
#include"ForUser/Timer.h"
#include"InGameUI.h"

namespace KuroEngine
{
	class TextureBuffer;
}

class StageInfoUI : public InGameUI
{
	//演出ステータス
	enum STATUS { APPEAR, DRAW, DISAPPEAR, STATUS_NUM }m_status;
	//演出時間計測
	KuroEngine::Timer m_timer;

	//ステージ名画像
	std::vector<std::shared_ptr<KuroEngine::TextureBuffer>>m_stageNameTex;
	std::shared_ptr<KuroEngine::TextureBuffer>m_stageNameDefaultTex;
	//ステージ名の装飾下線画像
	std::shared_ptr<KuroEngine::TextureBuffer>m_underLineTex;
	//ステージ名のインデックス
	int m_stageNameIdx = 0;

	//花の画像（小）
	std::shared_ptr<KuroEngine::TextureBuffer>m_miniFlowerTex;
	//収集した花の数字テクスチャのサイズ
	static const int FLOWER_NUM_TEX_SIZE = 12;
	//「 + 」
	static const int FLOWER_NUM_PLUS_IDX = 11;
	//「 / 」
	static const int FLOWER_NUM_SLASH_IDX = 10;
	//収集した花の数字
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, FLOWER_NUM_TEX_SIZE>m_flowerNumTex;

	//座標オフセットX
	float m_offsetX = 0.0f;
	//アルファ
	float m_alpha = 1.0f;

	//前フレームでの花の所持数
	int m_oldGetFlowerNum;
	//花の収集量増加UI演出のタイマー
	KuroEngine::Timer m_addFlowerNumTimer;
	//花の収集量増加UI演出
	int m_addFlowerNum = 0;
	//花増加UIアルファ
	float m_addFlowerAlpha = 1.0f;
	//花増加UI座標オフセットY
	float m_addFlowerOffsetY = 0.0f;

	//無操作からしばらく出現させておく
	KuroEngine::Timer m_idleTimer;
	//退場の予約（アイドル時間のとき）
	bool m_disappearCall = false;

	void SetUIStatus(STATUS arg_status);

	void Appear()override;
	void Disappear()override;
	bool IsAppeared()override { return m_status == DRAW; }
	bool IsDisappeared()override { return m_status == DISAPPEAR && m_timer.IsTimeUp(); }

public:
	StageInfoUI();
	void Init(int arg_stageNum, int arg_getFlowerNum);
	void Update(float arg_timeScale, int arg_getFlowerNum);
	void Draw(int arg_existFlowerNum, int arg_getFlowerNum);
};

