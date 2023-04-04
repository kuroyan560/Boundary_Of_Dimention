#pragma once
#include"Common/Transform.h"
#include"../../../src/engine/Render/RenderObject/SpriteMesh.h"
#include"../../../src/engine/ForUser/Timer.h"

class SceneChange
{
public:
	SceneChange();

	void Update();
	void Draw();

	void Start();
	bool IsHide();

private:
	bool m_startFlag;	//開始フラグ
	bool m_blackOutFlag;//暗転した瞬間

	//暗転と明転の時間関連----------------------------------------
	KuroEngine::Timer m_time;
	int m_countTimeUpNum;
	//暗転と明転の時間関連----------------------------------------

	//テクスチャ関連の情報----------------------------------------
	float m_alpha;
	std::shared_ptr<KuroEngine::TextureBuffer> m_blackTexBuff;
	//テクスチャ関連の情報----------------------------------------

};
