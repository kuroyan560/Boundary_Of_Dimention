#pragma once
#include"Common/Transform.h"
#include"Render/RenderObject/SpriteMesh.h"
#include"ForUser/Timer.h"

/// <summary>
/// 扉移動時の遷移演出
/// </summary>
class SceneChange
{
public:
	SceneChange();

	void Update();
	void Draw();

	void Start();
	bool IsHide();
	bool IsAppear();

	const bool& IsActive()const { return m_startFlag; }

private:
	bool m_startFlag;	//開始フラグ
	bool m_blackOutFlag;//暗転した瞬間
	bool m_appearFlag;	//明転

	//暗転と明転の時間関連----------------------------------------
	KuroEngine::Timer m_time;
	int m_countTimeUpNum;
	const float SCENE_CHANGE_TIME;
	//暗転と明転の時間関連----------------------------------------

	//テクスチャ関連の情報----------------------------------------
	float m_alpha, m_alphaOffset;
	KuroEngine::Vec2<float>m_size;
	std::shared_ptr<KuroEngine::TextureBuffer> m_blackTexBuff;
	//テクスチャ関連の情報----------------------------------------

};
