#include "Player.h"
#include"Render/RenderObject/Camera.h"
#include"../OperationConfig.h"
#include"FrameWork/Importer.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"../Graphics/BasicDraw.h"
#include"../Stage/Stage.h"
#include"../Graphics/BasicDrawParameters.h"

void Player::OnImguiItems()
{
	using namespace KuroEngine;

	//�g�����X�t�H�[��
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Transform"))
	{
		auto pos = m_transform.GetPos();
		auto angle = m_transform.GetRotateAsEuler();

		if (ImGui::DragFloat3("Position", (float*)&pos, 0.5f))
		{
			m_transform.SetPos(pos);
		}

		//���삵�₷���悤�ɃI�C���[�p�ɕϊ�
		KuroEngine::Vec3<float>eular = { angle.x.GetDegree(),angle.y.GetDegree(),angle.z.GetDegree() };
		if (ImGui::DragFloat3("Eular", (float*)&eular, 0.5f))
		{
			m_transform.SetRotate(Angle::ConvertToRadian(eular.x), Angle::ConvertToRadian(eular.y), Angle::ConvertToRadian(eular.z));
		}
		ImGui::TreePop();

		//�O�x�N�g��
		auto front = m_transform.GetFront();
		ImGui::Text("Front : %.2f ,%.2f , %.2f", front.x, front.y, front.z);

		//��x�N�g��
		auto up = m_transform.GetUp();
		ImGui::Text("Up : %.2f ,%.2f , %.2f", up.x, up.y, up.z);

		ImGui::Text("OnGround : %d", m_onGround);

		ImGui::DragFloat("GrassPosScatter : X", &m_grassPosScatter.x, 0.1f);
		ImGui::DragFloat("GrassPosScatter : Y", &m_grassPosScatter.y, 0.1f);

	}

	//�ړ�
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Move")) {

		ImGui::DragFloat("MoveAccel", &m_moveAccel, 0.01f);
		ImGui::DragFloat("MaxSpeed", &m_maxSpeed, 0.01f);
		ImGui::DragFloat("Brake", &m_brake, 0.01f);

		ImGui::TreePop();
	}

	//�J����
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Camera"))
	{
		// ImGui::DragFloat3("Target", (float*)&target, 0.5f);
		ImGui::DragFloat("Sensitivity", &m_camSensitivity, 0.05f);

		ImGui::TreePop();
	}
}

