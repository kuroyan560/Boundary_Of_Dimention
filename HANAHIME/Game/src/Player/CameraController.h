#pragma once
#include"Common/Vec.h"
#include"Common/Angle.h"
#include"ForUser/Debugger.h"

#include<memory>
namespace KuroEngine
{
	class Camera;
}

class CameraController : public KuroEngine::Debugger
{
	void OnImguiItems()override;

public:
	struct Parameter
	{
		//追従する対象からカメラまでの距離
		float m_distToTarget = 4.0f;
		//注視点の高さ
		float m_gazePointHeight = 0.0f;
		//注視点からカメラまでの距離
		float m_gazePointDist = 10.0f;

		//X軸角度（高さ傾き）
		KuroEngine::Angle m_xAngle;
	};
	//パラメータの初期化値（デフォルト値）
	Parameter m_initializedParam;
	//現在のパラメータ
	Parameter m_nowParam;

	//カメラ座標移動のLerp値
	float m_camPosLerpRate = 0.8f;
	//カメラ注視点移動のLerp値
	float m_camGazePointLerpRate = 0.8f;

	//コンストラクタ
	CameraController();

	void Init();
	void Update(std::shared_ptr<KuroEngine::Camera>arg_cam, KuroEngine::Vec3<float>arg_targetPos, KuroEngine::Vec3<float>arg_frontVec);
};