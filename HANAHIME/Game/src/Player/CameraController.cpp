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
		//m_nowParam = m_initializedParam;
	}

	//現在のパラメータ表示
	if (ImGui::BeginChild("NowParam"))
	{
		//ImGui::Text("posOffsetZ : %.2f", m_nowParam.m_posOffsetZ);
		//float degree = static_cast<float>(KuroEngine::Angle::ConvertToDegree(m_nowParam.m_xAxisAngle));
		//ImGui::Text("xAxisAngle : %.2f", degree);
		//degree = static_cast<float>(KuroEngine::Angle::ConvertToDegree(m_nowParam.m_yAxisAngle));
		//ImGui::Text("yAxisAngle : %.2f", degree);
		ImGui::EndChild();
	}

}

CameraController::CameraController()
	:KuroEngine::Debugger("CameraController", true, true)
{
	//AddCustomParameter("posOffsetZ", { "InitializedParameter","posOffsetZ" }, PARAM_TYPE::FLOAT, &m_initializedParam.m_posOffsetZ, "InitializedParameter");
	//AddCustomParameter("xAxisAngle", { "InitializedParameter","xAxisAngle" }, PARAM_TYPE::FLOAT, &m_initializedParam.m_xAxisAngle, "InitializedParameter");

	AddCustomParameter("gazePointOffset", { "gazePointOffset" }, PARAM_TYPE::FLOAT_VEC3, &m_gazePointOffset, "UpdateParameter");
	AddCustomParameter("posOffsetDepthMin", { "posOffsetDepth","min" }, PARAM_TYPE::FLOAT, &m_posOffsetDepthMin, "UpdateParameter");
	AddCustomParameter("posOffsetDepthMax", { "posOffsetDepth","max" }, PARAM_TYPE::FLOAT, &m_posOffsetDepthMax, "UpdateParameter");
	//AddCustomParameter("xAxisAngleMin", { "xAxisAngle","min" }, PARAM_TYPE::FLOAT, &m_xAxisAngleMin, "UpdateParameter");
	//AddCustomParameter("xAxisAngleMax", { "xAxisAngle","max" }, PARAM_TYPE::FLOAT, &m_xAxisAngleMax, "UpdateParameter");
	//AddCustomParameter("camFowardPosLerpRate", { "PosLerpRate" }, PARAM_TYPE::FLOAT, &m_camForwardPosLerpRate, "UpdateParameter");
	//AddCustomParameter("camFollowLerpRate", { "FollowLerpRate" }, PARAM_TYPE::FLOAT, &m_camFollowLerpRate, "UpdateParameter");

	LoadParameterLog();
}

void CameraController::AttachCamera(std::shared_ptr<KuroEngine::Camera> arg_cam)
{
	//操作対象となるカメラのポインタを保持
	m_attachedCam = arg_cam;
}