bool Player::HitCheckAndPushBack(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, const std::vector<Terrian>& arg_terrianArray, HitCheckResult* arg_hitInfo)
{
	/*
	arg_from �c �ړ��O�̍��W
	arg_to �c �ړ���̍��W
	arg_terrianArray �c �n�`�̔z��
	arg_terrianNormal �c ���������n�`�̃��b�V���̖@���A�i�[��
	*/

	//�����蔻�茋��
	bool isHitWall = false;
	bool onGround = false;
	HitCheckResult hitResult;

	//CastRay�ɓn������
	Player::CastRayArgument castRayArgument(onGround, isHitWall, hitResult);

	m_debugTransform.clear();

	//�n�`�z�񑖍�
	for (auto& terrian : arg_terrianArray)
	{
		//���f�����擾
		auto model = terrian.m_model.lock();
		//�g�����X�t�H�[�����
		auto& transform = terrian.m_transform;
		castRayArgument.m_targetTransform = terrian.m_transform;

		//���b�V���𑖍�
		for (auto& modelMesh : model->m_meshes)
		{
			//���b�V�����擾
			auto& mesh = modelMesh.mesh;

			//CastRay�ɓn���������X�V�B
			castRayArgument.m_mesh = terrian.m_collisionMesh[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//���聫============================================


			//�E�����Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_newPos, arg_newPos, m_transform.GetRight(), m_transform.GetScale().x, castRayArgument, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_newPos, arg_newPos, -m_transform.GetRight(), m_transform.GetScale().x, castRayArgument, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_newPos, arg_newPos, -m_transform.GetFront(), m_transform.GetScale().z, castRayArgument, RAY_ID::AROUND);

			//���ʕ����Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_newPos, arg_newPos, m_transform.GetFront(), m_transform.GetScale().z, castRayArgument, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͒n�ʂƂ̉����߂��p�B
			CastRay(arg_newPos, arg_newPos, -m_transform.GetUp(), m_transform.GetScale().y, castRayArgument, RAY_ID::GROUND);

			//�󒆂ɂ���g���K�[�̏ꍇ�͊R�̏����B
			if (!m_onGround && m_prevOnGround) {

				if (0 < m_moveSpeed.z) {

					//�O�ɐi��ŊR�ɗ������ꍇ�B
					CastRay(arg_newPos, arg_from - m_transform.GetUp() * m_transform.GetScale().y, -m_transform.GetFront(), m_transform.GetScale().x, castRayArgument, RAY_ID::CLIFF);

				}

				if (m_moveSpeed.z < 0) {

					//���ɐi��ŊR�ɗ������ꍇ�B
					CastRay(arg_newPos, arg_from - m_transform.GetUp() * m_transform.GetScale().y, m_transform.GetFront(), m_transform.GetScale().x, castRayArgument, RAY_ID::CLIFF);

				}

				if (0 < m_moveSpeed.x) {

					//���ɐi��ŊR�ɗ������ꍇ�B
					CastRay(arg_newPos, arg_from - m_transform.GetUp() * m_transform.GetScale().y, -m_transform.GetRight(), m_transform.GetScale().z, castRayArgument, RAY_ID::CLIFF);

				}

				if (m_moveSpeed.x < 0) {

					//�E�ɐi��ŊR�ɗ������ꍇ�B
					CastRay(arg_newPos, arg_from - m_transform.GetUp() * m_transform.GetScale().y, m_transform.GetRight(), m_transform.GetScale().z, castRayArgument, RAY_ID::CLIFF);

				}

			}


			//=================================================
		}
	}

	//���͂̃��C���ǂɓ������Ă��Ȃ�������A�������̃��C���p���Ƃ��Ďg�p����B
	if (!isHitWall && onGround) {
		hitResult.m_terrianNormal = hitResult.m_bottmRayTerrianNormal;
		isHitWall = true;
	}

	//�ڒn�t���O��ۑ�
	m_prevOnGround = m_onGround;
	m_onGround = onGround;

	//�����蔻�肪true�Ȃ瓖�������n�`�̖@�����i�[
	if (isHitWall && arg_hitInfo)
	{
		*arg_hitInfo = hitResult;
	}
	else {

		//�ǂ��Ƃ��Փ˂��Ă��Ȃ������猻�݂̏�x�N�g����@���Ƃ���B(�K����]�̏�����ʂ�悤�ɂ��邽��)
		hitResult.m_bottmRayTerrianNormal = m_transform.GetUp();
		hitResult.m_terrianNormal = m_transform.GetUp();
		*arg_hitInfo = hitResult;
		isHitWall = true;

	}

	return isHitWall;
}

Player::Player()
	:KuroEngine::Debugger("Player", true, true)
{
	AddCustomParameter("Sensitivity", { "camera", "sensitivity" }, PARAM_TYPE::FLOAT, &m_camSensitivity, "Camera");
	LoadParameterLog();

	//���f���ǂݍ���
	m_model = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Player.glb");
	m_axisModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Axis.glb");
	m_camModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Camera.glb");

	//�J��������
	m_cam = std::make_shared<KuroEngine::Camera>("Player's Camera");
	//�J�����̃R���g���[���[�ɃA�^�b�`
	m_camController.AttachCamera(m_cam);

	m_cameraRotY = 0;
	m_cameraQ = DirectX::XMQuaternionIdentity();

	m_moveSpeed = KuroEngine::Vec3<float>();
	m_isFlipMove = false;
}

void Player::Init(KuroEngine::Transform arg_initTransform)
{
	m_transform = arg_initTransform;
	m_camController.Init();
	m_cameraRotY = 0;
	m_cameraQ = DirectX::XMQuaternionIdentity();

	m_moveSpeed = KuroEngine::Vec3<float>();
	m_isFlipMove = false;
}

void Player::Update(const std::weak_ptr<Stage>arg_nowStage)
{

	using namespace KuroEngine;

	//�v���C���[�̉�]���J������ɂ���B(�ړ������̊���J�����̊p�x�Ȃ���)
	m_transform.SetRotate(m_cameraQ);

	auto beforePos = m_transform.GetPos();
	auto newPos = beforePos;
	auto rotate = m_transform.GetRotate();

	//���͂��ꂽ�ړ��ʂ��擾
	m_rowMoveVec = OperationConfig::Instance()->GetMoveVecFuna(XMQuaternionIdentity());	//���̓��͕������擾�B�v���C���[����͕����ɉ�]������ۂɁAXZ���ʂł̒l���g�p����������B

	//�������͓��͂𖳌����B
	if (!m_onGround) {
		m_rowMoveVec = KuroEngine::Vec3<float>();
	}
	m_moveSpeed += m_rowMoveVec * m_moveAccel;

	//�ړ����x���N�����v�B
	m_moveSpeed.x = std::clamp(m_moveSpeed.x, -m_maxSpeed, m_maxSpeed);
	m_moveSpeed.z = std::clamp(m_moveSpeed.z, -m_maxSpeed, m_maxSpeed);

	//���͂��ꂽ�l������������ړ����x�����炷�B
	if (std::fabs(m_rowMoveVec.x) < 0.001f) {

		m_moveSpeed.x = std::clamp(std::fabs(m_moveSpeed.x) - m_brake, 0.0f, m_maxSpeed) * (std::signbit(m_moveSpeed.x) ? -1.0f : 1.0f);

	}

	if (std::fabs(m_rowMoveVec.z) < 0.001f) {

		m_moveSpeed.z = std::clamp(std::fabs(m_moveSpeed.z) - m_brake, 0.0f, m_maxSpeed) * (std::signbit(m_moveSpeed.z) ? -1.0f : 1.0f);

	}

	//�v���C���[��Y-�̕ǂɒ���t���Ă��邩�ǂ�����X���̈ړ������𔽓]������B
	auto moveSpeed = m_moveSpeed;
	if (m_transform.GetUp().y < -0.9f && m_isFlipXinput) {
		moveSpeed.x *= -1.0f;
		moveSpeed.z *= -1.0f;
	}
	else if (-0.9f < m_transform.GetUp().y && m_isFlipXinput) {
		moveSpeed.z *= -1.0f;
	}
	else if (m_isFlipMove) {
		//X���̓����𔽓]�BZ���̓����̓J�����̃N�H�[�^�j�I�����Ŕ��]�����Ă���B
		moveSpeed.x *= -1.0f;
	}

	//���͂��ꂽ�����ړ��p�x�ʂ��擾
	auto scopeMove = OperationConfig::Instance()->GetScopeMove();

	//�J�����̉�]��ۑ��B
	m_cameraRotY += scopeMove.x;

	//�v���C���[�̉�]��ۑ��B���͂��������Ƃ��́B
	if (0 < m_rowMoveVec.Length()) {
		//Y-���ʂɒ���t���Ă����Ƃ���Z�����t�ɂ���B
		if (m_isFlipMove) {
			m_playerRotY = atan2f(m_rowMoveVec.x, m_rowMoveVec.z);
		}
		else {
			m_playerRotY = atan2f(m_rowMoveVec.x, m_rowMoveVec.z);
		}
	}

	//���͂�����������B
	if (m_moveSpeed.Length() < 0.001f) {
		//GetUp��Y���ɉ����Ĉړ������𔽓]�����邩�̃t���O��؂�ւ���B
		if (m_transform.GetUp().y < -0.9f) {
			m_isFlipMove = true;
		}
		else {
			m_isFlipMove = false;
		}
		m_isFlipXinput = false;
	}

	//���[�J�����̈ړ��������v���C���[�̉�]�ɍ��킹�ē������B
	auto moveAmount = KuroEngine::Math::TransformVec3(moveSpeed, rotate);

	//�ړ��ʉ��Z
	newPos += moveAmount;

	//�n�ʂɒ���t����p�̏d�́B
	if (!m_onGround) {
		newPos -= m_transform.GetUp() * (m_transform.GetScale().y / 2.0f);
	}

	//�����蔻��
	HitCheckResult hitResult;
	if (HitCheckAndPushBack(beforePos, newPos, arg_nowStage.lock()->GetTerrianArray(), &hitResult))
	{

		//�@������������N�H�[�^�j�I��
		auto spin = Math::GetLookAtQuaternion({ 0,1,0 }, hitResult.m_terrianNormal);

		//�J�����ڐ���Y����]������N�H�[�^�j�I��
		DirectX::XMVECTOR ySpin;
		//�v���C���[�̈ړ�������Y����]������N�H�[�^�j�I��
		DirectX::XMVECTOR playerYSpin;

		if (m_isFlipMove && -0.9f <= hitResult.m_terrianNormal.y) {

			//�v���C���[��Y-�̕ǂɒ���t���Ă���ꍇ�̓J�����̉�]��Y+�����Y-��ɐ؂�ւ���B
			ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_cameraRotY + DirectX::XM_PI);

			//�v���C���[�̈ړ�������Y����]������N�H�[�^�j�I���B�ړ������ɉ�]���Ă���悤�Ɍ��������邽�߂̂��́B
			playerYSpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_playerRotY + DirectX::XM_PI);

		}
		else if (m_isFlipMove) {

			//�v���C���[��Y-�̕ǂɒ���t���Ă���ꍇ�̓J�����̉�]��Y+�����Y-��ɐ؂�ւ���B
			ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, -m_cameraRotY + DirectX::XM_PI);

			//�v���C���[�̈ړ�������Y����]������N�H�[�^�j�I���B�ړ������ɉ�]���Ă���悤�Ɍ��������邽�߂̂��́B
			playerYSpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_playerRotY + DirectX::XM_PI);

		}
		else if (hitResult.m_terrianNormal.y < -0.9f) {

			//�v���C���[��Y-�̕ǂɒ���t���Ă���ꍇ�̓J�����̉�]��Y+�����Y-��ɐ؂�ւ���B
			ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, -m_cameraRotY);

			//�v���C���[�̈ړ�������Y����]������N�H�[�^�j�I���B�ړ������ɉ�]���Ă���悤�Ɍ��������邽�߂̂��́B
			playerYSpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_playerRotY);

		}
		else {

			ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_cameraRotY);

			//�v���C���[�̈ړ�������Y����]������N�H�[�^�j�I���B�ړ������ɉ�]���Ă���悤�Ɍ��������邽�߂̂��́B
			playerYSpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_playerRotY);
		}

		bool isPlayerCeil = m_transform.GetUp().y < -0.9f;
		bool isFlipNormalToCeil = 0 != m_rowMoveVec.x && !m_isFlipMove && hitResult.m_terrianNormal.y < -0.9f && !isPlayerCeil;
		bool isFlipCeilToNormal = 0 != m_rowMoveVec.x && m_isFlipMove && -0.9f <= hitResult.m_terrianNormal.y && isPlayerCeil;
		if (isFlipNormalToCeil || isFlipCeilToNormal) {
			m_isFlipXinput = true;
		}

		//�J���������ł̃N�H�[�^�j�I�������߂�B�i�ޕ����Ȃǂ𔻒f����̂Ɏg�p����̂͂������BF�̈�ԍŏ��ɂ��̒l�����邱�Ƃ�playerYSpin�̉�]��ł������B
		m_cameraQ = DirectX::XMQuaternionMultiply(spin, ySpin);

		//�v���C���[�̈ړ�������Y����]������N�H�[�^�j�I�����J�����̃N�H�[�^�j�I���ɂ����āA�v���C���[���ړ������Ɍ�������B
		m_moveQ = DirectX::XMQuaternionMultiply(m_cameraQ, playerYSpin);
		m_transform.SetRotate(m_moveQ);

	}

	//���W�ω��K�p
	m_transform.SetPos(newPos);
	m_ptLig.SetPos(newPos);

	//�J��������
	m_camController.Update(scopeMove, newPos);

}

