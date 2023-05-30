#pragma once
#include"Common/Vec.h"
#include"Common/Angle.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"
#include"../Stage/Stage.h"
#include"CollisionDetectionOfRayAndMesh.h"
#include<vector>

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

	//プレイヤーの正面に壁があるフラグ
	bool m_isOldFrontWall;
	KuroEngine::Vec3<float> m_oldHitFrontWallNormal;

	//カメラをZ方向に回転させる量。
	float m_rotateZ;

	//チェックポイントに達した時のカメラZ軸回転。
	float m_checkPointCameraZ;

	//プレイヤーの座標。
	KuroEngine::Vec3<float> m_playerLerpPos;

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
	Parameter m_checkPointTriggerParam;	//チェックポイントに到達した瞬間のパラメーター

	//地形に当たっているか
	bool m_isHitTerrian;

	//カメラモードがLookAroundになったときのフラグ検出用。
	bool m_isCameraModeLookAround;
	bool m_isLookAroundFinish;			//カメラモードの終了作業を開始する
	bool m_isLookAroundFinishComplete;	//カメラモードの終了作業が終わったかどうか。
	float m_lookAroundModeFar;		//LookAround状態の時の距離
	float m_lookAroundInitFar;
	float m_lookAroundFar;
	const float MIN_LOOK_AROUND_FAR = 20.0f;
	const float MAX_LOOK_AROUND_FAR = 550.0f;
	const float ADD_LOOK_AROUND_FAR = 5.0f;

	//カメラが地形に当たったらカメラのZを下げるために使うやつ。
	float m_cameraHitTerrianZ;
	KuroEngine::Timer m_cameraHitTerrianZTimer;
	const float CAMERA_HIT_TERRIAN_Z_TIMER = 5.0f;
	const float ADD_CAMERA_HIT_TERRIAN_Z = -60.0f;

	//LookAround状態の時に使用する変数。
	KuroEngine::Transform m_lookAroundTransform;
	KuroEngine::Transform m_lookAroundInitTransform;
	KuroEngine::Timer m_lookAroundFinishTimer;
	const float LOOKAROUND_FINISH_TIMER = 60.0f;

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

	KuroEngine::Vec3<float> m_playerOldPos;

	//カメラの前方向座標移動のLerp値
	float m_camForwardPosLerpRate = 0.8f;

	//カメラの座標追従のLerp値
	float m_camFollowLerpRate = 0.8f;

	//現在の上下方向で操作しているもの
	enum VERTICAL_MOVE { ANGLE, DIST }m_verticalControl = ANGLE;

	KuroEngine::Transform m_debugTransform;


