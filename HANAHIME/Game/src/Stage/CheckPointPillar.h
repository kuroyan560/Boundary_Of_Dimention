#pragma once
#include "../../../../src/engine/KuroEngine.h"
#include "../../../../src/engine/ForUser/Object/Model.h"
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

public:

	CheckPointPillar();
	void Init();
	void Update();
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

};