void Player::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, bool arg_cameraDraw)
{
	/*
	KuroEngine::DrawFunc3D::DrawNonShadingModel(
		m_model,
		m_transform,
		arg_cam);
	*/

	static IndividualDrawParameter drawParam = IndividualDrawParameter::GetDefault();
	drawParam.m_isPlayer = 1;

	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model,
		m_transform,
		drawParam);

	for (auto& index : m_debugTransform) {
		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			m_model,
			index);
	}

	/*KuroEngine::DrawFunc3D::DrawNonShadingModel(
		m_axisModel,
		m_transform,
		arg_cam);*/

	if (arg_cameraDraw)
	{
		auto camTransform = m_cam->GetTransform();
		KuroEngine::DrawFunc3D::DrawNonShadingModel(
			m_camModel,
			camTransform.GetMatWorld(),
			camTransform.GetPos().z,
			arg_cam);
	}
}

void Player::Finalize()
{
}

Player::MeshCollisionOutput Player::MeshCollision(const KuroEngine::Vec3<float>& arg_rayPos, const KuroEngine::Vec3<float>& arg_rayDir, std::vector<Terrian::Polygon>& arg_targetMesh, KuroEngine::Transform arg_targetTransform) {


	/*===== ���b�V���ƃ��C�̓����蔻�� =====*/


	/*-- �@ �|���S����@���������ƂɃJ�����O���� --*/

	//�@���ƃ��C�̕����̓��ς�0���傫�������ꍇ�A���̃|���S���͔w�ʂȂ̂ŃJ�����O����B
	for (auto& index : arg_targetMesh) {

		if (index.m_p1.normal.Dot(arg_rayDir) < -0.0001f) continue;

		index.m_isActive = false;

	}


	/*-- �A �|���S���ƃ��C�̓����蔻����s���A�e�����L�^���� --*/

	// �L�^�p�f�[�^
	std::vector<std::pair<Player::MeshCollisionOutput, Terrian::Polygon>> hitDataContainer;

	for (auto& index : arg_targetMesh) {

		//�|���S��������������Ă����玟�̏�����
		if (!index.m_isActive) continue;

		//���C�̊J�n�n�_���畽�ʂɂ��낵�������̒��������߂�
		//KuroEngine::Vec3<float> planeNorm = -index.m_p0.normal;
		KuroEngine::Vec3<float> planeNorm = KuroEngine::Vec3<float>(KuroEngine::Vec3<float>(index.m_p0.pos - index.m_p2.pos).GetNormal()).Cross(KuroEngine::Vec3<float>(index.m_p0.pos - index.m_p1.pos).GetNormal());
		float rayToOriginLength = arg_rayPos.Dot(planeNorm);
		float planeToOriginLength = index.m_p0.pos.Dot(planeNorm);
		//���_���畽�ʂɂ��낵�������̒���
		float perpendicularLine = rayToOriginLength - planeToOriginLength;

		//�O�p�֐��𗘗p���Ď��_����Փ˓_�܂ł̋��������߂�
		float dist = planeNorm.Dot(arg_rayDir);
		float impDistance = perpendicularLine / -dist;

		if (std::isnan(impDistance))continue;

		//�Փ˒n�_
		KuroEngine::Vec3<float> impactPoint = arg_rayPos + arg_rayDir * impDistance;

		/*----- �Փ˓_���|���S���̓����ɂ��邩�𒲂ׂ� -----*/

		/* ��1�{�� */
		KuroEngine::Vec3<float> P1ToImpactPos = (impactPoint - index.m_p0.pos).GetNormal();
		KuroEngine::Vec3<float> P1ToP2 = (index.m_p1.pos - index.m_p0.pos).GetNormal();
		KuroEngine::Vec3<float> P1ToP3 = (index.m_p2.pos - index.m_p0.pos).GetNormal();

		//�Փ˓_�ƕ�1�̓���
		float impactDot = P1ToImpactPos.Dot(P1ToP2);
		//�_1�Ɠ_3�̓���
		float P1Dot = P1ToP2.Dot(P1ToP3);

		//�Փ˓_�ƕ�1�̓��ς��_1�Ɠ_3�̓��ς�菬����������A�E�g
		if (impactDot < P1Dot) {
			index.m_isActive = false;
			continue;
		}

		/* ��2�{�� */
		KuroEngine::Vec3<float> P2ToImpactPos = (impactPoint - index.m_p1.pos).GetNormal();
		KuroEngine::Vec3<float> P2ToP3 = (index.m_p2.pos - index.m_p1.pos).GetNormal();
		KuroEngine::Vec3<float> P2ToP1 = (index.m_p0.pos - index.m_p1.pos).GetNormal();

		//�Փ˓_�ƕ�2�̓���
		impactDot = P2ToImpactPos.Dot(P2ToP3);
		//�_2�Ɠ_1�̓���
		float P2Dot = P2ToP3.Dot(P2ToP1);

		//�Փ˓_�ƕ�2�̓��ς��_2�Ɠ_1�̓��ς�菬����������A�E�g
		if (impactDot < P2Dot) {
			index.m_isActive = false;
			continue;
		}

		/* ��3�{�� */
		KuroEngine::Vec3<float> P3ToImpactPos = (impactPoint - index.m_p2.pos).GetNormal();
		KuroEngine::Vec3<float> P3ToP1 = (index.m_p0.pos - index.m_p2.pos).GetNormal();
		KuroEngine::Vec3<float> P3ToP2 = (index.m_p1.pos - index.m_p2.pos).GetNormal();

		//�Փ˓_�ƕ�3�̓���
		impactDot = P3ToImpactPos.Dot(P3ToP1);
		//�_3�Ɠ_2�̓���
		float P3Dot = P3ToP1.Dot(P3ToP2);

		//�Փ˓_�ƕ�3�̓��ς��_3�Ɠ_2�̓��ς�菬����������A�E�g
		if (impactDot < P3Dot) {
			index.m_isActive = false;
			continue;
		}

		/* �����܂ŗ�����|���S���ɏՓ˂��Ă�I */
		Player::MeshCollisionOutput data;
		data.m_isHit = true;
		data.m_pos = impactPoint;
		data.m_distance = impDistance;
		data.m_normal = index.m_p0.normal;
		hitDataContainer.emplace_back(std::pair(data, index));

	}


	/*-- �B �L�^������񂩂�ŏI�I�ȏՓ˓_�����߂� --*/

	//hitPorygon�̒l��1�ȏゾ�����狗�����ŏ��̗v�f������
	if (0 < hitDataContainer.size()) {

		//�������ŏ��̗v�f������
		int min = 0;
		float minDistance = std::numeric_limits<float>().max();
		for (auto& index : hitDataContainer) {
			if (fabs(index.first.m_distance) < fabs(minDistance)) {
				minDistance = index.first.m_distance;
				min = static_cast<int>(&index - &hitDataContainer[0]);
			}
		}

		//�d�S���W�����߂�B
		KuroEngine::Vec3<float> bary = CalBary(hitDataContainer[min].second.m_p0.pos, hitDataContainer[min].second.m_p1.pos, hitDataContainer[min].second.m_p2.pos, hitDataContainer[min].first.m_pos);

		KuroEngine::Vec3<float> baryBuff = bary;

		//UVW�̒l�������̂ŏC���B
		bary.x = baryBuff.y;
		bary.y = baryBuff.z;
		bary.z = baryBuff.x;

		KuroEngine::Vec2<float> uv = KuroEngine::Vec2<float>();

		//�d�S���W����UV�����߂�B
		uv.x += hitDataContainer[min].second.m_p0.uv.x * bary.x;
		uv.x += hitDataContainer[min].second.m_p1.uv.x * bary.y;
		uv.x += hitDataContainer[min].second.m_p2.uv.x * bary.z;

		uv.y += hitDataContainer[min].second.m_p0.uv.y * bary.x;
		uv.y += hitDataContainer[min].second.m_p1.uv.y * bary.y;
		uv.y += hitDataContainer[min].second.m_p2.uv.y * bary.z;

		hitDataContainer[min].first.m_uv = uv;

		return hitDataContainer[min].first;
	}
	else {

		return Player::MeshCollisionOutput();

	}


}

