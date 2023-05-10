#include "CameraController.h"
#include"Render/RenderObject/Camera.h"
#include"../../../../src/engine/ForUser/Object/Model.h"
#include"CollisionDetectionOfRayAndMesh.h"

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
	AddCustomParameter("xAxisAngleMin", { "xAxisAngle","min" }, PARAM_TYPE::FLOAT, &m_xAxisAngleMin, "UpdateParameter");
	AddCustomParameter("xAxisAngleMax", { "xAxisAngle","max" }, PARAM_TYPE::FLOAT, &m_xAxisAngleMax, "UpdateParameter");
	AddCustomParameter("camFowardPosLerpRate", { "PosLerpRate" }, PARAM_TYPE::FLOAT, &m_camForwardPosLerpRate, "UpdateParameter");
	AddCustomParameter("camFollowLerpRate", { "FollowLerpRate" }, PARAM_TYPE::FLOAT, &m_camFollowLerpRate, "UpdateParameter");

	LoadParameterLog();
}

void CameraController::AttachCamera(std::shared_ptr<KuroEngine::Camera> arg_cam)
{
	//����ΏۂƂȂ�J�����̃|�C���^��ێ�
	m_attachedCam = arg_cam;
	//�R���g���[���[�̃g�����X�t�H�[����e�Ƃ��Đݒ�
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

	//�J�������A�^�b�`����Ă��Ȃ�
	if (m_attachedCam.expired())return;

	//�g�����X�t�H�[����ۑ��B
	m_attachedCam.lock()->GetTransform() = arg_playerTransform;

	//�w��ɂ��炷�x�N�g��
	const float Z_HEIGHT_OFFSET = 0.3f;
	KuroEngine::Vec3<float> zOffsetVec = -arg_playerTransform.GetFront() + arg_playerTransform.GetUp() * Z_HEIGHT_OFFSET;
	KuroEngine::Vec3<float> zOffset = zOffsetVec.GetNormal() * arg_cameraZ;

	//�w��ɂ��炷�B
	m_attachedCam.lock()->GetTransform().SetPos(arg_playerTransform.GetPos() + zOffset);

	//�v���C���[�̂�����Ə��������������B
	const float UP_OFFSET = 3.0f;
	Vec3<float> playerDir = arg_playerTransform.GetPos() + (arg_playerTransform.GetUp() * UP_OFFSET) - m_attachedCam.lock()->GetTransform().GetPos();
	playerDir.Normalize();

	//��]����������B
	Vec3<float> defVec = arg_playerTransform.GetFront();
	defVec = defVec * (playerDir.Dot(defVec) / defVec.Dot(defVec));

	//��]���Ɗp�x�����߂�B
	Vec3<float> axis = defVec.Cross(playerDir);
	float angle = acosf(defVec.Dot(playerDir));

	//��]������������B
	if (0.1f < axis.Length()) {

		auto q = DirectX::XMQuaternionRotationAxis(axis, angle);
		m_attachedCam.lock()->GetTransform().SetRotate(DirectX::XMQuaternionMultiply(m_attachedCam.lock()->GetTransform().GetRotate(), q));

	}





	//oldTransform = m_attachedCam.lock()->GetTransform();

	////���E�J��������
	//m_nowParam.m_yAxisAngle = arg_playerRotY;

	////�㉺�J��������
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

	////����l�����Ȃ��悤�ɂ���
	//m_nowParam.m_posOffsetZ = arg_cameraZ;
	//m_nowParam.m_xAxisAngle = std::clamp(m_nowParam.m_xAxisAngle, m_xAxisAngleMin, m_xAxisAngleMax);


	////���삷��J�����̃g�����X�t�H�[���i�O��ړ��j�X�V
	//Vec3<float> localPos = { 0,0,0 };
	//localPos.z = m_nowParam.m_posOffsetZ;
	//localPos.y = m_gazePointOffset.y + tan(-m_nowParam.m_xAxisAngle) * m_nowParam.m_posOffsetZ;
	//m_attachedCam.lock()->GetTransform().SetPos(Math::Lerp(m_attachedCam.lock()->GetTransform().GetPos(), localPos, m_camForwardPosLerpRate));
	//m_attachedCam.lock()->GetTransform().SetRotate(Vec3<float>::GetXAxis(), m_nowParam.m_xAxisAngle);

	////�R���g���[���[�̃g�����X�t�H�[���i�Ώۂ̎��́A���E�ړ��j�X�V
	//m_camParentTransform.SetRotate(Vec3<float>::GetYAxis(), m_nowParam.m_yAxisAngle);
	//m_camParentTransform.SetPos(Math::Lerp(m_camParentTransform.GetPos(), arg_targetPos, m_camFollowLerpRate));






	////�g�p����J�����ɉ�]��K�p�B
	//XMVECTOR rotate, translation, scaling;
	//DirectX::XMMatrixDecompose(&scaling, &rotate, &translation, m_cameraLocalTransform.GetMatWorld());

	//rotate = DirectX::XMQuaternionNormalize(rotate);

	//auto local = m_cameraLocalTransform.GetRotate();
	//auto world = m_cameraLocalTransform.GetRotateWorld();
	//m_attachedCam.lock()->GetTransform().SetRotate({ rotate.m128_f32[0], rotate .m128_f32[1], rotate .m128_f32[2]});

	//auto localScale = m_cameraLocalTransform.GetScaleWorld();
	//auto worldScale = m_cameraLocalTransform.GetScaleWorld();

	//auto eye = m_attachedCam.lock()->GetEye();

	////�g�p����J�����̍��W���Ԃ��ēK�p�B
	//m_attachedCam.lock()->GetTransform().SetPos({ translation.m128_f32[0], translation.m128_f32[1], translation.m128_f32[2] });
	//m_attachedCam.lock()->GetTransform().SetScale({ scaling.m128_f32[0], scaling.m128_f32[1], scaling.m128_f32[2] });

	//m_attachedCam.lock()->GetTransform() = m_cameraLocalTransform;

	//int a = 0;

}

void CameraController::TerrianMeshCollision(const std::weak_ptr<Stage> arg_nowStage)
{

	//�ʏ�̒n�`�𑖍�
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
