#include "CameraController.h"
#include"Render/RenderObject/Camera.h"

void CameraController::OnImguiItems()
{
	ImGui::Text("NowParameter");
	ImGui::SameLine();

	//�p�����[�^�������{�^��
	if (ImGui::Button("Initialize"))
	{
		m_nowParam = m_initializedParam;
	}

	//���݂̃p�����[�^�\��
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
	AddCustomParameter("posOffsetDepthMin", { "posOffsetDepth","min"}, PARAM_TYPE::FLOAT, &m_posOffsetDepthMin, "UpdateParameter");
	AddCustomParameter("posOffsetDepthMax", { "posOffsetDepth","max"}, PARAM_TYPE::FLOAT, &m_posOffsetDepthMax, "UpdateParameter");
	AddCustomParameter("xAxisAngleMin", { "xAxisAngle","min"}, PARAM_TYPE::FLOAT, &m_xAxisAngleMin, "UpdateParameter");
	AddCustomParameter("xAxisAngleMax", { "xAxisAngle","max"}, PARAM_TYPE::FLOAT, &m_xAxisAngleMax, "UpdateParameter");
	AddCustomParameter("camFowardPosLerpRate", { "PosLerpRate" }, PARAM_TYPE::FLOAT, &m_camForwardPosLerpRate, "UpdateParameter");
	AddCustomParameter("camFollowLerpRate", { "FollowLerpRate" }, PARAM_TYPE::FLOAT, &m_camFollowLerpRate, "UpdateParameter");

	LoadParameterLog();
}

void CameraController::AttachCamera(std::shared_ptr<KuroEngine::Camera> arg_cam)
{
	//����ΏۂƂȂ�J�����̃|�C���^��ێ�
	m_attachedCam = arg_cam;
	//�R���g���[���[�̃g�����X�t�H�[����e�Ƃ��Đݒ�
	arg_cam->GetTransform().SetParent(&m_camParentTransform);
}

void CameraController::Init()
{
	m_nowParam = m_initializedParam;
	m_verticalControl = ANGLE;
}

void CameraController::Update(KuroEngine::Vec3<float>arg_scopeMove, KuroEngine::Vec3<float>arg_targetPos)
{
	using namespace KuroEngine;
	
	//�J�������A�^�b�`����Ă��Ȃ�
	if (m_attachedCam.expired())return;

	//���E�J��������
	m_nowParam.m_yAxisAngle += arg_scopeMove.x;

	//�㉺�J��������
	switch (m_verticalControl)
	{
		case ANGLE:
			m_nowParam.m_xAxisAngle -= arg_scopeMove.y * 0.3f;
			if (m_nowParam.m_xAxisAngle <= m_xAxisAngleMin)m_verticalControl = DIST;
			break;

		case DIST:
			m_nowParam.m_posOffsetZ += arg_scopeMove.y * 6.0f;
			if (m_nowParam.m_posOffsetZ <= m_posOffsetDepthMin)m_verticalControl = ANGLE;
			break;
	}

	//����l�����Ȃ��悤�ɂ���
	m_nowParam.m_posOffsetZ = std::clamp(m_nowParam.m_posOffsetZ, m_posOffsetDepthMin, m_posOffsetDepthMax);
	m_nowParam.m_xAxisAngle = std::clamp(m_nowParam.m_xAxisAngle, m_xAxisAngleMin, m_xAxisAngleMax);

	//���삷��J�����̃g�����X�t�H�[���i�O��ړ��j�X�V
	auto& transform = m_attachedCam.lock()->GetTransform();
	Vec3<float> localPos = { 0,0,0 };
	localPos.z = m_nowParam.m_posOffsetZ;
	localPos.y = m_gazePointOffset.y + tan(-m_nowParam.m_xAxisAngle) * m_nowParam.m_posOffsetZ;
	transform.SetPos(Math::Lerp(transform.GetPos(), localPos, m_camForwardPosLerpRate));
	transform.SetRotate(Vec3<float>::GetXAxis(), m_nowParam.m_xAxisAngle);

	//�R���g���[���[�̃g�����X�t�H�[���i�Ώۂ̎��́A���E�ړ��j�X�V
	m_camParentTransform.SetRotate(Vec3<float>::GetYAxis(), m_nowParam.m_yAxisAngle);
	m_camParentTransform.SetPos(Math::Lerp(m_camParentTransform.GetPos(), arg_targetPos, m_camFollowLerpRate));
}