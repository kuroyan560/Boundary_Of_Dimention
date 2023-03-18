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

	}

	//�J����
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Camera"))
	{
		auto pos = m_cam->GetPos();
		auto target = m_cam->GetGazePointPos();

		// ImGui::DragFloat3("Target", (float*)&target, 0.5f);
		ImGui::DragFloat("Sensitivity", &m_camSensitivity, 0.05f);

		ImGui::TreePop();
	}
}

bool Player::HitCheck(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_to, const std::vector<Terrian>& arg_terrianArray, KuroEngine::Vec3<float>* arg_terrianNormal)
{
	/*
	arg_from �c �ړ��O�̍��W
	arg_to �c �ړ���̍��W
	arg_terrianArray �c �n�`�̔z��
	arg_terrianNormal �c ���������n�`�̃��b�V���̖@���A�i�[��
	*/

	//�����蔻�茋��
	bool isHit = false;
	m_onGround = false;
	KuroEngine::Vec3<float> hitNormal;

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
			CastRay(arg_to, m_transform.GetRight(), modelMesh, terrian.m_transform, isHit, hitNormal, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_to, -m_transform.GetRight(), modelMesh, terrian.m_transform, isHit, hitNormal, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_to, -m_transform.GetFront(), modelMesh, terrian.m_transform, isHit, hitNormal, RAY_ID::AROUND);

			//���ʕ����Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_to, m_transform.GetFront(), modelMesh, terrian.m_transform, isHit, hitNormal, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͒n�ʂƂ̉����߂��p�B
			CastRay(arg_to, -m_transform.GetUp(), modelMesh, terrian.m_transform, isHit, hitNormal, RAY_ID::GROUND);


			//=================================================
		}
	}

	//�����蔻�肪true�Ȃ瓖�������n�`�̖@�����i�[
	if (isHit && arg_terrianNormal)
	{
		*arg_terrianNormal = hitNormal;
	}
	return isHit;
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

	AddCustomParameter("MoveScalar", { "moveScalar" }, PARAM_TYPE::FLOAT, &m_moveScalar, "Player");
	AddCustomParameter("Sensitivity", { "camera", "sensitivity" }, PARAM_TYPE::FLOAT, &m_camSensitivity, "Camera");
}

void Player::Init(KuroEngine::Transform arg_initTransform)
{
	m_transform = arg_initTransform;
	m_camController.Init();
}

void Player::Update(const std::weak_ptr<Stage>arg_nowStage)
{
	using namespace KuroEngine;

	auto beforePos = m_transform.GetPos();
	auto newPos = beforePos;
	auto rotate = m_transform.GetRotate();

	//���͂��ꂽ�ړ��ʂ��擾
	auto moveVec = OperationConfig::Instance()->GetMoveVec(rotate);
	//���͂��ꂽ�����ړ��p�x�ʂ��擾
	auto scopeMove = OperationConfig::Instance()->GetScopeMove();

	//�ړ��ʉ��Z
	newPos += moveVec * m_moveScalar;

	if (!m_onGround) {
		newPos.y -= 0.2f;
	}

	//�����ړ��p�x�ʉ��Z�iY���F���E�j
	auto yScopeSpin = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), scopeMove.x);
	rotate = XMQuaternionMultiply(yScopeSpin, rotate);

	//�����蔻��
	Vec3<float>hitTerrianNormal;
	if (HitCheck(beforePos, newPos, arg_nowStage.lock()->GetTerrianArray(), &hitTerrianNormal))
	{
		//�����蔻��Ɋ�Â��Ĉړ����C��
	}

	//�g�����X�t�H�[���̕ω���K�p
	m_transform.SetPos(newPos);
	m_transform.SetRotate(rotate);

	//�J��������
	auto front = m_transform.GetFront();
	m_camController.Update(m_cam, newPos, front);

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
			camTransform.GetWorldMat(),
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

	/*-- �A �|���S����@���������ƂɃJ�����O���� --*/

	//�@���ƃ��C�̕����̓��ς�0��菬���������ꍇ�A���̃|���S���͔w�ʂȂ̂ŃJ�����O����B	�����蔻�����苭�x�ɂ��邽�߂Ɉ�U�R�����g�A�E�g���邪�A�������ז�肪���������畜��������B
	for (auto& index : checkHitPolygons) {

		if (index.m_p1.normal.Dot(arg_rayDir) < -0.0001f) continue;

		index.m_isActive = false;

	}


	/*-- �B �|���S�������[���h�ϊ����� --*/

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
		index.m_p0.pos = MulMat(index.m_p0.pos, targetWorldMat);
		index.m_p1.pos = MulMat(index.m_p1.pos, targetWorldMat);
		index.m_p2.pos = MulMat(index.m_p2.pos, targetWorldMat);
		//�@������]�s�񕪂����ϊ�
		index.m_p0.normal = MulMat(index.m_p0.normal, targetRotMat);
		index.m_p0.normal.Normalize();
		index.m_p1.normal = MulMat(index.m_p1.normal, targetRotMat);
		index.m_p1.normal.Normalize();
		index.m_p2.normal = MulMat(index.m_p2.normal, targetRotMat);
		index.m_p2.normal.Normalize();
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

		//�O�p�֐��𗘗p���Ď��_����Փ˓X�܂ł̋��������߂�
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

