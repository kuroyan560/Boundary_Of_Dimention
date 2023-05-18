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
		//m_nowParam = m_initializedParam;
	}

	//���݂̃p�����[�^�\��
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
	//����ΏۂƂȂ�J�����̃|�C���^��ێ�
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

	//�J�������A�^�b�`����Ă��Ȃ�
	if (m_attachedCam.expired())return;

	//�g�����X�t�H�[����ۑ��B
	m_oldCameraWorldPos = m_attachedCam.lock()->GetTransform().GetPos();

	//���̕ǂɋ���Ƃ��̒����_�ړ��̏ꍇ�AY����]�𓮂����B
	if (m_isHitUnderGroundTerrian && fabs(arg_targetPos.GetUp().y) < 0.9f) {

		//�ǂɂ������Ă��Ȃ��Ƃ��ɕۑ�������]�p�ƌ��݂̉�]�p�̍��������߂āA�K��l�ȏ㓮���Ȃ��悤�ɂ���B
		float subAngleY = arg_playerRotY - m_playerRotYStorage;

		//����l�𒴂��Ă�����B
		if (PLAYER_TARGET_MOVE_SIDE < fabs(subAngleY)) {

			//��]�ʂ������߂��B
			arg_playerRotY = m_playerRotYStorage + (signbit(subAngleY) ? -1.0f : 1.0f) * PLAYER_TARGET_MOVE_SIDE;

		}

	}

	//�J���������]���Ă��邩�̃t���O�ɂ���ď�x�N�g�������߂�B
	Vec3<float> upVec(0, 1, 0);
	if (arg_isCameraUpInverse) {
		upVec = Vec3<float>(0, -1, 0);
	}
	Vec3<float> rightVec = upVec.Cross(Vec3<float>(arg_targetPos.GetPos() - m_attachedCam.lock()->GetTransform().GetPos()).GetNormal());

	//�f�t�H���g����Y���̓������t�Ȃ̂Ŕ��]�B
	arg_scopeMove.y *= -1.0f;

	//�J�����̃g�����X�t�H�[���B
	auto& cameraTransform = m_attachedCam.lock()->GetTransform();

	//���͂���N�H�[�^�j�I������]������B
	KuroEngine::Quaternion scopeMoveQ = DirectX::XMQuaternionIdentity();
	scopeMoveQ = DirectX::XMQuaternionMultiply(scopeMoveQ, XMQuaternionRotationAxis(upVec, arg_scopeMove.x));
	if (0.1f < rightVec.Length())scopeMoveQ = DirectX::XMQuaternionMultiply(scopeMoveQ, XMQuaternionRotationAxis(rightVec, arg_scopeMove.y));

	//�J�����̈ʒu���N�H�[�^�j�I�����狁�߂�B
	Vec3<float> cameraDir = Vec3<float>(cameraTransform.GetPos() - arg_targetPos.GetPosWorld()).GetNormal();
	cameraDir = KuroEngine::Math::TransformVec3(cameraDir, scopeMoveQ);
	Vec3<float> cameraPos = arg_targetPos.GetPosWorld() + cameraDir * fabs(arg_cameraZ);

	//�J�����̍��W���ԁB
	cameraTransform.SetPos(cameraPos);






	//���݂̍��W����v���C���[�Ɍ�������]�����߂�B
	Vec3<float> axisZ = arg_targetPos.GetPos() - cameraTransform.GetPosWorld();
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
	m_cameraQ = DirectX::XMQuaternionMultiply(rotate, scopeMoveQ);

	//�J���������]����t���O�������Ă����甽�]������B
	if (arg_isCameraUpInverse) {
		m_cameraQ = DirectX::XMQuaternionMultiply(m_cameraQ, XMQuaternionRotationAxis(axisZ, DirectX::XM_PI));
	}

	cameraTransform.SetRotate(m_cameraQ);

	////�ʏ�̒n�`�𑖍�
	//m_isHitTerrian = false;
	//m_isHitUnderGroundTerrian = false;
	//for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	//{
	//	//���f�����擾
	//	auto model = terrian.GetModel().lock();

	//	//���b�V���𑖍�
	//	for (auto& modelMesh : model->m_meshes)
	//	{

	//		//�����蔻��Ɏg�p���郁�b�V��
	//		auto checkHitMesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];

	//		//���聫============================================


	//		//�����蔻������s
	//		CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_targetPos.GetPos(), checkHitRay.GetNormal(), checkHitMesh);

	//		if (output.m_isHit && 0 < output.m_distance && output.m_distance < fabs(arg_cameraZ)) {

	//			pushBackPos = output.m_pos + output.m_normal;
	//			m_isHitTerrian = true;

	//			//�v���C���[�̖@���Ɣ�ׂē�����������n��ɓ�����������ɂ���B
	//			float dot = output.m_normal.Dot(arg_targetPos.GetUp());
	//			if (0.9f < dot) {
	//				//�n��ɂ������Ă���B
	//				m_isHitUnderGroundTerrian = true;
	//			}

	//		}

	//		//=================================================
	//	}
	//}

	////�n��ɂ������Ă�����n�`�Ɖ����߂��O�̍��W����̉�]�����߂邱�ƂŁA�����_����Ɍ�����B
	//if (m_isHitUnderGroundTerrian) {

	//	//�J�����܂ł̃x�N�g���B
	//	KuroEngine::Vec3<float> cameraDir = (m_attachedCam.lock()->GetTransform().GetPos() - arg_targetPos.GetPosWorld()).GetNormal();

	//	//�J�������������ʉ�]������B

	//	//���̖ʂɂ���ꍇ
	//	if (0.9f < arg_targetPos.GetUp().y) {
	//		cameraDir = KuroEngine::Math::TransformVec3(cameraDir, DirectX::XMQuaternionRotationAxis(arg_targetPos.GetUp(), arg_scopeMove.x));
	//	}
	//	//��̖ʂɂ���ꍇ
	//	else if (arg_targetPos.GetUp().y < -0.9f) {
	//		cameraDir = KuroEngine::Math::TransformVec3(cameraDir, DirectX::XMQuaternionRotationAxis(arg_targetPos.GetUp(), -arg_scopeMove.x));
	//	}
	//	//���̖ʂɂ���ꍇ
	//	else if (0.9f < arg_targetPos.GetUp().y) {
	//		cameraDir = KuroEngine::Math::TransformVec3(cameraDir, DirectX::XMQuaternionRotationAxis(arg_targetPos.GetUp(), arg_scopeMove.y));
	//	}

	//	//���W�𓮂����B
	//	m_attachedCam.lock()->GetTransform().SetPos(arg_targetPos.GetPosWorld() + cameraDir * fabs(arg_cameraZ));

	//	//���݂̍��W����v���C���[�Ɍ�������]�����߂�B
	//	Vec3<float> axisZ = arg_targetPos.GetPos() - m_cameraLocalTransform.GetPosWorldByMatrix();
	//	axisZ.Normalize();

	//	//�v���C���[�̖@���Ƃ̊O�ς��牼��X�x�N�g���𓾂�B
	//	Vec3<float> axisX = Vec3<float>(0, 1, 0).Cross(axisZ);

	//	//X�x�N�g�������x�N�g���𓾂�B
	//	Vec3<float> axisY = axisZ.Cross(axisX);

	//	//�p���𓾂�B
	//	DirectX::XMMATRIX matWorld = DirectX::XMMatrixIdentity();
	//	matWorld.r[0] = { axisX.x, axisX.y, axisX.z, 0.0f };
	//	matWorld.r[1] = { axisY.x, axisY.y, axisY.z, 0.0f };
	//	matWorld.r[2] = { axisZ.x, axisZ.y, axisZ.z, 0.0f };

	//	XMVECTOR rotate, scale, position;
	//	DirectX::XMMatrixDecompose(&scale, &rotate, &position, matWorld);

	//	//��]�𔽓]������B
	//	if (arg_isCameraUpInverse) {
	//		m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, DirectX::XM_PI, 0.08f);
	//	}
	//	else {
	//		m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, 0.0f, 0.1f);
	//	}
	//	rotate = DirectX::XMQuaternionMultiply(rotate, DirectX::XMQuaternionRotationAxis(axisZ, m_rotateZ));

	//	rotate = DirectX::XMQuaternionNormalize(rotate);

	//	//��]��K�p�B
	//	m_attachedCam.lock()->GetTransform().SetRotate(rotate);

	//}
	//else {

	//	//��Ԃ���B
	//	m_attachedCam.lock()->GetTransform().SetPos(KuroEngine::Math::Lerp(m_attachedCam.lock()->GetTransform().GetPos(), pushBackPos, 0.3f));

	//	//���݂̍��W����v���C���[�Ɍ�������]�����߂�B
	//	Vec3<float> axisZ = arg_targetPos.GetPos() - m_attachedCam.lock()->GetTransform().GetPosWorld();
	//	axisZ.Normalize();

	//	//�v���C���[�̖@���Ƃ̊O�ς��牼��X�x�N�g���𓾂�B
	//	Vec3<float> axisX = Vec3<float>(0, 1, 0).Cross(axisZ);

	//	//X�x�N�g�������x�N�g���𓾂�B
	//	Vec3<float> axisY = axisZ.Cross(axisX);

	//	//�p���𓾂�B
	//	DirectX::XMMATRIX matWorld = DirectX::XMMatrixIdentity();
	//	matWorld.r[0] = { axisX.x, axisX.y, axisX.z, 0.0f };
	//	matWorld.r[1] = { axisY.x, axisY.y, axisY.z, 0.0f };
	//	matWorld.r[2] = { axisZ.x, axisZ.y, axisZ.z, 0.0f };

	//	XMVECTOR rotate, scale, position;
	//	DirectX::XMMatrixDecompose(&scale, &rotate, &position, matWorld);

	//	//��]�𔽓]������B
	//	if (arg_isCameraUpInverse) {
	//		m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, DirectX::XM_PI, 0.08f);
	//	}
	//	else {
	//		m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, 0.0f, 0.08f);
	//	}
	//	rotate = DirectX::XMQuaternionMultiply(rotate, DirectX::XMQuaternionRotationAxis(axisZ, m_rotateZ));

	//	rotate = DirectX::XMQuaternionNormalize(rotate);

	//	//��]��K�p�B
	//	m_attachedCam.lock()->GetTransform().SetRotate(rotate);

	//}

	//�n��ɂ������Ă���t���O��ۑ��B���ꂪtrue���ƒ����_���[�h�ɂȂ�̂ŁA�v���C���[������ʂɂ���ăJ�����̉�]��ł������B
	arg_isHitUnderGround = m_isHitUnderGroundTerrian;

}