KuroEngine::Vec3<float> Player::CalBary(const KuroEngine::Vec3<float>& PosA, const KuroEngine::Vec3<float>& PosB, const KuroEngine::Vec3<float>& PosC, const KuroEngine::Vec3<float>& TargetPos)
{

	/*===== �d�S���W�����߂� =====*/

	KuroEngine::Vec3<float> uvw = KuroEngine::Vec3<float>();

	// �O�p�`�̖ʐς����߂�B
	float areaABC = (PosC - PosA).Cross(PosB - PosA).Length() / 2.0f;

	// �d�S���W�����߂�B
	uvw.x = ((PosA - TargetPos).Cross(PosB - TargetPos).Length() / 2.0f) / areaABC;
	uvw.y = ((PosB - TargetPos).Cross(PosC - TargetPos).Length() / 2.0f) / areaABC;
	uvw.z = ((PosC - TargetPos).Cross(PosA - TargetPos).Length() / 2.0f) / areaABC;

	return uvw;

}

void Player::CastRay(KuroEngine::Vec3<float>& arg_charaPos, const KuroEngine::Vec3<float>& arg_rayCastPos, const KuroEngine::Vec3<float>& arg_rayDir, float arg_rayLength, CastRayArgument arg_collisionData, RAY_ID arg_rayID)
{

	/*===== �����蔻��p�̃��C������ =====*/

	//���C���΂��B
	MeshCollisionOutput output = MeshCollision(arg_rayCastPos, arg_rayDir, arg_collisionData.m_mesh, arg_collisionData.m_targetTransform);

	//���C�����b�V���ɏՓ˂��Ă���A�Փ˒n�_�܂ł̋��������C�̒�����菬����������Փ˂��Ă���B
	if (output.m_isHit && std::fabs(output.m_distance) < arg_rayLength) {

		//�҂����艟���߂��Ă��܂��Əd�͂̊֌W�ŃK�N�K�N���Ă��܂��̂ŁA�����ɂ߂荞�܂��ĉ����߂��B
		static const float OFFSET = 0.01f;

		//���C�̎�ނɂ���ĕۑ�����f�[�^��ς���B
		switch (arg_rayID)
		{
		case Player::RAY_ID::GROUND:

			//�O���ɓn���p�̃f�[�^��ۑ�
			arg_collisionData.m_hitResult.m_interPos = output.m_pos;
			arg_collisionData.m_hitResult.m_bottmRayTerrianNormal = output.m_normal;
			arg_collisionData.m_onGround = true;

			//�����߂��B
			arg_charaPos += output.m_normal * (std::fabs(output.m_distance - arg_rayLength) - OFFSET);

			break;

		case Player::RAY_ID::DEBUG:
			//m_debugTransform.SetPos(output.m_pos);

		case Player::RAY_ID::CLIFF:
		case Player::RAY_ID::AROUND:

			//�O���ɓn���p�̃f�[�^��ۑ��B
			arg_collisionData.m_isHitWall = true;
			arg_collisionData.m_hitResult.m_interPos = output.m_pos;
			arg_collisionData.m_hitResult.m_terrianNormal = output.m_normal;

			//���C�̏Փ˒n�_����@�������ɐL�΂����ʒu�Ɉړ�������B
			arg_charaPos = output.m_pos + output.m_normal * (arg_rayLength - OFFSET);

			break;
		default:
			break;
		}
	}

}
