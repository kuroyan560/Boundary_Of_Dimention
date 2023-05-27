#pragma once
#include "../../../../src/engine/KuroEngine.h"
#include "../../../../src/engine/ForUser/Object/Model.h"
#include "../../../../src/engine/ForUser/Timer.h"
#include <Render/RenderObject/LightManager.h>
#include <memory>

namespace KuroEngine
{
	class Model;
	class Camera;
	class LightManager;
}

//次のチェックポイントの円柱
class CheckPointPillar {

private:

	std::shared_ptr<KuroEngine::Model> m_pillarModel;	//円柱のモデル
	KuroEngine::Transform m_transform;					//円柱の描画情報
	bool m_isDraw;										//描画するか？
	float m_alpha;										//円柱のアルファ
	bool m_isFirstFrame;								//最初のフレームかどうか。最初のフレームだけ座標を保存する。

	//各ステータスのタイマー
	KuroEngine::Timer m_appearModeTimer;
	const float APPEAR_MODE_TIMER = 20.0f;
	KuroEngine::Timer m_exitModeTimer;
	const float EXIT_MODE_TIMER = 30.0f;

	enum STATUS {
		NORMAL,
		EXIT,
		APPEAR,
	}m_status;

public:

	CheckPointPillar();
	void Init();
	void Update(const KuroEngine::Vec3<float>& arg_playerPos);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, std::weak_ptr<KuroEngine::DepthStencil>arg_ds);

};