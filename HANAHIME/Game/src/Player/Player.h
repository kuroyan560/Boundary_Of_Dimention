#pragma once
#include"KuroEngine.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"

#include<memory>
namespace KuroEngine
{
	class Camera;
	class Model;
}

class Player : public KuroEngine::Debugger
{
	//モデル
	std::shared_ptr<KuroEngine::Model>m_model;

	//トランスフォーム
	KuroEngine::Transform m_transform;

	//カメラインスタンス
	std::shared_ptr<KuroEngine::Camera>m_cam;
	
	//カメラ位置オフセット（デフォルト値）
	KuroEngine::Vec3<float>m_camPosOffsetDefault = { 0.0f,9.0f,-11.0f };
	//カメラ位置オフセット
	KuroEngine::Vec3<float>m_camPosOffset = m_camPosOffsetDefault;

	//カメラ感度
	float m_camSensitivity = 1.0f;

	//Imguiデバッグ関数オーバーライド
	void OnImguiItems()override;

public:
	Player();
	void Init(KuroEngine::Transform arg_initTransform);
	void Update();
	void Draw(KuroEngine::Camera& arg_cam);
	void Finalize();

	std::weak_ptr<KuroEngine::Camera>GetCamera() { return m_cam; }
};

