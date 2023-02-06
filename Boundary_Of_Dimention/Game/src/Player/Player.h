#pragma once
#include"KuroEngine.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"

#include<memory>
namespace KuroEngine
{
	class Camera;
}

class Player : public KuroEngine::Debugger
{
	//トランスフォーム
	KuroEngine::Transform m_transform;
	//一人称カメラ
	std::shared_ptr<KuroEngine::Camera>m_cam;

	void OnImguiItems()override;

public:
	Player();
	void Init(KuroEngine::Transform arg_initTransform);
	void Update();
	void Draw();
	void Finalize();

	std::weak_ptr<KuroEngine::Camera>GetCamera() { return m_cam; }
};

