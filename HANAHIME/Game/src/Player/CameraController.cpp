#include"CameraController.h"
#include"Render/RenderObject/Camera.h"
#include"../../../../src/engine/ForUser/Object/Model.h"
#include"CollisionDetectionOfRayAndMesh.h"
#include"FrameWork/UsersInput.h"
#include"../Stage/StageManager.h"
#include"../Stage/CheckPointHitFlag.h"

void CameraController::OnImguiItems()
{
	ImGui::Text("NowParameter");
	ImGui::SameLine();

	//�p�����[�^�������{�^��
	if (ImGui::Button("Initialize"))
	{
		m_nowParam = m_initializedParam;
		m_checkPointTriggerParam = m_initializedParam;
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

void CameraController::Init(bool arg_isRespawn)
{
	if (arg_isRespawn) {
		m_nowParam = m_checkPointTriggerParam;
		m_rotateZ = m_checkPointCameraZ;
	}
	else {
		m_nowParam = m_initializedParam;
	}
	m_verticalControl = ANGLE;
	m_rotateYLerpAmount = 0;
	m_cameraXAngleLerpAmount = 0;
	m_playerOldPos = KuroEngine::Vec3<float>();
	m_isOldFrontWall = false;
	m_isCameraModeLookAround = false;
	m_isLookAroundFinish = false;
	m_isLookAroundFinishComplete = false;
}

void CameraController::Update(KuroEngine::Vec3<float>arg_scopeMove, KuroEngine::Transform arg_targetPos, float& arg_playerRotY, float arg_cameraZ, const std::weak_ptr<Stage>arg_nowStage, bool arg_isCameraUpInverse, bool arg_isCameraDefaultPos, bool& arg_isHitUnderGround, bool arg_isMovePlayer, bool arg_isPlayerJump, KuroEngine::Quaternion arg_cameraQ, bool arg_isFrontWall, KuroEngine::Transform arg_drawTransform, KuroEngine::Vec3<float> arg_frontWallNormal, bool arg_isNoCollision, bool arg_isLookAroundMode, std::vector<HIT_POINT> arg_hitPointData)
{
	using namespace KuroEngine;

	//�`�F�b�N�|�C���g�ɓ��B���Ă�����p�����[�^�[��ۑ��B
	if (CheckPointHitFlag::Instance()->m_isHitCheckPointTrigger) {

		m_checkPointTriggerParam = m_nowParam;
		m_checkPointCameraZ = m_rotateZ;

	}

	//�J�������A�^�b�`����Ă��Ȃ�
	if (m_attachedCam.expired())return;

	//�v���C���[�����͂����郂�[�h��������
	if (arg_isLookAroundMode) {

		UpdateLookAround(arg_scopeMove, arg_targetPos, arg_playerRotY, arg_cameraZ, arg_nowStage, arg_isCameraUpInverse, arg_isCameraDefaultPos, arg_isHitUnderGround, arg_isMovePlayer, arg_isPlayerJump, arg_cameraQ, arg_isFrontWall, arg_drawTransform, arg_frontWallNormal, arg_isNoCollision, arg_isLookAroundMode);

		return;

	}
	m_isCameraModeLookAround = arg_isLookAroundMode;

	//�v���C���[�̍��W�����[�v
	m_playerLerpPos = KuroEngine::Math::Lerp(m_playerLerpPos, arg_targetPos.GetPos(), 0.4f);

	//�g�����X�t�H�[����ۑ��B
	m_oldCameraWorldPos = m_attachedCam.lock()->GetTransform().GetPos();

	//Y����ԗʂ��������炢�������ɕ�Ԃ���B
	if (0 < fabs(m_playerRotYLerp)) {
		float lerp = m_playerRotYLerp - KuroEngine::Math::Lerp(m_playerRotYLerp, 0.0f, 0.08f);
		m_playerRotYLerp -= lerp;
		arg_playerRotY += lerp;
		m_nowParam.m_yAxisAngle = arg_playerRotY;
	}

	//�J�����������ʒu�ɖ߂��ʂ�0�ȏゾ������J�����̉�]�ʂ��ԁB
	if (0 < fabs(m_rotateYLerpAmount)) {
		float lerpAmount = arg_isCameraUpInverse ? 0.16f : 0.08f;
		float lerp = m_rotateYLerpAmount - KuroEngine::Math::Lerp(m_rotateYLerpAmount, 0.0f, lerpAmount);
		m_rotateYLerpAmount -= lerp;
		arg_playerRotY += lerp;
		m_nowParam.m_yAxisAngle = arg_playerRotY;
	}
	if (0 < fabs(m_cameraXAngleLerpAmount)) {
		float lerpAmount = arg_isCameraUpInverse ? 0.16f : 0.08f;
		float lerp = m_cameraXAngleLerpAmount - KuroEngine::Math::Lerp(m_cameraXAngleLerpAmount, 0.0f, lerpAmount);
		m_cameraXAngleLerpAmount = (fabs(m_cameraXAngleLerpAmount) - fabs(lerp)) * (signbit(m_cameraXAngleLerpAmount) ? -1.0f : 1.0f);
		m_nowParam.m_xAxisAngle += lerp;
	}

	//��]��K�p����O��X��]
	float fromXAngle = m_nowParam.m_xAxisAngle;
	float fromYAngle = m_nowParam.m_yAxisAngle;

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

	//�v���C���[���ʂɕǂ��������炻�����̕�������������B
	if ((!m_isOldFrontWall && arg_isFrontWall) && arg_isMovePlayer && !arg_isPlayerJump) {

		//�v���C���[�������Ă�������B
		Vec3<float> front = arg_drawTransform.GetFront();

		const float SCALE = 1.5f;

		//�J���������]���Ă�����
		if (arg_isCameraUpInverse) {

			//�v���C���[��������������Ă�����B
			float dot = front.Dot(Vec3<float>(0, -1, 0));
			if (0.5f < dot) {

				m_cameraXAngleLerpAmount += m_xAxisAngleMax * SCALE;

			}

			//�v���C���[���������������Ă�����B
			dot = front.Dot(Vec3<float>(0, 1, 0));
			if (0.5f < dot) {

				m_cameraXAngleLerpAmount += m_xAxisAngleMin * SCALE;

			}

		}
		//���]���Ă��Ȃ�������
		else {


			//�v���C���[��������������Ă�����B
			float dot = front.Dot(Vec3<float>(0, 1, 0));
			if (0.5f < dot) {

				m_cameraXAngleLerpAmount += m_xAxisAngleMin * SCALE;

			}

			//�v���C���[���������������Ă�����B
			dot = front.Dot(Vec3<float>(0, -1, 0));
			if (0.5f < dot) {

				m_cameraXAngleLerpAmount += m_xAxisAngleMax * SCALE;

			}

		}

		JumpStart(arg_targetPos, arg_frontWallNormal, arg_isCameraUpInverse, 0.5f);

	}


	//�v���C���[�̈ړ��ɉ����ăJ�������Ԃ���B
	PlayerMoveCameraLerp(arg_scopeMove, arg_targetPos, arg_playerRotY, arg_cameraZ, arg_nowStage, arg_isCameraUpInverse, arg_isCameraDefaultPos, arg_isHitUnderGround, arg_isMovePlayer, arg_isPlayerJump, arg_cameraQ);



	//�t���O��ۑ����Ă����B
	m_isOldFrontWall = arg_isFrontWall;

	//�}�b�v�s���̍��W�̎󂯎M
	KuroEngine::Vec3<float>mapPinPos;
	//�J�����������ʒu�ɖ߂����B
	if (arg_isCameraDefaultPos && StageManager::Instance()->GetNowMapPinPos(&mapPinPos)) {

		//�J�����̐��ʃx�N�g��
		KuroEngine::Vec3<float> cameraDir = (arg_targetPos.GetPosWorld() - m_attachedCam.lock()->GetTransform().GetPos()).GetNormal();
		//�ڕW�n�_ ���������̎��_�ɂ��邽�߂ɁA�����_���������ɂ��炷�B
		KuroEngine::Vec3<float> targetPos = mapPinPos;

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

		}


	}

	//���삷��J�����̃g�����X�t�H�[���i�O��ړ��j�X�V
	Vec3<float> localPos = { 0,0,0 };
	localPos.z = m_nowParam.m_posOffsetZ;
	localPos.y = m_gazePointOffset.y + tan(-m_nowParam.m_xAxisAngle) * m_nowParam.m_posOffsetZ;
	m_cameraLocalTransform.SetPos(Math::Lerp(m_cameraLocalTransform.GetPos(), localPos, m_camForwardPosLerpRate));
	m_cameraLocalTransform.SetRotate(Vec3<float>::GetXAxis(), m_nowParam.m_xAxisAngle);

	//�R���g���[���[�̃g�����X�t�H�[���i�Ώۂ̎��́A���E�ړ��j�X�V
	m_camParentTransform.SetRotate(Vec3<float>::GetYAxis(), m_nowParam.m_yAxisAngle);
	m_camParentTransform.SetPos(Math::Lerp(m_camParentTransform.GetPos(), m_playerLerpPos, m_camFollowLerpRate));


	//�g�p����J�����̍��W���Ԃ��ēK�p�B
	Vec3<float> pushBackPos = m_cameraLocalTransform.GetPosWorldByMatrix();
	Vec3<float> playerDir = m_playerLerpPos - m_cameraLocalTransform.GetPosWorldByMatrix();

	//�J�����̉����߂�����Ɏg�p����p�����擾�B
	Vec3<float> cameraAxisZ = playerDir.GetNormal();
	Vec3<float> cameraAxisY = arg_isCameraUpInverse ? Vec3<float>(0, -1, 0) : Vec3<float>(0, 1, 0);
	Vec3<float> cameraAxisX = cameraAxisY.Cross(cameraAxisZ);
	cameraAxisZ = cameraAxisX.Cross(cameraAxisY);
	DirectX::XMMATRIX cameraMatWorld = DirectX::XMMatrixIdentity();
	cameraMatWorld.r[0] = { cameraAxisX.x, cameraAxisX.y, cameraAxisX.z, 0.0f };
	cameraMatWorld.r[1] = { cameraAxisY.x, cameraAxisY.y, cameraAxisY.z, 0.0f };
	cameraMatWorld.r[2] = { cameraAxisZ.x, cameraAxisZ.y, cameraAxisZ.z, 0.0f };
	XMVECTOR rotate, scale, position;
	DirectX::XMMatrixDecompose(&scale, &rotate, &position, cameraMatWorld);
	//�J�����̃g�����X�t�H�[��
	KuroEngine::Transform cameraT = arg_targetPos;
	cameraT.SetRotate(rotate);

	m_debugTransform = cameraT;

	//�����蔻��p�̃��C��ł��������߂�B
	Vec3<float> checkHitRay = m_cameraLocalTransform.GetPosWorldByMatrix() - m_oldCameraWorldPos;	//�܂��̓f�t�H���g�̃��C�ɐݒ�B

	//�����蔻��ϐ����������B
	m_isHitTerrian = false;

	//�W�����v���͓����蔻����s��Ȃ��B
	if (!arg_isPlayerJump && !arg_isCameraDefaultPos && arg_isNoCollision) {

		//�������ʂƂ̓����蔻��
		Vec3<float> push;
		bool isHit = RayPlaneIntersection(m_playerLerpPos, Vec3<float>(pushBackPos - m_playerLerpPos).GetNormal(), m_playerLerpPos - arg_targetPos.GetUp(), arg_targetPos.GetUp(), push);
		if (isHit) {

			//�㉺���̕ǂ�������
			if (0.9f < fabs(arg_targetPos.GetUp().Dot(Vec3<float>(0, 1, 0)))) {
				m_nowParam.m_xAxisAngle = fromXAngle;
			}
			else {
				m_nowParam.m_yAxisAngle = fromYAngle;
				arg_playerRotY = fromYAngle;
			}

		}

		//�v���C���[���Փ˂����n�_�Ƃ̓����蔻��
		int counter = 0;
		for (auto& index : arg_hitPointData) {

			//���_�t�߂�Nan�̒l�͏���
			if (std::isnan(index.m_pos.x)) continue;
			if (index.m_pos.Length() <= 0.1f) continue;

			//�J�����ƃv���C���[�̋���
			float cameraDistance = Vec3<float>(m_playerLerpPos - pushBackPos).Length();

			isHit = RayPlaneIntersection(m_playerLerpPos, Vec3<float>(pushBackPos - m_playerLerpPos).GetNormal(), index.m_pos, index.m_up, push);
			if (isHit) {

				//�㉺���̕ǂ�������
				if (0.9f < fabs(index.m_up.Dot(Vec3<float>(0, 1, 0)))) {
					m_nowParam.m_xAxisAngle = fromXAngle;
				}
				else {
					m_nowParam.m_yAxisAngle = fromYAngle;
					arg_playerRotY = fromYAngle;
				}

			}

			++counter;

		}

		//�ʏ�̒n�`�𑖍�
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

					PushBackGround(output, pushBackPos, arg_targetPos, arg_playerRotY, arg_isCameraUpInverse, true);

				}

				//�v���C���[�����̃��C�g�̓����蔻������s
				Vec3<float> playerDir = m_playerLerpPos - pushBackPos;
				output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(pushBackPos, playerDir.GetNormal(), checkHitMesh);
				if (output.m_isHit && 0 < output.m_distance && output.m_distance < playerDir.Length()) {

					PushBackGround(output, pushBackPos, arg_targetPos, arg_playerRotY, arg_isCameraUpInverse, false);

				}

			}

			//=================================================
		}
	}

	//��Ԃ���B
	m_attachedCam.lock()->GetTransform().SetPos(KuroEngine::Math::Lerp(m_attachedCam.lock()->GetTransform().GetPos(), pushBackPos, 0.3f));

	//���݂̍��W����v���C���[�Ɍ�������]�����߂�B
	Vec3<float> axisZ = m_playerLerpPos - m_attachedCam.lock()->GetTransform().GetPosWorld();
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

	//�v���C���[���ړ����Ă�����ۑ��B
	if (0.1f < KuroEngine::Vec3<float>(m_playerOldPos - m_playerLerpPos).Length()) {
		m_playerOldPos = m_playerLerpPos;
	}



}

