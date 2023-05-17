#include "CameraController.h"
#include"Render/RenderObject/Camera.h"
#include"../../../../src/engine/ForUser/Object/Model.h"
#include"CollisionDetectionOfRayAndMesh.h"
#include"FrameWork/UsersInput.h"

void CameraController::OnImguiItems()
{
	ImGui::Text("NowParameter");
	ImGui::SameLine();

	//パラメータ初期化ボタン
	if (ImGui::Button("Initialize"))
	{
		m_nowParam = m_initializedParam;
	}

	//現在のパラメータ表示
	if (ImGui::BeginChild("NowParam"))
	{
		ImGui::Text("posOffsetZ : %.2f", m_nowParam.m_posOffsetZ);
		float degree = static_cast<float>(KuroEngine::Angle::ConvertToDegree(m_nowParam.m_xAxisAngle));
		ImGui::Text("xAxisAngle : %.2f", degree);
		degree = static_cast<float>(KuroEngine::Angle::ConvertToDegree(m_nowParam.m_yAxisAngle));
		ImGui::Text("yAxisAngle : %.2f", degree);
		ImGui::EndChild();
	}

}

CameraController::CameraController()
	:KuroEngine::Debugger("CameraController", true, true)
{
	AddCustomParameter("posOffsetZ", { "InitializedParameter","posOffsetZ" }, PARAM_TYPE::FLOAT, &m_initializedParam.m_posOffsetZ, "InitializedParameter");
	AddCustomParameter("xAxisAngle", { "InitializedParameter","xAxisAngle" }, PARAM_TYPE::FLOAT, &m_initializedParam.m_xAxisAngle, "InitializedParameter");

	AddCustomParameter("gazePointOffset", { "gazePointOffset" }, PARAM_TYPE::FLOAT_VEC3, &m_gazePointOffset, "UpdateParameter");
	AddCustomParameter("posOffsetDepthMin", { "posOffsetDepth","min" }, PARAM_TYPE::FLOAT, &m_posOffsetDepthMin, "UpdateParameter");
	AddCustomParameter("posOffsetDepthMax", { "posOffsetDepth","max" }, PARAM_TYPE::FLOAT, &m_posOffsetDepthMax, "UpdateParameter");
	//AddCustomParameter("xAxisAngleMin", { "xAxisAngle","min" }, PARAM_TYPE::FLOAT, &m_xAxisAngleMin, "UpdateParameter");
	//AddCustomParameter("xAxisAngleMax", { "xAxisAngle","max" }, PARAM_TYPE::FLOAT, &m_xAxisAngleMax, "UpdateParameter");
	AddCustomParameter("camFowardPosLerpRate", { "PosLerpRate" }, PARAM_TYPE::FLOAT, &m_camForwardPosLerpRate, "UpdateParameter");
	AddCustomParameter("camFollowLerpRate", { "FollowLerpRate" }, PARAM_TYPE::FLOAT, &m_camFollowLerpRate, "UpdateParameter");

	LoadParameterLog();
}

void CameraController::AttachCamera(std::shared_ptr<KuroEngine::Camera> arg_cam)
{
	//操作対象となるカメラのポインタを保持
	m_attachedCam = arg_cam;
	//コントローラーのトランスフォームを親として設定
	m_cameraLocalTransform.SetParent(&m_camParentTransform);
}

void CameraController::Init()
{
	m_nowParam = m_initializedParam;
	m_verticalControl = ANGLE;
	m_rotateZ = 0;
	m_rotateYLerpAmount = 0;
	m_cameraXAngleLerpAmount = 0;
	m_cameraRotXStorage = 0;
	m_cameraRotYStorage = 0;
	m_isHitUnderGroundTerrian = false;
	m_isOldHitUnderGroundTerrian = false;
}

