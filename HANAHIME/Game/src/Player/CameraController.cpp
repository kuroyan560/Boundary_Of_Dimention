#include "CameraController.h"
#include"Render/RenderObject/Camera.h"
#include"../../../../src/engine/ForUser/Object/Model.h"
#include"CollisionDetectionOfRayAndMesh.h"
#include"FrameWork/UsersInput.h"

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
	//����ΏۂƂȂ�J�����̃|�C���^��ێ�
	m_attachedCam = arg_cam;
	//�R���g���[���[�̃g�����X�t�H�[����e�Ƃ��Đݒ�
	m_cameraLocalTransform.SetParent(&m_camParentTransform);
}

void CameraController::Init()
{
	m_nowParam = m_initializedParam;
	m_verticalControl = ANGLE;
	m_rotateZ = 0;
	m_rotateYLerpAmount = 0;
	m_cameraXAngleLerpAmount = 0;
}

void CameraController::Update(KuroEngine::Vec3<float>arg_scopeMove, KuroEngine::Transform arg_targetPos, float& arg_playerRotY, float arg_cameraZ, const std::weak_ptr<Stage>arg_nowStage, bool arg_isCameraUpInverse, bool arg_isCameraDefaultPos)
{
	using namespace KuroEngine;

	//�J�������A�^�b�`����Ă��Ȃ�
	if (m_attachedCam.expired())return;

	//�g�����X�t�H�[����ۑ��B
	m_oldCameraWorldPos = m_attachedCam.lock()->GetTransform().GetPos();

	//���E�J��������
	m_nowParam.m_yAxisAngle = arg_playerRotY;

	//�㉺�J��������
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

	//����l�����Ȃ��悤�ɂ���
	m_nowParam.m_posOffsetZ = arg_cameraZ;
	m_nowParam.m_xAxisAngle = std::clamp(m_nowParam.m_xAxisAngle, m_xAxisAngleMin, m_xAxisAngleMax);
	
	//�J�����������ʒu�ɖ߂����B
	if (arg_isCameraDefaultPos) {

		//�J���������]���Ă��邩���Ă��Ȃ����ɂ���ē����l�����߂�B
		if (arg_isCameraUpInverse) {
			m_cameraXAngleLerpAmount = m_xAxisAngleMin - m_nowParam.m_xAxisAngle;
		}
		else {
			m_cameraXAngleLerpAmount = m_xAxisAngleMax - m_nowParam.m_xAxisAngle;
		}

		//�n�`�ɓ������Ă�����
		if (m_isHitTerrian) {
			m_rotateYLerpAmount += DirectX::XM_PI;
		}

	}

	//�J�����������ʒu�ɖ߂��ʂ�0�ȏゾ������J�����̉�]�ʂ��ԁB
	if (0 < m_rotateYLerpAmount) {
		float lerp = m_rotateYLerpAmount - KuroEngine::Math::Lerp(m_rotateYLerpAmount, 0.0f, 0.2f);
		m_rotateYLerpAmount -= lerp;
		arg_playerRotY += lerp;
	}
	if (0 < m_cameraXAngleLerpAmount) {
		float lerp = m_cameraXAngleLerpAmount - KuroEngine::Math::Lerp(m_cameraXAngleLerpAmount, 0.0f, 0.2f);
		m_cameraXAngleLerpAmount = (fabs(m_cameraXAngleLerpAmount) - fabs(lerp)) * (signbit(m_cameraXAngleLerpAmount) ? -1.0f : 1.0f);
		m_nowParam.m_xAxisAngle += lerp;
	}

	//���삷��J�����̃g�����X�t�H�[���i�O��ړ��j�X�V
	Vec3<float> localPos = { 0,0,0 };
	localPos.z = m_nowParam.m_posOffsetZ;
	localPos.y = m_gazePointOffset.y + tan(-m_nowParam.m_xAxisAngle) * m_nowParam.m_posOffsetZ;
	m_cameraLocalTransform.SetPos(Math::Lerp(m_cameraLocalTransform.GetPos(), localPos, m_camForwardPosLerpRate));
	m_cameraLocalTransform.SetRotate(Vec3<float>::GetXAxis(), m_nowParam.m_xAxisAngle);

	//�R���g���[���[�̃g�����X�t�H�[���i�Ώۂ̎��́A���E�ړ��j�X�V
	m_camParentTransform.SetRotate(Vec3<float>::GetYAxis(), m_nowParam.m_yAxisAngle);
	m_camParentTransform.SetPos(Math::Lerp(m_camParentTransform.GetPos(), arg_targetPos.GetPosWorld(), m_camFollowLerpRate));


	//�g�p����J�����̍��W���Ԃ��ēK�p�B
	Vec3<float> pushBackPos = m_cameraLocalTransform.GetPosWorldByMatrix();

	//�ʏ�̒n�`�𑖍�
	m_isHitTerrian = false;
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//���f�����擾
		auto model = terrian.GetModel().lock();

		//���b�V���𑖍�
		for (auto& modelMesh : model->m_meshes)
		{

			//�����蔻��Ɏg�p���郁�b�V��
			auto checkHitMesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//���聫============================================


			//�����蔻������s
			auto moveVec = m_cameraLocalTransform.GetPosWorldByMatrix() - arg_targetPos.GetPos();
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_targetPos.GetPos(), moveVec.GetNormal(), checkHitMesh);

			if (output.m_isHit && 0 < output.m_distance && output.m_distance < moveVec.Length()) {

				pushBackPos = output.m_pos + output.m_normal;
				m_isHitTerrian = true;

			}

			//=================================================
		}
	}

	//��Ԃ���B
	m_attachedCam.lock()->GetTransform().SetPos(KuroEngine::Math::Lerp(m_attachedCam.lock()->GetTransform().GetPos(), pushBackPos, 0.3f));

	//���������߂�B
	const float PUSHBACK = 20.0f;
	float distance = KuroEngine::Vec3<float>(m_attachedCam.lock()->GetTransform().GetPos() - arg_targetPos.GetPos()).Length();
	if (distance <= PUSHBACK) {
		float pushBackDistance = PUSHBACK - distance;
		m_attachedCam.lock()->GetTransform().SetPos(m_attachedCam.lock()->GetTransform().GetPos() + KuroEngine::Vec3<float>(m_attachedCam.lock()->GetTransform().GetPos() - arg_targetPos.GetPos()).GetNormal() * pushBackDistance);
	}

	//���݂̍��W����v���C���[�Ɍ�������]�����߂�B
	Vec3<float> axisZ = arg_targetPos.GetPos() - m_attachedCam.lock()->GetTransform().GetPosWorld();
	axisZ.Normalize();

	//�v���C���[�̖@���Ƃ̊O�ς��牼��X�x�N�g���𓾂�B
	Vec3<float> axisX = Vec3<float>(0, 1, 0).Cross(axisZ);

	//X�x�N�g�������x�N�g���𓾂�B
	Vec3<float> axisY = axisZ.Cross(axisX);

	//�p���𓾂�B
	DirectX::XMMATRIX matWorld = DirectX::XMMatrixIdentity();
	matWorld.r[0] = { axisX.x, axisX.y, axisX.z, 0.0f };
	matWorld.r[1] = { axisY.x, axisY.y, axisY.z, 0.0f };
	matWorld.r[2] = { axisZ.x, axisZ.y, axisZ.z, 0.0f };

	XMVECTOR rotate, scale, position;
	DirectX::XMMatrixDecompose(&scale, &rotate, &position, matWorld);

	//��]�𔽓]������B
	if (arg_isCameraUpInverse) {
		m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, DirectX::XM_PI, 0.2f);
	}
	else {
		m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, 0.0f, 0.1f);
	}
	rotate = DirectX::XMQuaternionMultiply(rotate, DirectX::XMQuaternionRotationAxis(axisZ, m_rotateZ));

	rotate = DirectX::XMQuaternionNormalize(rotate);

	//��]��K�p�B
	m_attachedCam.lock()->GetTransform().SetRotate(rotate);


	//m_attachedCam.lock()->GetTransform() = m_cameraLocalTransform;

}
