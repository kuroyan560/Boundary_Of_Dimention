#include "CameraController.h"
#include"Render/RenderObject/Camera.h"

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
		auto pos = m_controllerTransform.GetPosWorld();
		ImGui::Text("pos : { %.2f , %.2f , %.2f }", pos.x, pos.y, pos.z);
		auto up = m_controllerTransform.GetUpWorld();
		ImGui::Text("up : { %.2f , %.2f , %.2f }", up.x, up.y, up.z);

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
	AddCustomParameter("posOffsetDepthMin", { "posOffsetDepth","min"}, PARAM_TYPE::FLOAT, &m_posOffsetDepthMin, "UpdateParameter");
	AddCustomParameter("posOffsetDepthMax", { "posOffsetDepth","max"}, PARAM_TYPE::FLOAT, &m_posOffsetDepthMax, "UpdateParameter");
	AddCustomParameter("xAxisAngleMin", { "xAxisAngle","min"}, PARAM_TYPE::FLOAT, &m_xAxisAngleMin, "UpdateParameter");
	AddCustomParameter("xAxisAngleMax", { "xAxisAngle","max"}, PARAM_TYPE::FLOAT, &m_xAxisAngleMax, "UpdateParameter");

	LoadParameterLog();
}

void CameraController::AttachCamera(std::shared_ptr<KuroEngine::Camera> arg_cam)
{
	//操作対象となるカメラのポインタを保持
	m_attachedCam = arg_cam;
	m_attachedCam.lock()->GetTransform().SetParent(&m_controllerTransform);
	m_controllerTransform.SetParent(&m_copyPlayerTransform);
}

void CameraController::Init(const KuroEngine::Vec3<float>& arg_playerPos, const KuroEngine::Quaternion& arg_playerRotate)
{
	m_nowParam = m_initializedParam;
	m_verticalControl = ANGLE;

	//プレイヤーのトランスフォームを補間なしでそのままコピー
	m_copyPlayerTransform.SetPos(arg_playerPos);
	m_copyPlayerTransform.SetRotate(arg_playerRotate);
}

void CameraController::Update(KuroEngine::Vec3<float>arg_scopeMove, const KuroEngine::Vec3<float>& arg_playerPos, const KuroEngine::Quaternion& arg_playerRotate, float arg_cameraY)
{
	using namespace KuroEngine;
	
	//カメラがアタッチされていない
	if (m_attachedCam.expired())return;

	//左右カメラ操作
	m_nowParam.m_yAxisAngle = arg_cameraY;

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

	//上限値超えないようにする
	m_nowParam.m_posOffsetZ = std::clamp(m_nowParam.m_posOffsetZ, m_posOffsetDepthMin, m_posOffsetDepthMax);
	m_nowParam.m_xAxisAngle = std::clamp(m_nowParam.m_xAxisAngle, m_xAxisAngleMin, m_xAxisAngleMax);

	//プレイヤーのトランスフォームを補間しながらコピー
	m_copyPlayerTransform.SetPos(Math::Lerp(m_copyPlayerTransform.GetPos(), arg_playerPos, m_playerPosLerpRate));
	m_copyPlayerTransform.SetRotate(XMQuaternionSlerp(m_copyPlayerTransform.GetRotate(), arg_playerRotate, m_playerQuaternionLerpRate));

	//操作するカメラのトランスフォーム（前後移動）更新
	auto& transform = m_attachedCam.lock()->GetTransform();
	Vec3<float> localPos = { 0,0,0 };
	localPos.z = m_nowParam.m_posOffsetZ;
	localPos.y = m_gazePointOffset.y + tan(-m_nowParam.m_xAxisAngle) * m_nowParam.m_posOffsetZ;
	localPos = Math::TransformVec3(localPos, { 0.0f,1.0f,0.0f }, m_nowParam.m_yAxisAngle);
	m_controllerTransform.SetPos(localPos);
	m_controllerTransform.SetRotate(m_nowParam.m_xAxisAngle, m_nowParam.m_yAxisAngle,0.0f );
}