void CameraController::Update(KuroEngine::Vec3<float>arg_scopeMove, KuroEngine::Transform arg_targetPos, float& arg_playerRotY, float arg_cameraZ, const std::weak_ptr<Stage>arg_nowStage, bool arg_isCameraUpInverse, bool arg_isCameraDefaultPos, bool& arg_isHitUnderGround, bool arg_isMovePlayer)
{
	using namespace KuroEngine;

	//カメラがアタッチされていない
	if (m_attachedCam.expired())return;

	//トランスフォームを保存。
	m_oldCameraWorldPos = m_attachedCam.lock()->GetTransform().GetPos();

	//横の壁に居るときの注視点移動の場合、Y軸回転を動かす。
	if (m_isHitUnderGroundTerrian && fabs(arg_targetPos.GetUp().y) < 0.9f) {

		//壁にあたっていないときに保存した回転角と現在の回転角の差分を求めて、規定値以上動かないようにする。
		float subAngleY = arg_playerRotY - m_playerRotYStorage;

		//既定値を超えていたら。
		if (PLAYER_TARGET_MOVE_SIDE < fabs(subAngleY)) {

			//回転量を押し戻す。
			arg_playerRotY = m_playerRotYStorage + (signbit(subAngleY) ? -1.0f : 1.0f) * PLAYER_TARGET_MOVE_SIDE;

		}

	}

	//左右カメラ操作
	m_nowParam.m_yAxisAngle = arg_playerRotY;

	//上下カメラ操作
	switch (m_verticalControl)
	{
	case ANGLE:
		m_nowParam.m_xAxisAngle -= arg_scopeMove.y * 0.3f;
		//if (m_nowParam.m_xAxisAngle <= m_xAxisAngleMin)m_verticalControl = DIST;
		break;

	case DIST:
		m_nowParam.m_posOffsetZ += arg_scopeMove.y * 6.0f;
		if (m_nowParam.m_posOffsetZ <= m_posOffsetDepthMin)m_verticalControl = ANGLE;
		break;
	}

	//カメラが地上にあたっていて、その当たっている面が横の壁だったら回転を保存しておく。
	if (!m_isHitUnderGroundTerrian) {
		m_playerRotYStorage = arg_playerRotY;
	}

	//上限値超えないようにする
	m_nowParam.m_posOffsetZ = arg_cameraZ;
	m_nowParam.m_xAxisAngle = std::clamp(m_nowParam.m_xAxisAngle, m_xAxisAngleMin, m_xAxisAngleMax);

	//床にカメラが当たっているときにプレイヤーが動いていたら、床がカメラに当たらないようにX軸回転を元に戻す。
	if ((m_isHitUnderGroundTerrian && arg_isMovePlayer)) {

		//横の壁にいたら 
		if (fabs(arg_targetPos.GetUp().y) < 0.9f && fabs(m_playerRotYLerp) < 0.1f) {

			//Y軸回転を押し戻す量。
			float rotYScale = (arg_playerRotY - m_playerRotYStorage);
			//Y軸回転をぴったり押し戻すと地面スレスレになってしまうので、多少オフセットを設けることで良い感じの高さにする。
			const float ROTY_OFFSET = 0.8f;
			float rotYOffset = (std::signbit(rotYScale) ? -1.0f : 1.0f) * ROTY_OFFSET + rotYScale;
			m_playerRotYLerp -= rotYOffset;

		}
		//上下の壁にいたら
		else if(0.9f < fabs(arg_targetPos.GetUp().y)) {

			//カメラが反転しているかしていないかによって入れる値を決める。
			if (arg_isCameraUpInverse) {
				m_cameraXAngleLerpAmount = m_xAxisAngleMin;
			}
			else {
				m_cameraXAngleLerpAmount = m_xAxisAngleMax;
			}

		}

	}

	////カメラを初期位置に戻すか。
	//if (arg_isCameraDefaultPos) {

	//	//カメラが反転しているかしていないかによって入れる値を決める。
	//	if (arg_isCameraUpInverse) {
	//		m_cameraXAngleLerpAmount = m_xAxisAngleMin;
	//	}
	//	else {
	//		m_cameraXAngleLerpAmount = m_xAxisAngleMax;
	//	}

	//	//地形に当たっていたら
	//	if (m_isHitTerrian) {
	//		m_rotateYLerpAmount += DirectX::XM_PI;
	//	}

	//}

	//Y軸補間量があったらいい感じに補間する。
	if (0 < fabs(m_playerRotYLerp)) {
		float lerp = m_playerRotYLerp - KuroEngine::Math::Lerp(m_playerRotYLerp, 0.0f, 0.08f);
		m_playerRotYLerp -= lerp;
		arg_playerRotY += lerp;
		m_nowParam.m_yAxisAngle = arg_playerRotY;
	}

	//カメラを初期位置に戻す量が0以上だったらカメラの回転量を補間。
	if (0 < fabs(m_rotateYLerpAmount)) {
		float lerp = m_rotateYLerpAmount - KuroEngine::Math::Lerp(m_rotateYLerpAmount, 0.0f, 0.08f);
		m_rotateYLerpAmount -= lerp;
		arg_playerRotY += lerp;
	}
	if (0 < fabs(m_cameraXAngleLerpAmount)) {
		float lerp = m_cameraXAngleLerpAmount - KuroEngine::Math::Lerp(m_cameraXAngleLerpAmount, 0.0f, 0.08f);
		m_cameraXAngleLerpAmount = (fabs(m_cameraXAngleLerpAmount) - fabs(lerp)) * (signbit(m_cameraXAngleLerpAmount) ? -1.0f : 1.0f);
		m_nowParam.m_xAxisAngle += lerp;
	}



	//X軸回転に計算する角度を決める。
	float useXAngle = m_nowParam.m_xAxisAngle;

	//カメラが地形にあたっているとき。
	if (m_isHitUnderGroundTerrian) {

		//プレイヤーが下のかべに居るとき。
		if (0.9f < arg_targetPos.GetUp().y) {

			//保存してある回転量より現在の回転量が下回っていたら戻す。
			float xAngle = m_nowParam.m_xAxisAngle;
			if (xAngle < m_cameraRotXStorage) {

				useXAngle = m_cameraRotXStorage;

			}

		}
		//プレイヤーが下のかべに居るとき。
		else if (arg_targetPos.GetUp().y < 0.9f) {

			//保存してある回転量より現在の回転量が上回っていたら戻す。
			float xAngle = m_nowParam.m_xAxisAngle;
			if (m_cameraRotXStorage < xAngle) {

				useXAngle = m_cameraRotXStorage;

			}

		}

	}

	//操作するカメラのトランスフォーム（前後移動）更新
	Vec3<float> localPos = { 0,0,0 };
	localPos.z = m_nowParam.m_posOffsetZ;
	localPos.y = m_gazePointOffset.y + tan(-useXAngle) * m_nowParam.m_posOffsetZ;
	m_cameraLocalTransform.SetPos(Math::Lerp(m_cameraLocalTransform.GetPos(), localPos, m_camForwardPosLerpRate));
	m_cameraLocalTransform.SetRotate(Vec3<float>::GetXAxis(), useXAngle);

	//コントローラーのトランスフォーム（対象の周囲、左右移動）更新
	m_camParentTransform.SetRotate(Vec3<float>::GetYAxis(), m_nowParam.m_yAxisAngle);
	m_camParentTransform.SetPos(Math::Lerp(m_camParentTransform.GetPos(), arg_targetPos.GetPosWorld(), m_camFollowLerpRate));


	//使用するカメラの座標を補間して適用。
	Vec3<float> pushBackPos = m_cameraLocalTransform.GetPosWorldByMatrix();

	//当たり判定用のレイを打つ方向を決める。
	Vec3<float> checkHitRay = m_cameraLocalTransform.GetPosWorldByMatrix() - arg_targetPos.GetPos();	//まずはデフォルトのレイに設定。

	//通常の地形を走査
	m_isHitTerrian = false;
	m_isOldHitUnderGroundTerrian = m_isHitUnderGroundTerrian;
	m_isHitUnderGroundTerrian = false;
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//モデル情報取得
		auto model = terrian.GetModel().lock();

		//メッシュを走査
		for (auto& modelMesh : model->m_meshes)
		{

			//当たり判定に使用するメッシュ
			auto checkHitMesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//判定↓============================================


			//当たり判定を実行
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_targetPos.GetPos(), checkHitRay.GetNormal(), checkHitMesh);

			if (output.m_isHit && 0 < output.m_distance && output.m_distance < fabs(arg_cameraZ)) {

				pushBackPos = output.m_pos + output.m_normal;
				m_isHitTerrian = true;

				//プレイヤーの法線と比べて同じだったら地上に当たった判定にする。
				float dot = output.m_normal.Dot(arg_targetPos.GetUp());
				if (0.9f < dot) {
					//地上にあたっている。
					m_isHitUnderGroundTerrian = true;
				}

			}

			//=================================================
		}
	}

	//地形に当たったトリガーだったら
	if (!m_isOldHitUnderGroundTerrian && m_isHitUnderGroundTerrian) {

		//上下の壁にいたら
		if (0.9f < fabs(arg_targetPos.GetUp().y)) {

			//その時点のX軸回転を保存。
			m_cameraRotXStorage = m_nowParam.m_xAxisAngle;

		}
		//左右の壁にいたら。
		else {

			m_cameraRotYStorage = m_nowParam.m_yAxisAngle;

		}

	}

	//地上にあたっていたら地形と押し戻す前の座標からの回転を求めることで、注視点を上に向ける。
	if (m_isHitUnderGroundTerrian) {


		//補間する。
		m_attachedCam.lock()->GetTransform().SetPos(KuroEngine::Math::Lerp(m_attachedCam.lock()->GetTransform().GetPos(), pushBackPos, 0.3f));

		Vec3<float> localPos = { 0,0,0 };
		localPos.z = m_nowParam.m_posOffsetZ;
		localPos.y = m_gazePointOffset.y + tan(-m_nowParam.m_xAxisAngle) * m_nowParam.m_posOffsetZ;
		KuroEngine::Transform local;
		local.SetPos(Math::Lerp(local.GetPos(), localPos, m_camForwardPosLerpRate));
		local.SetRotate(Vec3<float>::GetXAxis(), m_nowParam.m_xAxisAngle);

		//コントローラーのトランスフォーム（対象の周囲、左右移動）更新
		KuroEngine::Transform world;
		world.SetRotate(Vec3<float>::GetYAxis(), m_nowParam.m_yAxisAngle);
		world.SetPos(Math::Lerp(world.GetPos(), arg_targetPos.GetPosWorld(), m_camFollowLerpRate));
		local.SetParent(&world);

		//現在の座標からプレイヤーに向かう回転を求める。
		Vec3<float> axisZ = arg_targetPos.GetPos() - local.GetPosWorldByMatrix();
		axisZ.Normalize();

		//プレイヤーの法線との外積から仮のXベクトルを得る。
		Vec3<float> axisX = Vec3<float>(0, 1, 0).Cross(axisZ);

		//Xベクトルから上ベクトルを得る。
		Vec3<float> axisY = axisZ.Cross(axisX);

		//姿勢を得る。
		DirectX::XMMATRIX matWorld = DirectX::XMMatrixIdentity();
		matWorld.r[0] = { axisX.x, axisX.y, axisX.z, 0.0f };
		matWorld.r[1] = { axisY.x, axisY.y, axisY.z, 0.0f };
		matWorld.r[2] = { axisZ.x, axisZ.y, axisZ.z, 0.0f };

		XMVECTOR rotate, scale, position;
		DirectX::XMMatrixDecompose(&scale, &rotate, &position, matWorld);

		//回転を反転させる。
		if (arg_isCameraUpInverse) {
			m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, DirectX::XM_PI, 0.2f);
		}
		else {
			m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, 0.0f, 0.1f);
		}
		rotate = DirectX::XMQuaternionMultiply(rotate, DirectX::XMQuaternionRotationAxis(axisZ, m_rotateZ));

		rotate = DirectX::XMQuaternionNormalize(rotate);

		//回転を適用。
		m_attachedCam.lock()->GetTransform().SetRotate(rotate);

	}
	else {

		//補間する。
		m_attachedCam.lock()->GetTransform().SetPos(KuroEngine::Math::Lerp(m_attachedCam.lock()->GetTransform().GetPos(), pushBackPos, 0.3f));

		//現在の座標からプレイヤーに向かう回転を求める。
		Vec3<float> axisZ = arg_targetPos.GetPos() - m_attachedCam.lock()->GetTransform().GetPosWorld();
		axisZ.Normalize();

		//プレイヤーの法線との外積から仮のXベクトルを得る。
		Vec3<float> axisX = Vec3<float>(0, 1, 0).Cross(axisZ);

		//Xベクトルから上ベクトルを得る。
		Vec3<float> axisY = axisZ.Cross(axisX);

		//姿勢を得る。
		DirectX::XMMATRIX matWorld = DirectX::XMMatrixIdentity();
		matWorld.r[0] = { axisX.x, axisX.y, axisX.z, 0.0f };
		matWorld.r[1] = { axisY.x, axisY.y, axisY.z, 0.0f };
		matWorld.r[2] = { axisZ.x, axisZ.y, axisZ.z, 0.0f };

		XMVECTOR rotate, scale, position;
		DirectX::XMMatrixDecompose(&scale, &rotate, &position, matWorld);

		//回転を反転させる。
		if (arg_isCameraUpInverse) {
			m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, DirectX::XM_PI, 0.08f);
		}
		else {
			m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, 0.0f, 0.08f);
		}
		rotate = DirectX::XMQuaternionMultiply(rotate, DirectX::XMQuaternionRotationAxis(axisZ, m_rotateZ));

		rotate = DirectX::XMQuaternionNormalize(rotate);

		//回転を適用。
		m_attachedCam.lock()->GetTransform().SetRotate(rotate);

	}

	//地上にあたっているフラグを保存。これがtrueだと注視点モードになるので、プレイヤーが居る面によってカメラの回転を打ち消す。
	arg_isHitUnderGround = m_isHitUnderGroundTerrian;


	//距離を求める。
	//const float PUSHBACK = 20.0f;
	//float distance = KuroEngine::Vec3<float>(m_attachedCam.lock()->GetTransform().GetPos() - arg_targetPos.GetPos()).Length();
	//if (distance <= PUSHBACK) {
	//	float pushBackDistance = PUSHBACK - distance;
	//	m_attachedCam.lock()->GetTransform().SetPos(m_attachedCam.lock()->GetTransform().GetPos() + KuroEngine::Vec3<float>(m_attachedCam.lock()->GetTransform().GetPos() - arg_targetPos.GetPos()).GetNormal() * pushBackDistance);
	//}

}
