#include "CameraController.h"
#include"Render/RenderObject/Camera.h"
#include"../../../../src/engine/ForUser/Object/Model.h"
#include"CollisionDetectionOfRayAndMesh.h"

void CameraController::OnImguiItems()
{
	ImGui::Text("NowParameter");
	ImGui::SameLine();

}

CameraController::CameraController()
	:KuroEngine::Debugger("CameraController", true, true)
{

	AddCustomParameter("gazePointOffset", { "gazePointOffset" }, PARAM_TYPE::FLOAT_VEC3, &m_gazePointOffset, "UpdateParameter");
	AddCustomParameter("posOffsetDepthMin", { "posOffsetDepth","min" }, PARAM_TYPE::FLOAT, &m_posOffsetDepthMin, "UpdateParameter");
	AddCustomParameter("posOffsetDepthMax", { "posOffsetDepth","max" }, PARAM_TYPE::FLOAT, &m_posOffsetDepthMax, "UpdateParameter");
	AddCustomParameter("xAxisAngleMin", { "xAxisAngle","min" }, PARAM_TYPE::FLOAT, &m_xAxisAngleMin, "UpdateParameter");
	AddCustomParameter("xAxisAngleMax", { "xAxisAngle","max" }, PARAM_TYPE::FLOAT, &m_xAxisAngleMax, "UpdateParameter");
	AddCustomParameter("camFowardPosLerpRate", { "PosLerpRate" }, PARAM_TYPE::FLOAT, &m_camForwardPosLerpRate, "UpdateParameter");
	AddCustomParameter("camFollowLerpRate", { "FollowLerpRate" }, PARAM_TYPE::FLOAT, &m_camFollowLerpRate, "UpdateParameter");

	LoadParameterLog();
}

void CameraController::AttachCamera(std::shared_ptr<KuroEngine::Camera> arg_cam)
{
	//操作対象となるカメラのポインタを保持
	m_attachedCam = arg_cam;
	//コントローラーのトランスフォームを親として設定
	//m_attachedCam.lock()->GetTransform().SetParent(&m_camParentTransform);
}

void CameraController::Init()
{
	m_angleX = 0;
	m_oldAngleX = 0;
}

void CameraController::Update(KuroEngine::Vec3<float> arg_scopeMove, KuroEngine::Transform arg_playerTransform, float arg_cameraZ, const std::weak_ptr<Stage> arg_nowStage)
{
	using namespace KuroEngine;

	//カメラがアタッチされていない
	if (m_attachedCam.expired())return;

	//トランスフォームを保存。
	m_distanceZ = arg_cameraZ;
	m_playerTransform = arg_playerTransform;
	m_baseTransform.SetRotate(m_playerTransform.GetRotateWorld());

	//カメラのX軸回転を保存。
	const float SCOPE_SCALE = 0.3f;
	m_oldAngleX = m_angleX;
	m_angleX -= arg_scopeMove.y * SCOPE_SCALE;

	//補間先の位置と回転を設定
	SetCameraPos();

	//一旦回転を適用。
	m_oldPos = m_attachedCam.lock()->GetTransform().GetPos();
	m_attachedCam.lock()->GetTransform().SetRotate(m_baseTransform.GetRotate());
	m_attachedCam.lock()->GetTransform().SetPos(m_baseTransform.GetPos());

	//通常の地形を走査
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
			auto eyePos = m_attachedCam.lock()->GetTransform().GetPosWorld();
			auto moveVec = m_attachedCam.lock()->GetTransform().GetPosWorld() - m_oldPos;
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(m_oldPos, moveVec.GetNormal(), checkHitMesh);

			if (output.m_isHit && 0 < output.m_distance && output.m_distance < moveVec.Length()) {

				m_attachedCam.lock()->GetTransform().SetPos(output.m_pos + output.m_normal);

			}

			//=================================================
		}
	}

	//現在の座標からプレイヤーに向かう回転を求める。
	Vec3<float> playerDir = m_playerTransform.GetPos() - m_attachedCam.lock()->GetTransform().GetPosWorld();
	playerDir.Normalize();

	//プレイヤーの法線との外積から仮のXベクトルを得る。
	Vec3<float> preAxisX = -playerDir.Cross(m_playerTransform.GetUp());

	//仮のXベクトルから上ベクトルを得る。
	Vec3<float> axisY = playerDir.Cross(preAxisX);

	//本当のXベクトルを得る。
	Vec3<float> axisX = -playerDir.Cross(axisY);

	//姿勢を得る。
	DirectX::XMMATRIX matWorld = DirectX::XMMatrixIdentity();
	matWorld.r[0] = { axisX.x, axisX.y, axisX.z, 0.0f };
	matWorld.r[1] = { axisY.x, axisY.y, axisY.z, 0.0f };
	matWorld.r[2] = { playerDir.x, playerDir.y, playerDir.z, 0.0f };

	XMVECTOR rotate, scale, position;
	DirectX::XMMatrixDecompose(&scale, &rotate, &position, matWorld);

	m_attachedCam.lock()->GetTransform().SetRotate(rotate);

}

void CameraController::SetCameraPos() {

	using namespace KuroEngine;

	//背後にずらすベクトル
	const float Z_HEIGHT_OFFSET = 0.3f;
	KuroEngine::Vec3<float> zOffsetVec = -m_playerTransform.GetFront() + m_playerTransform.GetUp() * m_angleX;
	KuroEngine::Vec3<float> zOffset = zOffsetVec.GetNormal() * m_distanceZ;

	//背後にずらす。
	Vec3<float> cameraBasePos = m_playerTransform.GetPos() + zOffset;
	m_baseTransform.SetPos(KuroEngine::Math::Lerp(m_baseTransform.GetPos(), cameraBasePos, 0.9f));

	//プレイヤーのちょっと上方向を向かせる。
	const float UP_OFFSET = 3.0f;
	Vec3<float> playerDir = m_playerTransform.GetPos() + (m_playerTransform.GetUp() * UP_OFFSET) - m_baseTransform.GetPos();
	playerDir.Normalize();

	//回転を持たせる。
	Vec3<float> defVec = m_playerTransform.GetFront();
	defVec = defVec * (playerDir.Dot(defVec) / defVec.Dot(defVec));

	//回転軸と角度を求める。
	Vec3<float> axis = defVec.Cross(playerDir);
	float angle = acosf(defVec.Dot(playerDir));

	//回転軸があったら。
	if (0.001f < axis.Length()) {

		auto q = DirectX::XMQuaternionRotationAxis(axis, angle);
		m_baseTransform.SetRotate(DirectX::XMQuaternionMultiply(m_baseTransform.GetRotate(), q));

	}

}