#include "CameraController.h"
#include"Render/RenderObject/Camera.h"
#include"../../../../src/engine/ForUser/Object/Model.h"
#include"CollisionDetectionOfRayAndMesh.h"

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
	m_nowParam = m_initializedParam;
	m_verticalControl = ANGLE;
}

void CameraController::Update(KuroEngine::Transform arg_playerTransform, float arg_cameraZ)
{
	using namespace KuroEngine;

	//カメラがアタッチされていない
	if (m_attachedCam.expired())return;

	//トランスフォームを保存。
	m_attachedCam.lock()->GetTransform() = arg_playerTransform;

	//背後にずらすベクトル
	const float Z_HEIGHT_OFFSET = 0.3f;
	KuroEngine::Vec3<float> zOffsetVec = -arg_playerTransform.GetFront() + arg_playerTransform.GetUp() * Z_HEIGHT_OFFSET;
	KuroEngine::Vec3<float> zOffset = zOffsetVec.GetNormal() * arg_cameraZ;

	//背後にずらす。
	m_attachedCam.lock()->GetTransform().SetPos(arg_playerTransform.GetPos() + zOffset);

	//プレイヤーのちょっと上方向を向かせる。
	const float UP_OFFSET = 3.0f;
	Vec3<float> playerDir = arg_playerTransform.GetPos() + (arg_playerTransform.GetUp() * UP_OFFSET) - m_attachedCam.lock()->GetTransform().GetPos();
	playerDir.Normalize();

	//回転を持たせる。
	Vec3<float> defVec = arg_playerTransform.GetFront();
	defVec = defVec * (playerDir.Dot(defVec) / defVec.Dot(defVec));

	//回転軸と角度を求める。
	Vec3<float> axis = defVec.Cross(playerDir);
	float angle = acosf(defVec.Dot(playerDir));

	//回転軸があったら。
	if (0.1f < axis.Length()) {

		auto q = DirectX::XMQuaternionRotationAxis(axis, angle);
		m_attachedCam.lock()->GetTransform().SetRotate(DirectX::XMQuaternionMultiply(m_attachedCam.lock()->GetTransform().GetRotate(), q));

	}





	//oldTransform = m_attachedCam.lock()->GetTransform();

	////左右カメラ操作
	//m_nowParam.m_yAxisAngle = arg_playerRotY;

	////上下カメラ操作
	//switch (m_verticalControl)
	//{
	//case ANGLE:
	//	m_nowParam.m_xAxisAngle -= arg_scopeMove.y * 0.3f;
	//	//if (m_nowParam.m_xAxisAngle <= m_xAxisAngleMin)m_verticalControl = DIST;
	//	break;

	//case DIST:
	//	m_nowParam.m_posOffsetZ += arg_scopeMove.y * 6.0f;
	//	if (m_nowParam.m_posOffsetZ <= m_posOffsetDepthMin)m_verticalControl = ANGLE;
	//	break;
	//}

	////上限値超えないようにする
	//m_nowParam.m_posOffsetZ = arg_cameraZ;
	//m_nowParam.m_xAxisAngle = std::clamp(m_nowParam.m_xAxisAngle, m_xAxisAngleMin, m_xAxisAngleMax);


	////操作するカメラのトランスフォーム（前後移動）更新
	//Vec3<float> localPos = { 0,0,0 };
	//localPos.z = m_nowParam.m_posOffsetZ;
	//localPos.y = m_gazePointOffset.y + tan(-m_nowParam.m_xAxisAngle) * m_nowParam.m_posOffsetZ;
	//m_attachedCam.lock()->GetTransform().SetPos(Math::Lerp(m_attachedCam.lock()->GetTransform().GetPos(), localPos, m_camForwardPosLerpRate));
	//m_attachedCam.lock()->GetTransform().SetRotate(Vec3<float>::GetXAxis(), m_nowParam.m_xAxisAngle);

	////コントローラーのトランスフォーム（対象の周囲、左右移動）更新
	//m_camParentTransform.SetRotate(Vec3<float>::GetYAxis(), m_nowParam.m_yAxisAngle);
	//m_camParentTransform.SetPos(Math::Lerp(m_camParentTransform.GetPos(), arg_targetPos, m_camFollowLerpRate));






	////使用するカメラに回転を適用。
	//XMVECTOR rotate, translation, scaling;
	//DirectX::XMMatrixDecompose(&scaling, &rotate, &translation, m_cameraLocalTransform.GetMatWorld());

	//rotate = DirectX::XMQuaternionNormalize(rotate);

	//auto local = m_cameraLocalTransform.GetRotate();
	//auto world = m_cameraLocalTransform.GetRotateWorld();
	//m_attachedCam.lock()->GetTransform().SetRotate({ rotate.m128_f32[0], rotate .m128_f32[1], rotate .m128_f32[2]});

	//auto localScale = m_cameraLocalTransform.GetScaleWorld();
	//auto worldScale = m_cameraLocalTransform.GetScaleWorld();

	//auto eye = m_attachedCam.lock()->GetEye();

	////使用するカメラの座標を補間して適用。
	//m_attachedCam.lock()->GetTransform().SetPos({ translation.m128_f32[0], translation.m128_f32[1], translation.m128_f32[2] });
	//m_attachedCam.lock()->GetTransform().SetScale({ scaling.m128_f32[0], scaling.m128_f32[1], scaling.m128_f32[2] });

	//m_attachedCam.lock()->GetTransform() = m_cameraLocalTransform;

	//int a = 0;

}

void CameraController::TerrianMeshCollision(const std::weak_ptr<Stage> arg_nowStage)
{

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
			auto moveVec = m_attachedCam.lock()->GetTransform().GetPosWorld() - oldTransform.GetPosWorld();
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(eyePos, moveVec.GetNormal(), checkHitMesh);

			if (output.m_isHit && 0 < output.m_distance && output.m_distance < moveVec.Length()) {

				//m_attachedCam.lock()->GetTransform().SetPos(m_attachedCam.lock()->GetTransform().GetPos() - moveVec.GetNormal() * moveVec.Length());

			}

			//=================================================
		}
	}

}