void CameraController::JumpStart(const KuroEngine::Transform& arg_playerTransform, const KuroEngine::Vec3<float>& arg_jumpEndNormal, bool arg_isCameraUpInverse, float arg_scale)
{

	using namespace KuroEngine;

	//�J����������̈ʒu�ɒB���Ă��Ȃ��������Ԃ�������B

	//�v���C���[��Y�ʂɂ�����
	if (0.9f < fabs(arg_playerTransform.GetUp().y)) {

		//�J���������������̈ʒu�ɕ�Ԃ���ʁB
		const float CAMERA_LERP_AMOUNT = 0.5f;	//���ςŎg�p����̂ŁA�܂�n�ʂ��猩��45�x�̈ʒu�ɕ�Ԃ���B

		//�܂茻�݂�XZ���ʏ�ɂ���Ƃ������ƂȂ̂ŁA�J��������v���C���[�܂ł̃x�N�g����2D�Ɏˉe�B
		Vec3<float> cameraVec = Vec3<float>(m_attachedCam.lock()->GetTransform().GetPos() - arg_playerTransform.GetPos()).GetNormal();
		Vec2<float> cameraVec2D = Project3Dto2D(cameraVec, Vec3<float>(1, 0, 0), Vec3<float>(0, 0, 1));
		Vec2<float> jumpEndNormal2D = Project3Dto2D(arg_jumpEndNormal, Vec3<float>(1, 0, 0), Vec3<float>(0, 0, 1));

		//��]�ʂ�񎟌��œ���B
		float dot = jumpEndNormal2D.Dot(cameraVec2D);

		//�x�N�g���̍���CAMERA_LARP_AMOUNT��艺���������Ԃ̏���������B
		if (dot < CAMERA_LERP_AMOUNT) {

			//���W�A���ɒ����B
			float rad = acos(dot);

			//��Ԃ�����������߂�B
			float cross = std::signbit(jumpEndNormal2D.Cross(cameraVec2D)) ? -1.0f : 1.0f;

			//��Ԃ�����B
			m_rotateYLerpAmount += ((rad - CAMERA_LERP_AMOUNT) * cross) * arg_scale;

		}


	}
	//�v���C���[��Z�ʂɂ����� and Y�ʂɃW�����v���Ă�����B
	if (0.9f < fabs(arg_playerTransform.GetUp().z) && 0.9f < fabs(arg_jumpEndNormal.y)) {

		//�J���������������̈ʒu�ɕ�Ԃ���ʁB
		const float CAMERA_LERP_AMOUNT = 0.25f;	//���ςŎg�p����̂ŁA�܂�n�ʂ��猩��45�x�̈ʒu�ɕ�Ԃ���B

		//�܂茻�݂�XY���ʏ�ɂ���Ƃ������ƂȂ̂ŁA�J��������v���C���[�܂ł̃x�N�g����2D�Ɏˉe�B
		Vec3<float> cameraVec = Vec3<float>(m_attachedCam.lock()->GetTransform().GetPos() - arg_playerTransform.GetPos()).GetNormal();
		Vec2<float> cameraVec2D = Project3Dto2D(cameraVec, Vec3<float>(1, 0, 0), Vec3<float>(0, 1, 0));
		Vec2<float> jumpEndNormal2D = Project3Dto2D(arg_jumpEndNormal, Vec3<float>(1, 0, 0), Vec3<float>(0, 1, 0));

		//��]�ʂ�񎟌��œ���B
		float dot = jumpEndNormal2D.Dot(cameraVec2D);

		//�x�N�g���̍���CAMERA_LARP_AMOUNT��艺���������Ԃ̏���������B
		if (dot < CAMERA_LERP_AMOUNT) {

			//���W�A���ɒ����B
			float rad = acos(dot);

			//�ʈړ��̏u�Ԃ�������B
			bool isUpInverseTrigger = (!arg_isCameraUpInverse && arg_jumpEndNormal.y < -0.9f) || (arg_isCameraUpInverse && 0.9f < arg_jumpEndNormal.y);

			//��Ԃ�����������߂�B
			float inverse = arg_isCameraUpInverse ? -1.0f : 1.0f;

			//��Ԃ�����B
			m_cameraXAngleLerpAmount += ((rad - CAMERA_LERP_AMOUNT) * inverse * (isUpInverseTrigger ? -1.0f : 1.0f)) * arg_scale;

		}

	}

}

