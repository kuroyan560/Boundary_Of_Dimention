#pragma once
#include"Common/Vec.h"
#include"Common/Angle.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"
#include"../Stage/Stage.h"

#include<memory>
namespace KuroEngine
{
	class Camera;
}

class CameraController : public KuroEngine::Debugger
{
	void OnImguiItems()override;

	//対象と注視点の相対的な座標オフセット
	KuroEngine::Vec3<float>m_gazePointOffset = { 0,0.5f,0 };
	//追従対象との相対的な座標オフセットの最小と最大
	float m_posOffsetDepthMin = -10.0f;
	float m_posOffsetDepthMax = -0.1f;
	//X軸角度（高さ傾き）の最小と最大
	KuroEngine::Angle m_xAxisAngleMin = KuroEngine::Angle(10);
	KuroEngine::Angle m_xAxisAngleMax = KuroEngine::Angle(20);

	//カメラのX軸回転
	float m_oldAngleX;
	float m_angleX;
	float m_distanceZ;

	//理想的なカメラのトランスフォーム カメラはこの値に向かって補間する。
	KuroEngine::Transform m_baseTransform;

	//操作するカメラのポインタ
	KuroEngine::Vec3<float> m_oldPos;
	std::weak_ptr<KuroEngine::Camera>m_attachedCam;

	KuroEngine::Transform m_cameraMoveTransform;
	KuroEngine::Transform m_playerTransform;

	//カメラの前方向座標移動のLerp値
	float m_camForwardPosLerpRate = 0.8f;

	//カメラの座標追従のLerp値
	float m_camFollowLerpRate = 0.8f;

	//現在の上下方向で操作しているもの
	enum VERTICAL_MOVE { ANGLE, DIST }m_verticalControl = ANGLE;

public:
	//コンストラクタ
	CameraController();

	void AttachCamera(std::shared_ptr<KuroEngine::Camera>arg_cam);

	void Init();
	void Update(KuroEngine::Vec3<float> arg_scopeMove, KuroEngine::Transform arg_playerTransform, float arg_cameraZ, const std::weak_ptr<Stage> arg_nowStage);

	std::weak_ptr<KuroEngine::Camera> GetCamera() { return m_attachedCam; }

private:

	//カメラを定位置にセット。
	void SetCameraPos();

};