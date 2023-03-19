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
		ImGui::Text("DistnceToTarget : %.2f", m_nowParam.m_distToTarget);
		ImGui::Text("GazePoint_Height : %.2f", m_nowParam.m_gazePointHeight);
		ImGui::Text("GazePoint_Distance : %.2f", m_nowParam.m_gazePointDist);
		float degree = static_cast<float>(KuroEngine::Angle::ConvertToDegree(m_nowParam.m_xAngle));
		ImGui::Text("Angle_X : %.2f", degree);
		ImGui::EndChild();
	}
}

CameraController::CameraController()
	:KuroEngine::Debugger("CameraController", true, true)
{
	AddCustomParameter("Distance_To_Target", { "InitializedParameter","DistanceToTarget" }, PARAM_TYPE::FLOAT, &m_initializedParam.m_distToTarget, "InitializedParameter");
	AddCustomParameter("GazePoint_Height", { "InitializedParameter","GazePoint","Height" }, PARAM_TYPE::FLOAT, &m_initializedParam.m_gazePointHeight, "InitializedParameter");
	AddCustomParameter("GazePoint_Distance", { "InitializedParameter","GazePoint","Distance" }, PARAM_TYPE::FLOAT, &m_initializedParam.m_gazePointDist, "InitializedParameter");
	AddCustomParameter("Angle_X", { "InitializedParameter","Angle_X" }, PARAM_TYPE::FLOAT, &m_initializedParam.m_xAngle, "InitializedParameter");
	AddCustomParameter("PosLerpRate", { "PosLerpRate" }, PARAM_TYPE::FLOAT, &m_camPosLerpRate, "UpdateParameter");
	AddCustomParameter("GazePointLerpRate", { "GazePointLerpRate" }, PARAM_TYPE::FLOAT, &m_camGazePointLerpRate, "UpdateParameter");
}

void CameraController::Init()
{
	m_nowParam = m_initializedParam;
}

void CameraController::Update(std::shared_ptr<KuroEngine::Camera>arg_cam, KuroEngine::Vec3<float>arg_targetPos, KuroEngine::Vec3<float>arg_frontVec)
{
	using namespace KuroEngine;

	//XZ���ʏ�ł̑O�x�N�g��
	Vec3<float>frontOnXZ = arg_frontVec;
	frontOnXZ.y = 0.0f;

	//�J�����̈ʒu(XZ)���Z�o
	Vec3<float>newPos = arg_targetPos - (arg_frontVec * m_nowParam.m_distToTarget);

	//�����_�̈ʒu���Z�o
	Vec3<float>newGazePoint = newPos + (arg_frontVec * m_nowParam.m_gazePointDist);
	newGazePoint.y = arg_targetPos.y + m_nowParam.m_gazePointHeight;

	//�J�����̍������Z�o
	newPos.y = arg_targetPos.y + tan(m_nowParam.m_xAngle) * m_nowParam.m_gazePointDist;

	//�v�Z�����l��Lerp�ŋ߂Â�
	auto nowPos = arg_cam->GetPos();
	arg_cam->SetPos(Math::Lerp(nowPos, newPos, m_camPosLerpRate));
	auto nowGazePoint = arg_cam->GetGazePointPos();
	arg_cam->SetTarget(Math::Lerp(nowGazePoint, newGazePoint, m_camGazePointLerpRate));
}