void CameraController::PushBackGround(const CollisionDetectionOfRayAndMesh::MeshCollisionOutput& arg_output, const KuroEngine::Vec3<float> arg_pushBackPos, const KuroEngine::Transform& arg_targetPos, float& arg_playerRotY, bool arg_isCameraUpInverse, bool arg_isAroundRay) {

	using namespace KuroEngine;

	//�������Ă���ǂ��㉺�������炾������B
	if (0.9f < std::fabs(arg_output.m_normal.y)) {

		//���E������s���B
		Vec3<float> pushBackVec = KuroEngine::Vec3<float>(arg_pushBackPos - m_playerLerpPos).GetNormal();

		//�v���C���[�̉E�x�N�g���ƌ��݂̃J�����x�N�g�����ׁA0�ȏゾ������E�B
		if (0 < arg_targetPos.GetRight().Dot(pushBackVec) && (!arg_isCameraUpInverse)) {

			//���݂�X�p�x�����߂�B
			Vec2<float> nowXVec = Project3Dto2D(KuroEngine::Vec3<float>(m_cameraLocalTransform.GetPosWorldByMatrix() - m_playerLerpPos).GetNormal(), arg_targetPos.GetFront(), arg_targetPos.GetRight());
			Vec2<float> pushBackVec = Project3Dto2D(KuroEngine::Vec3<float>(arg_pushBackPos - m_playerLerpPos).GetNormal(), arg_targetPos.GetFront(), arg_targetPos.GetRight());

			//�p�x�̍�
			float nowAngle = atan2f(nowXVec.y, nowXVec.x);
			float pushBackAngle = atan2f(pushBackVec.y, pushBackVec.x);
			float divAngle = pushBackAngle - nowAngle;

			//�p�x�������߂��B
			m_nowParam.m_xAxisAngle += divAngle;

		}
		else {

			//���݂�X�p�x�����߂�B
			Vec2<float> nowXVec = Project3Dto2D(KuroEngine::Vec3<float>(m_cameraLocalTransform.GetPosWorldByMatrix() - m_playerLerpPos).GetNormal(), arg_targetPos.GetRight(), arg_targetPos.GetFront());
			Vec2<float> pushBackVec = Project3Dto2D(KuroEngine::Vec3<float>(arg_pushBackPos - m_playerLerpPos).GetNormal(), arg_targetPos.GetRight(), arg_targetPos.GetFront());

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
		Vec3<float> pushBackVec = KuroEngine::Vec3<float>(arg_pushBackPos - m_playerLerpPos).GetNormal();

		//�v���C���[�̉E�x�N�g���ƌ��݂̃J�����x�N�g�����ׁA0�ȏゾ������E�B
		float dot = arg_targetPos.GetRight().Dot(pushBackVec);
		if (arg_isCameraUpInverse ? (dot < 0) : (0 < dot)) {

			//���݂�Y�p�x�����߂�B
			Vec2<float> nowYVec = Project3Dto2D(KuroEngine::Vec3<float>(m_cameraLocalTransform.GetPosWorldByMatrix() - m_playerLerpPos).GetNormal(), arg_targetPos.GetFront(), arg_targetPos.GetUp());
			Vec2<float> pushBackVec = Project3Dto2D(KuroEngine::Vec3<float>(arg_pushBackPos - m_playerLerpPos).GetNormal(), arg_targetPos.GetFront(), arg_targetPos.GetUp());


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
			Vec2<float> nowYVec = Project3Dto2D(KuroEngine::Vec3<float>(m_cameraLocalTransform.GetPosWorldByMatrix() - m_playerLerpPos).GetNormal(), arg_targetPos.GetUp(), arg_targetPos.GetFront());
			Vec2<float> pushBackVec = Project3Dto2D(KuroEngine::Vec3<float>(arg_pushBackPos - m_playerLerpPos).GetNormal(), arg_targetPos.GetUp(), arg_targetPos.GetFront());


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

void CameraController::PlayerMoveCameraLerp(KuroEngine::Vec3<float> arg_scopeMove, KuroEngine::Transform arg_targetPos, float& arg_playerRotY, float arg_cameraZ, const std::weak_ptr<Stage> arg_nowStage, bool arg_isCameraUpInverse, bool arg_isCameraDefaultPos, bool& arg_isHitUnderGround, bool arg_isMovePlayer, bool arg_isPlayerJump, KuroEngine::Quaternion arg_cameraQ)
{

	using namespace KuroEngine;

	KuroEngine::Transform cameraT;
	cameraT.SetRotate(arg_cameraQ);

	//�v���C���[���W�����v���Ă��Ȃ��āA�v���C���[�������Ă��鎞�B
	if (!arg_isPlayerJump && arg_isMovePlayer) {

		//�v���C���[���������x�N�g���̋t��2D�Ɏˉe����B
		Vec3<float> playerMoveVec = Vec3<float>(m_playerLerpPos - m_playerOldPos).GetNormal();
		Vec3<float> cameraVec = Vec3<float>(m_playerLerpPos - m_attachedCam.lock()->GetTransform().GetPos()).GetNormal();

		Vec2<float> playerMoveVec2D = -Project3Dto2D(playerMoveVec, cameraT.GetFront(), cameraT.GetRight());

		//�J�����̃x�N�g����2D�Ɏˉe����B
		Vec2<float> cameraVec2D = Project3Dto2D(cameraVec, cameraT.GetFront(), cameraT.GetRight());

		//�J�����x�N�g���ƈړ������x�N�g���̓��ς̌��ʂ�0.5�ȉ��������瓖���蔻����s���B
		float dot = cameraVec2D.Dot(playerMoveVec2D);
		if (-0.8f < dot) {

			//Y����̂�����m�F�B
			float zureY = acos(playerMoveVec2D.Dot(cameraVec2D)) * (0.9f < fabs(KuroEngine::Vec3<float>(0, 1, 0).Dot(arg_targetPos.GetUp())) ? 0.01f : 0.001f);
			float cross = playerMoveVec2D.Cross(cameraVec2D);

			if (0 < fabs(cross)) {

				cross = (signbit(cross) ? -1.0f : 1.0f);
				cross *= (arg_isCameraUpInverse ? -1.0f : 1.0f);

				if (0.9f < fabs(arg_targetPos.GetUp().y)) {

					//Y���𓮂����B
					m_nowParam.m_yAxisAngle += zureY * cross;
					arg_playerRotY += zureY * cross;

				}
				else {

					//Y���𓮂����B
					m_nowParam.m_yAxisAngle -= zureY * cross;
					arg_playerRotY -= zureY * cross;

				}

			}

		}

	}

}

bool CameraController::RayPlaneIntersection(const KuroEngine::Vec3<float>& arg_rayOrigin, const KuroEngine::Vec3<float>& arg_rayDirection, const KuroEngine::Vec3<float>& arg_planePoint, const KuroEngine::Vec3<float>& arg_planeNormal, KuroEngine::Vec3<float>& arg_hitResult)
{

	using namespace KuroEngine;

	//�x�N�g���𐳋K��
	Vec3<float> nRayDirection = arg_rayDirection.GetNormal();
	Vec3<float> nPlaneNormal = arg_planeNormal.GetNormal();

	//���C�ƕ��ʂ����s�ł͂Ȃ������`�F�b�N�B
	float denominator = nRayDirection.Dot(nPlaneNormal);
	if (abs(denominator) < FLT_EPSILON) {
		return false;
	}

	//��_�����߂邽�߂�t���v�Z�B
	float t = (arg_planePoint - arg_rayOrigin).Dot(nPlaneNormal) / denominator;

	//t��0�����������ꍇ�A��_�����o�ł��Ȃ��B
	if (t < 0.0f) {
		return false;
	}

	//�Փ˒n�_��Ԃ��I
	arg_hitResult = arg_rayOrigin + nRayDirection * t;
	return true;

}

void CameraController::UpdateLookAround(KuroEngine::Vec3<float> arg_scopeMove, KuroEngine::Transform arg_targetPos, float& arg_playerRotY, float arg_cameraZ, const std::weak_ptr<Stage> arg_nowStage, bool arg_isCameraUpInverse, bool arg_isCameraDefaultPos, bool& arg_isHitUnderGround, bool arg_isMovePlayer, bool arg_isPlayerJump, KuroEngine::Quaternion arg_cameraQ, bool arg_isFrontWall, KuroEngine::Transform arg_drawTransform, KuroEngine::Vec3<float> arg_frontWallNormal, bool arg_isNoCollision, bool arg_isLookAroundMode)
{

	using namespace KuroEngine;

	//�J�������[�h���؂�ւ�����u�Ԃ�������
	if (!m_isCameraModeLookAround && arg_isLookAroundMode) {

		//�J�����܂ł̃x�N�g���B
		KuroEngine::Vec3<float> cameraDir = Vec3<float>(m_attachedCam.lock()->GetTransform().GetPosWorld() - arg_targetPos.GetPosWorld()).GetNormal();

		//��x�N�g���Ƃ̓��ςƊO�ς����]�𓾂�B
		KuroEngine::Vec3<float> upVec = arg_isCameraUpInverse ? Vec3<float>(0, 1, 0) : Vec3<float>(0, 1, 0);
		KuroEngine::Vec3<float> axis = upVec.Cross(cameraDir);
		float angle = acos(upVec.Dot(cameraDir));

		//��]�p�����݂�����
		if (0.0f < axis.Length()) {

			m_lookAroundTransform.SetRotate(DirectX::XMQuaternionRotationAxis(axis, angle));

		}
		else {
			m_lookAroundTransform.SetRotate(DirectX::XMQuaternionIdentity());
		}

		//��]�̏����l��ۑ��B
		m_lookAroundInitTransform = m_lookAroundTransform;

		m_lookAroundModeFar = Vec3<float>(m_attachedCam.lock()->GetTransform().GetPos() - arg_targetPos.GetPos()).Length();
		m_isLookAroundFinish = false;
		m_isLookAroundFinishComplete = false;

	}

	//�J�����̋������ԁB
	if (m_isLookAroundFinish) {

		m_lookAroundModeFar = KuroEngine::Math::Lerp(m_lookAroundModeFar, fabs(arg_cameraZ), 0.08f);
		m_lookAroundTransform.SetRotate(DirectX::XMQuaternionSlerp(m_lookAroundTransform.GetRotate(), m_lookAroundInitTransform.GetRotate(), 0.08f));

		//�J�����̋������K��l�ɒB������I���B
		if (fabs(fabs(m_lookAroundModeFar) - fabs(arg_cameraZ)) < 1.0f) {

			m_isLookAroundFinishComplete = true;

		}

	}
	else {

		m_lookAroundModeFar = KuroEngine::Math::Lerp(m_lookAroundModeFar, LOOK_AROUND_FAR, 0.08f);

	}

	//�J�����̈ʒu�𓮂����B
	m_attachedCam.lock()->GetTransform().SetPos(arg_targetPos.GetPosWorld() + m_lookAroundTransform.GetUp() * m_lookAroundModeFar);

	//���݂̍��W����v���C���[�Ɍ�������]�����߂�B
	Vec3<float> axisZ = m_playerLerpPos - m_attachedCam.lock()->GetTransform().GetPosWorld();
	axisZ.Normalize();

	//�v���C���[�̖@���Ƃ̊O�ς��牼��X�x�N�g���𓾂�B
	Vec3<float> axisX = Vec3<float>(0, arg_isCameraUpInverse ? -1.0f : 1.0f, 0).Cross(axisZ);

	//X�x�N�g�������x�N�g���𓾂�B
	Vec3<float> axisY = axisZ.Cross(axisX);

	//�p���𓾂�B
	DirectX::XMMATRIX matWorld = DirectX::XMMatrixIdentity();
	XMVECTOR rotate, scale, position;
	matWorld.r[0] = { axisX.x, axisX.y, axisX.z, 0.0f };
	matWorld.r[1] = { axisY.x, axisY.y, axisY.z, 0.0f };
	matWorld.r[2] = { axisZ.x, axisZ.y, axisZ.z, 0.0f };

	DirectX::XMMatrixDecompose(&scale, &rotate, &position, matWorld);

	//��]��K�p�B
	m_attachedCam.lock()->GetTransform().SetRotate(rotate);

	//�J�����̏㉺�����]��Ԃ���������͂����]������B
	if (arg_isCameraUpInverse) {
		arg_scopeMove *= -1.0f;
	}

	//�X�e�B�b�N����ɂ���ĉ�]������B
	if (0 < fabs(arg_scopeMove.x)) {
		m_lookAroundTransform.SetRotate(DirectX::XMQuaternionMultiply(m_lookAroundTransform.GetRotate(), DirectX::XMQuaternionRotationAxis(arg_isCameraUpInverse ? Vec3<float>(0, -1, 0) : Vec3<float>(0, 1, 0), arg_scopeMove.x * 0.5f)));
	}
	if (0 < fabs(arg_scopeMove.y)) {
		m_lookAroundTransform.SetRotate(DirectX::XMQuaternionMultiply(m_lookAroundTransform.GetRotate(), DirectX::XMQuaternionRotationAxis(axisX, arg_scopeMove.y * -0.5f)));
	}


	m_isCameraModeLookAround = arg_isLookAroundMode;

}
