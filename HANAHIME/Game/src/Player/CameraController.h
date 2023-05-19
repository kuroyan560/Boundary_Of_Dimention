#pragma once
#include"Common/Vec.h"
#include"Common/Angle.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"
#include"../Stage/Stage.h"
#include"CollisionDetectionOfRayAndMesh.h"

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
	KuroEngine::Angle m_xAxisAngleMin = KuroEngine::Angle(-40);
	KuroEngine::Angle m_xAxisAngleMax = KuroEngine::Angle(40);
	//カメラの角度 天井にいるときバージョン
	KuroEngine::Angle m_xAxisAngleMinCeiling = KuroEngine::Angle(-40);
	KuroEngine::Angle m_xAxisAngleMaxCeiling = KuroEngine::Angle(40);

	//カメラをZ方向に回転させる量。
	float m_rotateZ;

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

	//地形に当たっているか
	bool m_isHitTerrian;

	//下側の地形に当たっているか。当たっていたら注視点をずらすやつをやる。
	bool m_isHitUnderGroundTerrian;

	//プレイヤーのY軸回転を保存しておく変数。プレイヤーが横の壁に居るときは注視点の移動をY軸回転で行うので、注視点移動が終わったら動かした量を戻すため。
	float m_playerRotYStorage;
	float m_playerRotYLerp;
	const float PLAYER_TARGET_MOVE_SIDE = 0.8f;		//プレイヤーの横面の注視点移動のときの動かせる限界。


	float m_rotateYLerpAmount;
	float m_cameraXAngleLerpAmount;

	//操作するカメラのポインタ
	std::weak_ptr<KuroEngine::Camera>m_attachedCam;

	KuroEngine::Vec3<float> m_oldCameraWorldPos;
	KuroEngine::Transform m_cameraLocalTransform;	//カメラのローカルでの回転と移動を計算する用。
	KuroEngine::Transform m_camParentTransform;		//プレイヤーの座標と回転を適応させる用。

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
	void Update(KuroEngine::Vec3<float>arg_scopeMove, KuroEngine::Transform arg_targetPos, float& arg_playerRotY, float arg_cameraZ, const std::weak_ptr<Stage>arg_nowStage, bool arg_isCameraUpInverse, bool arg_isCameraDefaultPos, bool& arg_isHitUnderGround, bool arg_isMovePlayer, bool arg_isPlayerJump);

	const KuroEngine::Quaternion& GetPosRotate() {
		return m_camParentTransform.GetRotate();
	}

	std::weak_ptr<KuroEngine::Camera> GetCamera() { return m_attachedCam; }

private:

	//3次元ベクトルを2次元に射影する関数
	inline KuroEngine::Vec2<float> Project3Dto2D(KuroEngine::Vec3<float> arg_vector3D, KuroEngine::Vec3<float> arg_basis1, KuroEngine::Vec3<float> arg_basis2) {
		
		//基底ベクトルを正規化
		arg_basis1.Normalize();
		arg_basis2.Normalize();

		//3次元ベクトルを2次元ベクトルに射影
		float x = arg_vector3D.Dot(arg_basis1);
		float y = arg_vector3D.Dot(arg_basis2);

		return KuroEngine::Vec2<float>(x, y);
	}

	//下方向の押し戻し処理
	void PushBackGround(const CollisionDetectionOfRayAndMesh::MeshCollisionOutput& arg_output, const KuroEngine::Vec3<float> arg_pushBackPos, const KuroEngine::Transform& arg_targetPos, float& arg_playerRotY, bool arg_isCameraUpInverse);

};