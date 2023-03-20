#include "Player.h"
#include"Render/RenderObject/Camera.h"
#include"../OperationConfig.h"
#include"FrameWork/Importer.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"../Graphics/BasicDraw.h"
#include"../Stage/Stage.h"

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

	//�n�`�z�񑖍�
	for (auto& terrian : arg_terrianArray)
	{
		//���f�����擾
		auto model = terrian.m_model.lock();
		//�g�����X�t�H�[�����
		auto& transform = terrian.m_transform;

		//���b�V���𑖍�
		for (auto& modelMesh : model->m_meshes)
		{
			//���b�V�����擾
			auto& mesh = modelMesh.mesh;

			//���聫============================================


			//�E�����Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_newPos, m_transform.GetRight(), m_transform.GetScale().x, modelMesh, terrian.m_transform, onGround, isHitWall, hitResult, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_newPos, -m_transform.GetRight(), m_transform.GetScale().x, modelMesh, terrian.m_transform, onGround, isHitWall, hitResult, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_newPos, -m_transform.GetFront(), m_transform.GetScale().z, modelMesh, terrian.m_transform, onGround, isHitWall, hitResult, RAY_ID::AROUND);

			//���ʕ����Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_newPos, m_transform.GetFront(), m_transform.GetScale().z, modelMesh, terrian.m_transform, onGround, isHitWall, hitResult, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͒n�ʂƂ̉����߂��p�B
			CastRay(arg_newPos, -m_transform.GetUp(), m_transform.GetScale().y, modelMesh, terrian.m_transform, onGround, isHitWall, hitResult, RAY_ID::GROUND);

			//�󒆂ɂ���g���K�[�̏ꍇ�͊R�̏����B
			if (!m_onGround && m_prevOnGround) {

				//�O�ɐi��ŊR�ɗ������ꍇ�B	*2���Ă���̂́A�f�t�H���g�̃T�C�Y�����̂܂܎g���ƈړ����x���������Ĕ�яo���Ă��܂�����B
				CastRay(arg_newPos - m_transform.GetUp() * m_transform.GetScale().y, -m_transform.GetFront(), m_transform.GetScale().x * 2.0f, modelMesh, terrian.m_transform, onGround, isHitWall, hitResult, RAY_ID::CLIFF);

				//���ɐi��ŊR�ɗ������ꍇ�B
				CastRay(arg_newPos - m_transform.GetUp() * m_transform.GetScale().y, m_transform.GetFront(), m_transform.GetScale().x * 2.0f, modelMesh, terrian.m_transform, onGround, isHitWall, hitResult, RAY_ID::CLIFF);

				//�E�ɐi��ŊR�ɗ������ꍇ�B
				CastRay(arg_newPos - m_transform.GetUp() * m_transform.GetScale().y, m_transform.GetRight(), m_transform.GetScale().z * 2.0f, modelMesh, terrian.m_transform, onGround, isHitWall, hitResult, RAY_ID::CLIFF);

				//���ɐi��ŊR�ɗ������ꍇ�B
				CastRay(arg_newPos - m_transform.GetUp() * m_transform.GetScale().y, -m_transform.GetRight(), m_transform.GetScale().z * 2.0f, modelMesh, terrian.m_transform, onGround, isHitWall, hitResult, RAY_ID::CLIFF);

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
	return isHitWall;
}

Player::Player()
	:KuroEngine::Debugger("Player", true, true)
{
	//���f���ǂݍ���
	m_model = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Player.glb");
	m_axisModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Axis.glb");
	m_camModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Camera.glb");

	//�J��������
	m_cam = std::make_shared<KuroEngine::Camera>("Player's Camera");
	//�J�����̃R���g���[���[�ɃA�^�b�`
	m_camController.AttachCamera(m_cam);

	AddCustomParameter("MoveScalar", { "moveScalar" }, PARAM_TYPE::FLOAT, &m_moveScalar, "Player");
	AddCustomParameter("Sensitivity", { "camera", "sensitivity" }, PARAM_TYPE::FLOAT, &m_camSensitivity, "Camera");

	m_cameraRotY = 0;
	m_cameraQ = DirectX::XMQuaternionIdentity();
}

void Player::Init(KuroEngine::Transform arg_initTransform)
{
	m_transform = arg_initTransform;
	m_camController.Init();
	m_cameraRotY = 0;
	m_cameraQ = DirectX::XMQuaternionIdentity();
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
	auto moveVec = OperationConfig::Instance()->GetMoveVec(rotate);
	auto rowMoveVec = OperationConfig::Instance()->GetMoveVec(XMQuaternionIdentity());	//���̓��͕������擾�B�v���C���[����͕����ɉ�]������ۂɁAXZ���ʂł̒l���g�p����������B
	//���͂��ꂽ�����ړ��p�x�ʂ��擾
	auto scopeMove = OperationConfig::Instance()->GetScopeMove();

	//�J�����̉�]��ۑ��B
	m_cameraRotY += scopeMove.x;

	//�v���C���[�̉�]��ۑ��B���͂��������Ƃ��́B
	if (0 < rowMoveVec.Length()) {
		m_playerRotY = atan2f(rowMoveVec.x, rowMoveVec.z);
	}

	//�ړ��ʉ��Z
	newPos += moveVec * m_moveScalar;

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
		auto spinMat = DirectX::XMMatrixRotationQuaternion(spin);

		//�J�����ڐ���Y����]������N�H�[�^�j�I��
		auto ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_cameraRotY);

		//�J���������ł̃N�H�[�^�j�I�������߂�B�i�ޕ����Ȃǂ𔻒f����̂Ɏg�p����̂͂������BF�̈�ԍŏ��ɂ��̒l�����邱�Ƃ�playerYSpin�̉�]��ł������B
		m_cameraQ = DirectX::XMQuaternionMultiply(spin, ySpin);

		//�v���C���[�̈ړ�������Y����]������N�H�[�^�j�I���B�ړ������ɉ�]���Ă���悤�Ɍ��������邽�߂̂��́B
		auto playerYSpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_playerRotY);
		m_transform.SetRotate(DirectX::XMQuaternionMultiply(m_cameraQ, playerYSpin));

	}

	//���W�ω��K�p
	m_transform.SetPos(newPos);

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

	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model,
		m_transform);

	KuroEngine::DrawFunc3D::DrawNonShadingModel(
		m_axisModel,
		m_transform,
		arg_cam);

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


