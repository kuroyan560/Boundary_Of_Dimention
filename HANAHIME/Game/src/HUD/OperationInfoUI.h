#pragma once
#include<array>
#include<memory>
#include"ForUser/Timer.h"
#include"InGameUI.h"

namespace KuroEngine
{
	class TextureBuffer;
}

class OperationInfoUI : public InGameUI
{
	//演出ステータス
	enum STATUS { APPEAR, DRAW, DISAPPEAR, STATUS_NUM }m_status;
	//演出時間計測
	KuroEngine::Timer m_timer;

	//操作表記のベース
	std::shared_ptr<KuroEngine::TextureBuffer>m_opeBaseTex;

	//入力しているかどうか
	enum INPUT_STATUS { ON, OFF, INPUT_STATUS_NUM };
	enum BUTTON { X, LT, RT, BUTTON_NUM, };
	//入力しているときとしていないときの画像各２枚分
	std::array<std::array<std::shared_ptr<KuroEngine::TextureBuffer>, INPUT_STATUS_NUM>, BUTTON_NUM>m_opeButtonTexArray;

	//ボタン表記の座標オフセットX
	float m_opeButtonOffsetX = 0.0f;
	//ボタン表記のアルファ
	float m_opeButtonAlpha = 1.0f;

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
	OperationInfoUI();
	void Init();
	void Update(float arg_timeScale);
	void Draw();
};

