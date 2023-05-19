#include "CameraController.h"
#include"Render/RenderObject/Camera.h"
#include"../../../../src/engine/ForUser/Object/Model.h"
#include"CollisionDetectionOfRayAndMesh.h"
#include"FrameWork/UsersInput.h"
#include"../Stage/StageManager.h"

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
	m_isHitUnderGroundTerrian = false;
}

void CameraController::Update(KuroEngine::Vec3<float>arg_scopeMove, KuroEngine::Transform arg_targetPos, float& arg_playerRotY, float arg_cameraZ, const std::weak_ptr<Stage>arg_nowStage, bool arg_isCameraUpInverse, bool arg_isCameraDefaultPos, bool& arg_isHitUnderGround, bool arg_isMovePlayer, bool arg_isPlayerJump)
{
	using namespace KuroEngine;

	//�J�������A�^�b�`����Ă��Ȃ�
	if (m_attachedCam.expired())return;

	//�g�����X�t�H�[����ۑ��B
	m_oldCameraWorldPos = m_attachedCam.lock()->GetTransform().GetPos();

	//��]��K�p����O��X��]
	float fromXAngle = m_nowParam.m_xAxisAngle;

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

	//�J�������n��ɓ������Ă���Ƃ��Ɋ��x��������B
	if (m_isHitUnderGroundTerrian) {
		arg_scopeMove *= 0.8f;
	}

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

	//�J�������n��ɂ������Ă��āA���̓������Ă���ʂ����̕ǂ��������]��ۑ����Ă����B
	if (!m_isHitUnderGroundTerrian) {
		m_playerRotYStorage = arg_playerRotY;
	}

	//����l�����Ȃ��悤�ɂ���
	m_nowParam.m_posOffsetZ = arg_cameraZ;
	m_nowParam.m_xAxisAngle = std::clamp(m_nowParam.m_xAxisAngle, m_xAxisAngleMin, m_xAxisAngleMax);

	//���ɃJ�������������Ă���Ƃ��Ƀv���C���[�������Ă�����A�����J�����ɓ�����Ȃ��悤��X����]�����ɖ߂��B
	if ((m_isHitUnderGroundTerrian && arg_isMovePlayer) || (m_isHitUnderGroundTerrian && arg_isCameraDefaultPos)) {

		//���̕ǂɂ����� 
		if (fabs(arg_targetPos.GetUp().y) < 0.9f && fabs(m_playerRotYLerp) < 0.1f) {

			//Y����]�������߂��ʁB
			float rotYScale = (arg_playerRotY - m_playerRotYStorage);
			//Y����]���҂����艟���߂��ƒn�ʃX���X���ɂȂ��Ă��܂��̂ŁA�����I�t�Z�b�g��݂��邱�Ƃŗǂ������̍����ɂ���B
			const float ROTY_OFFSET = 0.8f;
			float rotYOffset = (std::signbit(rotYScale) ? -1.0f : 1.0f) * ROTY_OFFSET + rotYScale;
			m_playerRotYLerp -= rotYOffset;

		}
		//�㉺�̕ǂɂ�����
		else if (0.9f < fabs(arg_targetPos.GetUp().y)) {

			//�J���������]���Ă��邩���Ă��Ȃ����ɂ���ē����l�����߂�B
			if (arg_isCameraUpInverse) {
				m_cameraXAngleLerpAmount = m_xAxisAngleMin;
			}
			else {
				m_cameraXAngleLerpAmount = m_xAxisAngleMax;
			}

		}

	}

	//�J�����������ʒu�ɖ߂����B
	if (arg_isCameraDefaultPos && !m_isHitUnderGroundTerrian) {

		//�J�����̐��ʃx�N�g��
		KuroEngine::Vec3<float> cameraDir = (arg_targetPos.GetPosWorld() - m_attachedCam.lock()->GetTransform().GetPos()).GetNormal();

		//�ڕW�n�_ ���������̎��_�ɂ��邽�߂ɁA�����_���������ɂ��炷�B
		KuroEngine::Vec3<float> targetPos = StageManager::Instance()->GetNowMapPingPos();

		//�ڕW�n�_�܂ł̃x�N�g��
		KuroEngine::Vec3<float> targetDir = (targetPos - m_attachedCam.lock()->GetTransform().GetPos()).GetNormal();

		//�e�x�N�g���Ԃ̖@�������߂�B�@�������݂��Ȃ��������Ԃ���K�v�͂Ȃ��B
		KuroEngine::Vec3<float> upVec = cameraDir.Cross(targetDir);
		if (0 < upVec.Length()) {

			//�J������̃v���C���[�̎p�������߂�B
			KuroEngine::Vec3<float> upVec(0, 1, 0);

			//�v���C���[�̖@���Ƃ̊O�ς���X�x�N�g���𓾂�B
			Vec3<float> axisX = upVec.Cross(cameraDir);

			//�v���C���[��Z���W
			Vec3<float> axisZ = axisX.Cross(upVec);

			//�܂���Y����]�����߂�B
			Vec2<float> cameraDir2DY = Project3Dto2D(cameraDir, axisX, axisZ);
			cameraDir2DY.Normalize();
			Vec2<float> targetDir2DY = Project3Dto2D(targetDir, axisX, axisZ);
			targetDir2DY.Normalize();

			//��]�ʂ����߂�B
			float angle = acos(cameraDir2DY.Dot(targetDir2DY));
			float cross = cameraDir2DY.Cross(targetDir2DY);
			m_rotateYLerpAmount = angle * (cross < 0 ? 1.0f : -1.0f);

			////����X������]�����߂�B
			//Vec2<float> cameraDir2DX = Project3Dto2D(cameraDir, axisX, axisZ);
			//cameraDir2DX.Normalize();
			//Vec2<float> targetDir2DX = Project3Dto2D(targetDir, axisX, axisZ);
			//targetDir2DX.Normalize();

			////��]�ʂ����߂�B
			//angle = acos(cameraDir2DX.Dot(targetDir2DX)) * 1.0f;
			//cross = cameraDir2DX.Cross(targetDir2DX);
			//m_cameraXAngleLerpAmount = angle * (cross < 0 ? -1.0f : 1.0f);

		}

	}

	//Y����ԗʂ��������炢�������ɕ�Ԃ���B
	if (0 < fabs(m_playerRotYLerp)) {
		float lerp = m_playerRotYLerp - KuroEngine::Math::Lerp(m_playerRotYLerp, 0.0f, 0.08f);
		m_playerRotYLerp -= lerp;
		arg_playerRotY += lerp;
		m_nowParam.m_yAxisAngle = arg_playerRotY;
	}

	//�J�����������ʒu�ɖ߂��ʂ�0�ȏゾ������J�����̉�]�ʂ��ԁB
	if (0 < fabs(m_rotateYLerpAmount)) {
		float lerp = m_rotateYLerpAmount - KuroEngine::Math::Lerp(m_rotateYLerpAmount, 0.0f, 0.08f);
		m_rotateYLerpAmount -= lerp;
		arg_playerRotY += lerp;
		m_nowParam.m_yAxisAngle = arg_playerRotY;
	}
	if (0 < fabs(m_cameraXAngleLerpAmount)) {
		float lerp = m_cameraXAngleLerpAmount - KuroEngine::Math::Lerp(m_cameraXAngleLerpAmount, 0.0f, 0.08f);
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

	//�����蔻��p�̃��C��ł��������߂�B
	Vec3<float> checkHitRay = m_cameraLocalTransform.GetPosWorldByMatrix() - m_oldCameraWorldPos;	//�܂��̓f�t�H���g�̃��C�ɐݒ�B

	//�ʏ�̒n�`�𑖍�
	m_isHitTerrian = false;
	m_isHitUnderGroundTerrian = false;
	auto& cameraTransform = m_attachedCam.lock()->GetTransform();
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


			//�����Ȓn�`�ƃ��C�̓����蔻������s
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(m_oldCameraWorldPos, checkHitRay.GetNormal(), checkHitMesh);
			if (output.m_isHit && 0 < output.m_distance && output.m_distance < checkHitRay.Length()) {

				pushBackPos = output.m_pos + output.m_normal;
				m_isHitTerrian = true;

				//�v���C���[�̖@���Ɣ�ׂē�����������n��ɓ�����������ɂ���B
				float dot = output.m_normal.Dot(arg_targetPos.GetUp());
				if (0.9f < dot) {

					//�n��ɂ������Ă���B
					m_isHitUnderGroundTerrian = true;

				}
				//�n��ɂ��Ȃ������牟���߂��B�������Ă���ʂɂ����X����Y���̂ǂ��炩�������߂��B
				else {

					//�������Ă���ǂ��㉺�������炾������B
					if (0.9f < std::fabs(output.m_normal.y)) {

						//���E������s���B
						Vec3<float> pushBackVec = KuroEngine::Vec3<float>(pushBackPos - arg_targetPos.GetPos()).GetNormal();

						//�v���C���[�̉E�x�N�g���ƌ��݂̃J�����x�N�g�����ׁA0�ȏゾ������E�B
						if (0 < arg_targetPos.GetRight().Dot(pushBackVec)) {

							//���݂�X�p�x�����߂�B
							Vec2<float> nowXVec = Project3Dto2D(KuroEngine::Vec3<float>(m_cameraLocalTransform.GetPosWorldByMatrix() - arg_targetPos.GetPos()).GetNormal(), arg_targetPos.GetFront(), arg_targetPos.GetRight());
							Vec2<float> pushBackVec = Project3Dto2D(KuroEngine::Vec3<float>(pushBackPos - arg_targetPos.GetPos()).GetNormal(), arg_targetPos.GetFront(), arg_targetPos.GetRight());

							//�p�x�̍�
							float nowAngle = atan2f(nowXVec.y, nowXVec.x);
							float pushBackAngle = atan2f(pushBackVec.y, pushBackVec.x);
							float divAngle = pushBackAngle - nowAngle;

							//�p�x�������߂��B
							m_nowParam.m_xAxisAngle += divAngle;

						}
						else {

							//���݂�X�p�x�����߂�B
							Vec2<float> nowXVec = Project3Dto2D(KuroEngine::Vec3<float>(m_cameraLocalTransform.GetPosWorldByMatrix() - arg_targetPos.GetPos()).GetNormal(), arg_targetPos.GetRight(), arg_targetPos.GetFront());
							Vec2<float> pushBackVec = Project3Dto2D(KuroEngine::Vec3<float>(pushBackPos - arg_targetPos.GetPos()).GetNormal(), arg_targetPos.GetRight(), arg_targetPos.GetFront());

							//�p�x�̍�
							float nowAngle = atan2f(nowXVec.y, nowXVec.x);
							float pushBackAngle = atan2f(pushBackVec.y, pushBackVec.x);
							float divAngle = pushBackAngle - nowAngle;

							//�p�x�������߂��B
							m_nowParam.m_xAxisAngle += divAngle;

						}

					}
					else {

						//���E������s���B
						Vec3<float> pushBackVec = KuroEngine::Vec3<float>(pushBackPos - arg_targetPos.GetPos()).GetNormal();
						
						//�v���C���[�̉E�x�N�g���ƌ��݂̃J�����x�N�g�����ׁA0�ȏゾ������E�B
						if (0 < arg_targetPos.GetRight().Dot(pushBackVec)) {

							//���݂�Y�p�x�����߂�B
							Vec2<float> nowYVec = Project3Dto2D(KuroEngine::Vec3<float>(m_cameraLocalTransform.GetPosWorldByMatrix() - arg_targetPos.GetPos()).GetNormal(), arg_targetPos.GetFront(), arg_targetPos.GetUp());
							Vec2<float> pushBackVec = Project3Dto2D(KuroEngine::Vec3<float>(pushBackPos - arg_targetPos.GetPos()).GetNormal(), arg_targetPos.GetFront(), arg_targetPos.GetUp());


							//�p�x�̍�
							float nowAngle = atan2f(nowYVec.y, nowYVec.x);
							float pushBackAngle = atan2f(pushBackVec.y, pushBackVec.x);
							float divAngle = pushBackAngle - nowAngle;

							//�p�x�������߂��B
							m_nowParam.m_yAxisAngle += divAngle;
							arg_playerRotY += divAngle;

						}
						else {

							//���݂�Y�p�x�����߂�B
							Vec2<float> nowYVec = Project3Dto2D(KuroEngine::Vec3<float>(m_cameraLocalTransform.GetPosWorldByMatrix() - arg_targetPos.GetPos()).GetNormal(), arg_targetPos.GetUp(), arg_targetPos.GetFront());
							Vec2<float> pushBackVec = Project3Dto2D(KuroEngine::Vec3<float>(pushBackPos - arg_targetPos.GetPos()).GetNormal(), arg_targetPos.GetUp(), arg_targetPos.GetFront());


							//�p�x�̍�
							float nowAngle = atan2f(nowYVec.y, nowYVec.x);
							float pushBackAngle = atan2f(pushBackVec.y, pushBackVec.x);
							float divAngle = pushBackAngle - nowAngle;

							//�p�x�������߂��B
							m_nowParam.m_yAxisAngle += divAngle;
							arg_playerRotY += divAngle;

						}

					}

				}

			}

			//�v���C���[�̂��郁�b�V�����H
			bool onPlayer = 0.9f < output.m_normal.Dot(arg_targetPos.GetUp());

			//�v���C���[���炿����Ƃ��炵���ʒu������n�`�փ��C���Ƃ΂��A�J�����̉�]�������߂��������s���B
			const float PUSH_BACK_ANGLE_X = 0.5f;
			const float PUSH_BACK_ANGLE_Y = 0.5f;

			if (arg_isMovePlayer && !onPlayer && !arg_isPlayerJump) {


				//�R�̎��^�̒ʘH�΍�̂��߂ɁA������Ƀ��C���΂��B
				output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(m_attachedCam.lock()->GetTransform().GetPos(), { 0,arg_isCameraUpInverse ? -1.0f : 1.0f,0 }, checkHitMesh);

				const float CHECK_HIT_TOP = 5.0f;
				if (output.m_isHit && 0 < output.m_distance && output.m_distance < fabs(CHECK_HIT_TOP)) {

					m_cameraXAngleLerpAmount = -PUSH_BACK_ANGLE_X * (arg_isCameraUpInverse ? -1.0f : 1.0f);

				}


				//�R�̎��^�̒ʘH�΍�̂��߂ɁA�������Ƀ��C���΂��B
				output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(m_attachedCam.lock()->GetTransform().GetPos(), { 0,arg_isCameraUpInverse ? 1.0f : -1.0f,0 }, checkHitMesh);

				if (output.m_isHit && 0 < output.m_distance && output.m_distance < fabs(CHECK_HIT_TOP)) {

					m_cameraXAngleLerpAmount = +PUSH_BACK_ANGLE_X * (arg_isCameraUpInverse ? -1.0f : 1.0f);

				}

				//�R�̎��^�̒ʘH�΍�̂��߂ɁA�E�����Ƀ��C���΂��B
				output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(m_attachedCam.lock()->GetTransform().GetPos(), m_attachedCam.lock()->GetTransform().GetRight(), checkHitMesh);

				if (output.m_isHit && 0 < output.m_distance && output.m_distance < fabs(CHECK_HIT_TOP)) {

					m_rotateYLerpAmount = +PUSH_BACK_ANGLE_Y * (arg_isCameraUpInverse ? -1.0f : 1.0f);

				}


				//�R�̎��^�̒ʘH�΍�̂��߂ɁA�������Ƀ��C���΂��B
				output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(m_attachedCam.lock()->GetTransform().GetPos(), -m_attachedCam.lock()->GetTransform().GetRight(), checkHitMesh);

				if (output.m_isHit && 0 < output.m_distance && output.m_distance < fabs(CHECK_HIT_TOP)) {

					m_rotateYLerpAmount = -PUSH_BACK_ANGLE_Y * (arg_isCameraUpInverse ? -1.0f : 1.0f);

				}

			}

			//=================================================
		}
	}

	//�n��ɂ������Ă�����n�`�Ɖ����߂��O�̍��W����̉�]�����߂邱�ƂŁA�����_����Ɍ�����B
	if (m_isHitUnderGroundTerrian) {

		//�J�����܂ł̃x�N�g���B
		KuroEngine::Vec3<float> cameraDir = (m_attachedCam.lock()->GetTransform().GetPos() - arg_targetPos.GetPosWorld()).GetNormal();

		//�J�������������ʉ�]������B

		//���̖ʂɂ���ꍇ
		if (0.9f < arg_targetPos.GetUp().y) {
			cameraDir = KuroEngine::Math::TransformVec3(cameraDir, DirectX::XMQuaternionRotationAxis(arg_targetPos.GetUp(), arg_scopeMove.x));
		}
		//��̖ʂɂ���ꍇ
		else if (arg_targetPos.GetUp().y < -0.9f) {
			cameraDir = KuroEngine::Math::TransformVec3(cameraDir, DirectX::XMQuaternionRotationAxis(arg_targetPos.GetUp(), -arg_scopeMove.x));
		}
		//���̖ʂɂ���ꍇ
		else if (0.9f < arg_targetPos.GetUp().y) {
			cameraDir = KuroEngine::Math::TransformVec3(cameraDir, DirectX::XMQuaternionRotationAxis(arg_targetPos.GetUp(), arg_scopeMove.y));
		}

		//���W�𓮂����B
		m_attachedCam.lock()->GetTransform().SetPos(arg_targetPos.GetPosWorld() + cameraDir * fabs(arg_cameraZ));

		//���݂̍��W����v���C���[�Ɍ�������]�����߂�B
		Vec3<float> axisZ = arg_targetPos.GetPos() - m_cameraLocalTransform.GetPosWorldByMatrix();
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
			m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, DirectX::XM_PI, 0.08f);
		}
		else {
			m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, 0.0f, 0.1f);
		}
		rotate = DirectX::XMQuaternionMultiply(rotate, DirectX::XMQuaternionRotationAxis(axisZ, m_rotateZ));

		rotate = DirectX::XMQuaternionNormalize(rotate);

		//��]��K�p�B
		m_attachedCam.lock()->GetTransform().SetRotate(rotate);

	}
	else {

		//��Ԃ���B
		m_attachedCam.lock()->GetTransform().SetPos(KuroEngine::Math::Lerp(m_attachedCam.lock()->GetTransform().GetPos(), pushBackPos, 0.3f));

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
			m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, DirectX::XM_PI, 0.08f);
		}
		else {
			m_rotateZ = KuroEngine::Math::Lerp(m_rotateZ, 0.0f, 0.08f);
		}
		rotate = DirectX::XMQuaternionMultiply(rotate, DirectX::XMQuaternionRotationAxis(axisZ, m_rotateZ));

		rotate = DirectX::XMQuaternionNormalize(rotate);

		//��]��K�p�B
		m_attachedCam.lock()->GetTransform().SetRotate(rotate);

	}

	//�n��ɂ������Ă���t���O��ۑ��B���ꂪtrue���ƒ����_���[�h�ɂȂ�̂ŁA�v���C���[������ʂɂ���ăJ�����̉�]��ł������B
	arg_isHitUnderGround = m_isHitUnderGroundTerrian;

	//���������߂�B
	//const float PUSHBACK = 20.0f;
	//float distance = KuroEngine::Vec3<float>(m_attachedCam.lock()->GetTransform().GetPos() - arg_targetPos.GetPos()).Length();
	//if (distance <= PUSHBACK) {
	//	float pushBackDistance = PUSHBACK - distance;
	//	m_attachedCam.lock()->GetTransform().SetPos(m_attachedCam.lock()->GetTransform().GetPos() + KuroEngine::Vec3<float>(m_attachedCam.lock()->GetTransform().GetPos() - arg_targetPos.GetPos()).GetNormal() * pushBackDistance);
	//}

}