Player::MeshCollisionOutput Player::MeshCollision(const KuroEngine::Vec3<float>& arg_rayPos, const KuroEngine::Vec3<float>& arg_rayDir, KuroEngine::ModelMesh arg_targetMesh, KuroEngine::Transform arg_targetTransform) {


	/*===== ���b�V���ƃ��C�̓����蔻�� =====*/

	/*-- �@ ���f����񂩂瓖���蔻��p�̃|���S�������o�� --*/

	//�����蔻��p�|���S��
	struct Polygon {
		bool m_isActive;					//���̃|���S�����L��������Ă��邩�̃t���O
		KuroEngine::ModelMesh::Vertex m_p0;	//���_0
		KuroEngine::ModelMesh::Vertex m_p1;	//���_1
		KuroEngine::ModelMesh::Vertex m_p2;	//���_2
	};

	//�����蔻��p�|���S���R���e�i���쐬�B
	std::vector<Polygon> checkHitPolygons;
	checkHitPolygons.resize(arg_targetMesh.mesh->indices.size() / static_cast<size_t>(3));

	//�����蔻��p�|���S���R���e�i�Ƀf�[�^�����Ă����B
	for (auto& index : checkHitPolygons) {

		// ���݂�Index���B
		int nowIndex = static_cast<int>(&index - &checkHitPolygons[0]);

		// ���_����ۑ��B
		index.m_p0 = arg_targetMesh.mesh->vertices[arg_targetMesh.mesh->indices[nowIndex * 3 + 0]];
		index.m_p1 = arg_targetMesh.mesh->vertices[arg_targetMesh.mesh->indices[nowIndex * 3 + 1]];
		index.m_p2 = arg_targetMesh.mesh->vertices[arg_targetMesh.mesh->indices[nowIndex * 3 + 2]];

		// �|���S����L�����B
		index.m_isActive = true;

	}


	/*-- �A �|���S�������[���h�ϊ����� --*/

	//���[���h�s��
	DirectX::XMMATRIX targetRotMat = DirectX::XMMatrixRotationQuaternion(arg_targetTransform.GetRotate());
	DirectX::XMMATRIX targetWorldMat = DirectX::XMMatrixIdentity();
	targetWorldMat *= DirectX::XMMatrixScaling(arg_targetTransform.GetScale().x, arg_targetTransform.GetScale().y, arg_targetTransform.GetScale().z);
	targetWorldMat *= targetRotMat;
	targetWorldMat.r[3].m128_f32[0] = arg_targetTransform.GetPos().x;
	targetWorldMat.r[3].m128_f32[1] = arg_targetTransform.GetPos().y;
	targetWorldMat.r[3].m128_f32[2] = arg_targetTransform.GetPos().z;
	for (auto& index : checkHitPolygons) {
		//���_��ϊ�
		index.m_p0.pos = KuroEngine::Math::TransformVec3(index.m_p0.pos, targetWorldMat);
		index.m_p1.pos = KuroEngine::Math::TransformVec3(index.m_p1.pos, targetWorldMat);
		index.m_p2.pos = KuroEngine::Math::TransformVec3(index.m_p2.pos, targetWorldMat);
		//�@������]�s�񕪂����ϊ�
		index.m_p0.normal = KuroEngine::Math::TransformVec3(index.m_p0.normal, targetRotMat);
		index.m_p0.normal.Normalize();
		index.m_p1.normal = KuroEngine::Math::TransformVec3(index.m_p1.normal, targetRotMat);
		index.m_p1.normal.Normalize();
		index.m_p2.normal = KuroEngine::Math::TransformVec3(index.m_p2.normal, targetRotMat);
		index.m_p2.normal.Normalize();
	}

	/*-- �B �|���S����@���������ƂɃJ�����O���� --*/

	//�@���ƃ��C�̕����̓��ς�0��菬���������ꍇ�A���̃|���S���͔w�ʂȂ̂ŃJ�����O����B
	for (auto& index : checkHitPolygons) {

		if (index.m_p1.normal.Dot(arg_rayDir) < -0.0001f) continue;

		index.m_isActive = false;

	}


	/*-- �C �|���S���ƃ��C�̓����蔻����s���A�e�����L�^���� --*/

	// �L�^�p�f�[�^
	std::vector<std::pair<Player::MeshCollisionOutput, Polygon>> hitDataContainer;

	for (auto& index : checkHitPolygons) {

		//�|���S��������������Ă����玟�̏�����
		if (!index.m_isActive) continue;

		//���C�̊J�n�n�_���畽�ʂɂ��낵�������̒��������߂�
		KuroEngine::Vec3<float> planeNorm = -index.m_p0.normal;
		float rayToOriginLength = arg_rayPos.Dot(planeNorm);
		float planeToOriginLength = index.m_p0.pos.Dot(planeNorm);
		//���_���畽�ʂɂ��낵�������̒���
		float perpendicularLine = rayToOriginLength - planeToOriginLength;

		//�O�p�֐��𗘗p���Ď��_����Փ˓_�܂ł̋��������߂�
		float dist = planeNorm.Dot(arg_rayDir);
		float impDistance = perpendicularLine / -dist;

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


	/*-- �D �L�^������񂩂�ŏI�I�ȏՓ˓_�����߂� --*/

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

void Player::CastRay(KuroEngine::Vec3<float>& arg_rayPos, KuroEngine::Vec3<float>& arg_rayDir, float arg_rayLength, KuroEngine::ModelMesh arg_targetMesh, KuroEngine::Transform arg_targetTransform, bool& arg_onGround, bool& arg_isHitWall, HitCheckResult& arg_hitResult, RAY_ID arg_rayID)
{

	/*===== �����蔻��p�̃��C������ =====*/

	//���C���΂��B
	MeshCollisionOutput output = MeshCollision(arg_rayPos, arg_rayDir, arg_targetMesh, arg_targetTransform);

	//���C�����b�V���ɏՓ˂��Ă���A�Փ˒n�_�܂ł̋��������C�̒�����菬����������Փ˂��Ă���B
	if (output.m_isHit && std::fabs(output.m_distance) < arg_rayLength) {

		//�҂����艟���߂��Ă��܂��Əd�͂̊֌W�ŃK�N�K�N���Ă��܂��̂ŁA�����ɂ߂荞�܂��ĉ����߂��B
		static const float OFFSET = 0.01f;


		//���C�̎�ނɂ���ĕۑ�����f�[�^��ς���B
		switch (arg_rayID)
		{
		case Player::RAY_ID::GROUND:

			//�O���ɓn���p�̃f�[�^��ۑ�
			if (!arg_isHitWall) {/*
				arg_hitResult.m_interPos = output.m_pos;
				arg_hitResult.m_terrianNormal = output.m_normal;*/
			}
			arg_hitResult.m_interPos = output.m_pos;
			arg_hitResult.m_bottmRayTerrianNormal = output.m_normal;
			arg_onGround = true;
			//arg_isHitWall = true;

			//�����߂��B
			arg_rayPos += output.m_normal * (std::fabs(output.m_distance - arg_rayLength) - OFFSET);

			break;

		case Player::RAY_ID::CLIFF:

			//�O���ɓn���p�̃f�[�^��ۑ��B
			arg_isHitWall = true;
			arg_hitResult.m_interPos = output.m_pos;
			arg_hitResult.m_terrianNormal = output.m_normal;

			//���C�̏Փ˒n�_����@�������ɐL�΂����ʒu�Ɉړ�������B /2���Ă���̂̓��C�̒��������炩���ߓ�{�ɂ��Ă��邩��B
			arg_rayPos = output.m_pos + output.m_normal * (arg_rayLength / 2.0f - OFFSET);

			break;

		case Player::RAY_ID::AROUND:

			//�O���ɓn���p�̃f�[�^��ۑ��B
			arg_isHitWall = true;
			arg_hitResult.m_interPos = output.m_pos;
			arg_hitResult.m_terrianNormal = output.m_normal;

			//���C�̏Փ˒n�_����@�������ɐL�΂����ʒu�Ɉړ�������B
			arg_rayPos = output.m_pos + output.m_normal * (arg_rayLength - OFFSET);

			break;
		default:
			break;
		}
	}
}