public:

	struct HIT_POINT {
		KuroEngine::Vec3<float> m_pos;
		KuroEngine::Vec3<float> m_up;
	};

	enum class CAMERA_STATUS {
		NORMAL,
		LOOK_AROUND
	};

	//C字の対策で作ったカメラモード
	enum class CAMERA_FUNA_MODE {
		NORMAL,					//通常状態
		STAGE1_1_C,				//C字の更新処理
		STAGE1_2_INV_U,			//逆U字の更新処理
		STAGE1_3_HEIGHT_LIMIT,	//高さに制限をかける場所。
	};

	//コンストラクタ
	CameraController();

	void AttachCamera(std::shared_ptr<KuroEngine::Camera>arg_cam);

	void Init(bool arg_isRespawn = false);
	void CameraController::Update(KuroEngine::Vec3<float>arg_scopeMove, KuroEngine::Transform arg_targetPos, float& arg_playerRotY, float arg_cameraZ, const std::weak_ptr<Stage>arg_nowStage, bool arg_isCameraUpInverse, bool arg_isCameraDefaultPos, bool& arg_isHitUnderGround, bool arg_isMovePlayer, bool arg_isPlayerJump, KuroEngine::Quaternion arg_cameraQ, bool arg_isFrontWall, KuroEngine::Transform arg_drawTransform, KuroEngine::Vec3<float> arg_frontWallNormal, bool arg_isNoCollision, CAMERA_STATUS arg_cameraMode, CAMERA_FUNA_MODE arg_cameraFunaMode);

	//リスポーン時のカメラ挙動
	void Respawn(KuroEngine::Transform arg_playerTransform, bool arg_isCameraUpInverse);

	//ジャンプを開始した瞬間、X軸回転とY軸回転を本来あるべき値に近づける。
	void JumpStart(const KuroEngine::Transform& arg_playerTransform, const KuroEngine::Vec3<float>& arg_jumpEndNormal, bool arg_isCameraUpInverse, float arg_scale = 1.0f);

	const KuroEngine::Quaternion& GetPosRotate() {
		return m_camParentTransform.GetRotate();
	}

	//ラープを強制的に終わらせる。
	void LerpForcedToEnd(float& arg_playerRotY);

	//FUNA 通常状態の更新処理
	void UpdateFunaNormal(KuroEngine::Vec3<float>arg_scopeMove, KuroEngine::Transform arg_targetPos, float& arg_playerRotY, float arg_cameraZ, const std::weak_ptr<Stage>arg_nowStage, bool arg_isCameraUpInverse, bool arg_isCameraDefaultPos, bool& arg_isHitUnderGround, bool arg_isMovePlayer, bool arg_isPlayerJump, KuroEngine::Quaternion arg_cameraQ, bool arg_isFrontWall, KuroEngine::Transform arg_drawTransform, KuroEngine::Vec3<float> arg_frontWallNormal, bool arg_isNoCollision, CAMERA_STATUS arg_cameraMode);
	void UpdateFunaStage1_1C(KuroEngine::Vec3<float>arg_scopeMove, KuroEngine::Transform arg_targetPos, float& arg_playerRotY, float arg_cameraZ, const std::weak_ptr<Stage>arg_nowStage, bool arg_isCameraUpInverse, bool arg_isCameraDefaultPos, bool& arg_isHitUnderGround, bool arg_isMovePlayer, bool arg_isPlayerJump, KuroEngine::Quaternion arg_cameraQ, bool arg_isFrontWall, KuroEngine::Transform arg_drawTransform, KuroEngine::Vec3<float> arg_frontWallNormal, bool arg_isNoCollision, CAMERA_STATUS arg_cameraMode);
	void UpdateFunaStage1_2InvU(KuroEngine::Vec3<float>arg_scopeMove, KuroEngine::Transform arg_targetPos, float& arg_playerRotY, float arg_cameraZ, const std::weak_ptr<Stage>arg_nowStage, bool arg_isCameraUpInverse, bool arg_isCameraDefaultPos, bool& arg_isHitUnderGround, bool arg_isMovePlayer, bool arg_isPlayerJump, KuroEngine::Quaternion arg_cameraQ, bool arg_isFrontWall, KuroEngine::Transform arg_drawTransform, KuroEngine::Vec3<float> arg_frontWallNormal, bool arg_isNoCollision, CAMERA_STATUS arg_cameraMode);
	void UpdateFunaStage1_3HeightLimit(KuroEngine::Vec3<float>arg_scopeMove, KuroEngine::Transform arg_targetPos, float& arg_playerRotY, float arg_cameraZ, const std::weak_ptr<Stage>arg_nowStage, bool arg_isCameraUpInverse, bool arg_isCameraDefaultPos, bool& arg_isHitUnderGround, bool arg_isMovePlayer, bool arg_isPlayerJump, KuroEngine::Quaternion arg_cameraQ, bool arg_isFrontWall, KuroEngine::Transform arg_drawTransform, KuroEngine::Vec3<float> arg_frontWallNormal, bool arg_isNoCollision, CAMERA_STATUS arg_cameraMode);

	//カメラの位置を計算。
	void CalCameraPosAndRotate(KuroEngine::Transform arg_targetPos, bool arg_isCameraUpInverse, KuroEngine::Vec3<float> arg_moveAmount = KuroEngine::Vec3<float>(), KuroEngine::Vec3<float>* arg_insertPos = nullptr);

	std::weak_ptr<KuroEngine::Camera> GetCamera() { return m_attachedCam; }


	KuroEngine::Transform GetDebugTransform() { return m_debugTransform; }

	void EndLookAround() { m_isLookAroundFinish = true; }
	bool IsFinishLookAround() { return m_isLookAroundFinish; }
	bool IsCompleteFinishLookAround() { return m_isLookAroundFinishComplete; }

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

	//ベクトルを指定してクォータニオンを返す。ベクトルが一致している場合は単位クォータニオンを返す。
	inline KuroEngine::Quaternion CalQuaternionVector3(KuroEngine::Vec3<float> arg_vecA, KuroEngine::Vec3<float> arg_vecB) {

		//外積から回転軸を取得。
		KuroEngine::Vec3<float> axis = arg_vecA.Cross(arg_vecB);

		//回転軸が存在しなかったら単位クォータニオンを返す。
		if (axis.Length() <= 0.0f) return DirectX::XMQuaternionIdentity();

		//回転量を計算。
		float rad = acos(arg_vecA.Dot(arg_vecB));

		//クォータニオンを計算して返す。
		return DirectX::XMQuaternionRotationAxis(axis, rad);

	}

	//下方向の押し戻し処理
	void PushBackGround(const CollisionDetectionOfRayAndMesh::MeshCollisionOutput& arg_output, const KuroEngine::Vec3<float> arg_pushBackPos, const KuroEngine::Transform& arg_targetPos, float& arg_playerRotY, bool arg_isCameraUpInverse, bool arg_isAroundRay);

	//プレイヤーの動きによってカメラの回転を制御する。
	void PlayerMoveCameraLerp(KuroEngine::Vec3<float>arg_scopeMove, KuroEngine::Transform arg_targetPos, float& arg_playerRotY, float arg_cameraZ, const std::weak_ptr<Stage>arg_nowStage, bool arg_isCameraUpInverse, bool arg_isCameraDefaultPos, bool& arg_isHitUnderGround, bool arg_isMovePlayer, bool arg_isPlayerJump, KuroEngine::Quaternion arg_cameraQ);

	//レイと平面(無限)の当たり判定
	bool RayPlaneIntersection(const KuroEngine::Vec3<float>& arg_rayOrigin, const KuroEngine::Vec3<float>& arg_rayDirection, const KuroEngine::Vec3<float>& arg_planePoint, const KuroEngine::Vec3<float>& arg_planeNormal, KuroEngine::Vec3<float>& arg_hitResult);

	//ベクトルAをベクトルBに射影
	inline KuroEngine::Vec3<float> Project(const KuroEngine::Vec3<float>& arg_A, const KuroEngine::Vec3<float>& arg_B) {
		float dotProduct = arg_A.Dot(arg_B);
		float bLengthSquared = arg_B.Length() * arg_B.Length();
		return arg_B * (dotProduct / bLengthSquared);
	}
	
	//LookAround状態の更新処理。
	void UpdateLookAround(KuroEngine::Vec3<float>arg_scopeMove, KuroEngine::Transform arg_targetPos, float& arg_playerRotY, float arg_cameraZ, const std::weak_ptr<Stage>arg_nowStage, bool arg_isCameraUpInverse, bool arg_isCameraDefaultPos, bool& arg_isHitUnderGround, bool arg_isMovePlayer, bool arg_isPlayerJump, KuroEngine::Quaternion arg_cameraQ, bool arg_isFrontWall, KuroEngine::Transform arg_drawTransform, KuroEngine::Vec3<float> arg_frontWallNormal, bool arg_isNoCollision, CAMERA_STATUS arg_cameraMode);

};