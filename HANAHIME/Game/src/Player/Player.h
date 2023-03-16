#pragma once
#include"KuroEngine.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"
#include"CameraController.h"

#include<memory>
namespace KuroEngine
{
	class Camera;
	class Model;
}

class Player : public KuroEngine::Debugger
{
	//プレイヤーのモデル
	std::shared_ptr<KuroEngine::Model>m_model;

	//カメラのモデル（デバッグ用）
	std::shared_ptr<KuroEngine::Model>m_camModel;

	//トランスフォーム
	KuroEngine::Transform m_transform;

	//カメラインスタンス
	std::shared_ptr<KuroEngine::Camera>m_cam;

	//カメラのコントローラー
	CameraController m_camController;
	
	//カメラ感度
	float m_camSensitivity = 1.0f;

	//Imguiデバッグ関数オーバーライド
	void OnImguiItems()override;

public:
	Player();
	void Init(KuroEngine::Transform arg_initTransform);
	void Update();
	void Draw(KuroEngine::Camera& arg_cam, bool arg_cameraDraw = false);
	void Finalize();

	std::weak_ptr<KuroEngine::Camera>GetCamera() { return m_cam; }

	//カメラコントローラーのデバッガポインタ取得
	KuroEngine::Debugger* GetCameraControllerDebugger() { return &m_camController; }
};

