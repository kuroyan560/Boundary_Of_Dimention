#pragma once
#include"Common/Vec.h"
#include"Common/Angle.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"

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

	struct Parameter
	{
		//対象との相対的なZオフセット
		float m_posOffsetZ = -10.0f;
		//X軸角度（高さ傾き）
		KuroEngine::Angle m_xAxisAngle = KuroEngine::Angle(20);

		//Y軸角度
		KuroEngine::Angle m_yAxisAngle = KuroEngine::Angle(0);
	};
	//パラメータの初期化値（デフォルト値）
	Parameter m_initializedParam;
	//現在のパラメータ
	Parameter m_nowParam;

	//操作するカメラのポインタ
	std::weak_ptr<KuroEngine::Camera>m_attachedCam;

	//プレイヤーのトランスフォームを補間しながらコピー
	KuroEngine::Transform m_copyPlayerTransform;
	//プレイヤー座標コピーのlerp率
	float m_playerPosLerpRate = 0.2f;
	//プレイヤークォータニオンコピーのlerp率
	float m_playerQuaternionLerpRate = 0.2f;

	//カメラコントローラーのトランスフォーム
	KuroEngine::Transform m_controllerTransform;

	//現在の上下方向で操作しているもの
	enum VERTICAL_MOVE { ANGLE, DIST }m_verticalControl = ANGLE;

public:
	//コンストラクタ
	CameraController();

	void AttachCamera(std::shared_ptr<KuroEngine::Camera>arg_cam);

	void Init(const KuroEngine::Vec3<float>& arg_playerPos, const KuroEngine::Quaternion& arg_playerRotate);
	void Update(KuroEngine::Vec3<float>arg_scopeMove, const KuroEngine::Vec3<float>& arg_playerPos, const KuroEngine::Quaternion& arg_playerRotate, float arg_cameraY, bool arg_isNoLerp);

	const KuroEngine::Quaternion& GetPosRotate() {
		return m_controllerTransform.GetRotate();
	}
};