inline KuroEngine::Vec3<float> Player::CalBary(const KuroEngine::Vec3<float>& PosA, const KuroEngine::Vec3<float>& PosB, const KuroEngine::Vec3<float>& PosC, const KuroEngine::Vec3<float>& TargetPos)
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

inline KuroEngine::Vec3<float> Player::MulMat(const KuroEngine::Vec3<float>& arg_target, const DirectX::XMMATRIX arg_mat)
{

	/*===== �x�N�g���ɍs�����Z���� =====*/

	//��Z����B
	DirectX::XMVECTOR resultVec = DirectX::XMVector3Transform(arg_target, arg_mat);

	//KuroEngine::Vec3<float>�ɂȂ����B
	KuroEngine::Vec3<float> returnVec = { resultVec.m128_f32[0], resultVec.m128_f32[1], resultVec.m128_f32[2] };

	return returnVec;

}

void Player::CastRay(KuroEngine::Vec3<float>& arg_rayPos, KuroEngine::Vec3<float>& arg_rayDir, KuroEngine::ModelMesh arg_targetMesh, KuroEngine::Transform arg_targetTransform, bool& arg_isHit, KuroEngine::Vec3<float>& arg_hitNormal, RAY_ID arg_rayID)
{

	/*===== �����蔻��p�̃��C������ =====*/

	//���C���΂��B
	MeshCollisionOutput output = MeshCollision(arg_rayPos, arg_rayDir, arg_targetMesh, arg_targetTransform);

	//���C�����b�V���ɏՓ˂��Ă���A�Փ˒n�_�܂ł̋������v���C���[�̑傫����菬����������Փ˂��Ă���B
	if (output.m_isHit && std::fabs(output.m_distance) <= m_transform.GetScale().x) {

		//�҂����艟���߂��Ă��܂��Əd�͂̊֌W�ŃK�N�K�N���Ă��܂��̂ŁA�����ɂ߂荞�܂��ĉ����߂��B
		static const float OFFSET = 0.01f;

		//�����߂��B
		arg_rayPos = output.m_pos + output.m_normal * (m_transform.GetScale().x - OFFSET);

		//���C�̎�ނɂ���ĕۑ�����f�[�^��ς���B
		switch (arg_rayID)
		{
		case Player::RAY_ID::GROUND:

			//�ڒn����
			m_onGround = true;

			break;
		case Player::RAY_ID::AROUND:

			//�O���ɓn���p�̃f�[�^��ۑ��B
			arg_isHit = true;
			arg_hitNormal = output.m_normal;

			break;
		default:
			break;
		}

	}

}
