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

//���̃`�F�b�N�|�C���g�̉~��
class CheckPointPillar {

private:

	std::shared_ptr<KuroEngine::Model> m_pillarModel;	//�~���̃��f��
	KuroEngine::Transform m_transform;					//�~���̕`����
	bool m_isDraw;										//�`�悷�邩�H

public:

	CheckPointPillar();
	void Init();
	void Update();
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

};