void CameraController::Init(const KuroEngine::Vec3<float>& arg_up, float arg_rotateY)
{
	m_cameraQ = DirectX::XMQuaternionRotationAxis(arg_up, -arg_rotateY);
	m_attachedCam.lock()->GetTransform().SetPos(KuroEngine::Vec3<float>(0,19,0) + KuroEngine::Vec3<float>(0, 0, -1) * fabs(20.0f));
	m_rotateZ = 0;
	m_rotateYLerpAmount = 0;
	m_cameraXAngleLerpAmount = 0;
	m_isHitUnderGroundTerrian = false;
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

	//カメラが反転しているかのフラグによって上ベクトルを決める。
	Vec3<float> upVec(0, 1, 0);
	if (arg_isCameraUpInverse) {
		upVec = Vec3<float>(0, -1, 0);
	}
	Vec3<float> rightVec = upVec.Cross(Vec3<float>(arg_targetPos.GetPos() - m_attachedCam.lock()->GetTransform().GetPos()).GetNormal());

	//デフォルトだとY軸の動きが逆なので反転。
	arg_scopeMove.y *= -1.0f;

	//カメラのトランスフォーム。
	auto& cameraTransform = m_attachedCam.lock()->GetTransform();

	//入力からクォータニオンを回転させる。
	KuroEngine::Quaternion scopeMoveQ = DirectX::XMQuaternionIdentity();
	scopeMoveQ = DirectX::XMQuaternionMultiply(scopeMoveQ, XMQuaternionRotationAxis(upVec, arg_scopeMove.x));
	if (0.1f < rightVec.Length())scopeMoveQ = DirectX::XMQuaternionMultiply(scopeMoveQ, XMQuaternionRotationAxis(rightVec, arg_scopeMove.y));

	//カメラの位置をクォータニオンから求める。
	Vec3<float> cameraDir = Vec3<float>(cameraTransform.GetPos() - arg_targetPos.GetPosWorld()).GetNormal();
	cameraDir = KuroEngine::Math::TransformVec3(cameraDir, scopeMoveQ);
	Vec3<float> cameraPos = arg_targetPos.GetPosWorld() + cameraDir * fabs(arg_cameraZ);

	//カメラの座標を補間。
	cameraTransform.SetPos(cameraPos);






	//現在の座標からプレイヤーに向かう回転を求める。
	Vec3<float> axisZ = arg_targetPos.GetPos() - cameraTransform.GetPosWorld();
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
	m_cameraQ = DirectX::XMQuaternionMultiply(rotate, scopeMoveQ);

	//カメラが反転するフラグが立っていたら反転させる。
	if (arg_isCameraUpInverse) {
		m_cameraQ = DirectX::XMQuaternionMultiply(m_cameraQ, XMQuaternionRotationAxis(axisZ, DirectX::XM_PI));
	}

	cameraTransform.SetRotate(m_cameraQ);

	////通常の地形を走査
	//m_isHitTerrian = false;
	//m_isHitUnderGroundTerrian = false;
	//for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	//{
	//	//モデル情報取得
	//	auto model = terrian.GetModel().lock();

	//	//メッシュを走査
	//	for (auto& modelMesh : model->m_meshes)
	//	{

	//		//当たり判定に使用するメッシュ
	//		auto checkHitMesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];

	//		//判定↓============================================


	//		//当たり判定を実行
	//		CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_targetPos.GetPos(), checkHitRay.GetNormal(), checkHitMesh);

	//		if (output.m_isHit && 0 < output.m_distance && output.m_distance < fabs(arg_cameraZ)) {

	//			pushBackPos = output.m_pos + output.m_normal;
	//			m_isHitTerrian = true;

	//			//プレイヤーの法線と比べて同じだったら地上に当たった判定にする。
	//			float dot = output.m_normal.Dot(arg_targetPos.GetUp());
	//			if (0.9f < dot) {
	//				//地上にあたっている。
	//				m_isHitUnderGroundTerrian = true;
	//			}

	//		}

	//		//=================================================
	//	}
	//}

	////地上にあたっていたら地形と押し戻す前の座標からの回転を求めることで、注視点を上に向ける。
	//if (m_isHitUnderGroundTerrian) {

	//	//カメラまでのベクトル。
	//	KuroEngine::Vec3<float> cameraDir = (m_attachedCam.lock()->GetTransform().GetPos() - arg_targetPos.GetPosWorld()).GetNormal();

	//	//カメラが動いた量回転させる。

	//	//下の面にいる場合
	//	if (0.9f < arg_targetPos.GetUp().y) {
	//		cameraDir = KuroEngine::Math::TransformVec3(cameraDir, DirectX::XMQuaternionRotationAxis(arg_targetPos.GetUp(), arg_scopeMove.x));
	//	}
	//	//上の面にいる場合
	//	else if (arg_targetPos.GetUp().y < -0.9f) {
	//		cameraDir = KuroEngine::Math::TransformVec3(cameraDir, DirectX::XMQuaternionRotationAxis(arg_targetPos.GetUp(), -arg_scopeMove.x));
	//	}
	//	//横の面にいる場合
	//	else if (0.9f < arg_targetPos.GetUp().y) {
	//		cameraDir = KuroEngine::Math::TransformVec3(cameraDir, DirectX::XMQuaternionRotationAxis(arg_targetPos.GetUp(), arg_scopeMove.y));
	//	}

	//	//座標を動かす。
	//	m_attachedCam.lock()->GetTransform().SetPos(arg_targetPos.GetPosWorld() + cameraDir * fabs(arg_cameraZ));

	//	//現在の座標からプレイヤーに向かう回転を求める。
	//	Vec3<float> axisZ = arg_targetPos.GetPos() - m_cameraLocalTransform.GetPosWorldByMatrix();
	//	axisZ.Normalize();

	//	//プレイヤーの法線との外積から仮のXベクトルを得る。
	//	Vec3<float> axisX = Vec3<float>(0, 1, 0).Cross(axisZ);

	//	//Xベクトルから上ベクトルを得る。
	//	Vec3<float> axisY = axisZ.Cross(axisX);

	//	//姿勢を得る。
	//	DirectX::XMMATRIX matWorld = DirectX::XMMatrixIdentity();
	//	matWorld.r[0] = { axisX.x, axisX.y, axisX.z, 0.0f };
	//	matWorld.r[1] = { axisY.x, axisY.y, axisY.z, 0.0f };
	//	matWorld.r[2] = { axisZ.x, axisZ.y, axisZ.z, 0.0f };

	//	XMVECTOR rotate, scale, position;
	//	DirectX::XMMatrixDecompose(&scale, &rotate, &position, matWorld);

	//	//回転を反転させる。
	//	if (arg_isCameraUpInverse) {
	//		m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, DirectX::XM_PI, 0.08f);
	//	}
	//	else {
	//		m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, 0.0f, 0.1f);
	//	}
	//	rotate = DirectX::XMQuaternionMultiply(rotate, DirectX::XMQuaternionRotationAxis(axisZ, m_rotateZ));

	//	rotate = DirectX::XMQuaternionNormalize(rotate);

	//	//回転を適用。
	//	m_attachedCam.lock()->GetTransform().SetRotate(rotate);

	//}
	//else {

	//	//補間する。
	//	m_attachedCam.lock()->GetTransform().SetPos(KuroEngine::Math::Lerp(m_attachedCam.lock()->GetTransform().GetPos(), pushBackPos, 0.3f));

	//	//現在の座標からプレイヤーに向かう回転を求める。
	//	Vec3<float> axisZ = arg_targetPos.GetPos() - m_attachedCam.lock()->GetTransform().GetPosWorld();
	//	axisZ.Normalize();

	//	//プレイヤーの法線との外積から仮のXベクトルを得る。
	//	Vec3<float> axisX = Vec3<float>(0, 1, 0).Cross(axisZ);

	//	//Xベクトルから上ベクトルを得る。
	//	Vec3<float> axisY = axisZ.Cross(axisX);

	//	//姿勢を得る。
	//	DirectX::XMMATRIX matWorld = DirectX::XMMatrixIdentity();
	//	matWorld.r[0] = { axisX.x, axisX.y, axisX.z, 0.0f };
	//	matWorld.r[1] = { axisY.x, axisY.y, axisY.z, 0.0f };
	//	matWorld.r[2] = { axisZ.x, axisZ.y, axisZ.z, 0.0f };

	//	XMVECTOR rotate, scale, position;
	//	DirectX::XMMatrixDecompose(&scale, &rotate, &position, matWorld);

	//	//回転を反転させる。
	//	if (arg_isCameraUpInverse) {
	//		m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, DirectX::XM_PI, 0.08f);
	//	}
	//	else {
	//		m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, 0.0f, 0.08f);
	//	}
	//	rotate = DirectX::XMQuaternionMultiply(rotate, DirectX::XMQuaternionRotationAxis(axisZ, m_rotateZ));

	//	rotate = DirectX::XMQuaternionNormalize(rotate);

	//	//回転を適用。
	//	m_attachedCam.lock()->GetTransform().SetRotate(rotate);

	//}

	//地上にあたっているフラグを保存。これがtrueだと注視点モードになるので、プレイヤーが居る面によってカメラの回転を打ち消す。
	arg_isHitUnderGround = m_isHitUnderGroundTerrian;

}
