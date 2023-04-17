#include "Player.h"
#include"Render/RenderObject/Camera.h"
#include"../OperationConfig.h"
#include"FrameWork/Importer.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"../Graphics/BasicDraw.h"
#include"../Stage/Stage.h"
#include"../Graphics/BasicDrawParameters.h"
#include"../../../../src/engine/ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"FrameWork/UsersInput.h"

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

bool Player::HitCheckAndPushBack(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo)
{
	/*
	arg_from �c �ړ��O�̍��W
	arg_to �c �ړ���̍��W
	arg_terrianArray �c �n�`�̔z��
	arg_terrianNormal �c ���������n�`�̃��b�V���̖@���A�i�[��
	*/

	//CastRay�ɓn������
	Player::CastRayArgument castRayArgument;
	castRayArgument.m_onGround = false;
	castRayArgument.m_stageType = StageParts::TERRIAN;
	for (auto& index : castRayArgument.m_checkDeathCounter) {
		index = 0;
	}

	//�M�~�b�N�ɓ������Ă��邩�ǂ����̕ϐ���������
	m_prevOnGimmick = m_onGimmick;
	m_onGimmick = false;

	//���͂̕ǂƂ̓����蔻��
	CheckHitAround(arg_from, arg_newPos, arg_nowStage, arg_hitInfo, castRayArgument);

	//�W�����v���͒n�ʂƂ̓����蔻����s��Ȃ�(�W�����v��̏�x�N�g�������̒n�ʂ̏�x�N�g���ɂȂ��Ă��܂�����)
	if (m_playerMoveStatus == PLAYER_MOVE_STATUS::JUMP) return true;

	//�n�ʂƂ̓����蔻��
	CheckHitGround(arg_from, arg_newPos, arg_nowStage, arg_hitInfo, castRayArgument);

	//���񂾂�(���܂��Ă��邩)�ǂ����𔻒�
	CheckDeath(arg_from, arg_newPos, arg_nowStage, arg_hitInfo, castRayArgument);

	//����ł����珈�����΂��B
	m_isDeath = false;
	m_isDeath |= (castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::RIGHT)] && castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::LEFT)]);
	m_isDeath |= (castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::TOP)] && castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::BOTTOM)]);
	m_isDeath |= (castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::FRONT)] && castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::BEHIND)]);
	if (m_isDeath) {
		return false;
	}

	return true;
}

void Player::CheckDeath(const KuroEngine::Vec3<float> arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, Player::CastRayArgument& arg_castRayArgment)
{

	//�n�`�z�񑖍�
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//���f�����擾
		auto model = terrian.m_model.lock();
		//�����擾�B
		arg_castRayArgment.m_stageType = StageParts::TERRIAN;

		//���b�V���𑖍�
		for (auto& modelMesh : model->m_meshes)
		{
			//���b�V�����擾
			auto& mesh = modelMesh.mesh;

			//CastRay�ɓn���������X�V�B
			arg_castRayArgment.m_mesh = terrian.m_collisionMesh[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//���聫============================================

			//�E�����Ƀ��C���΂��B
			CastRay(arg_newPos, arg_newPos, m_transform.GetRight(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::RIGHT);

			//�������Ƀ��C���΂��B
			CastRay(arg_newPos, arg_newPos, -m_transform.GetRight(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::LEFT);

			//�������Ƀ��C���΂��B
			CastRay(arg_newPos, arg_newPos, -m_transform.GetFront(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::BEHIND);

			//���ʕ����Ƀ��C���΂��B
			CastRay(arg_newPos, arg_newPos, m_transform.GetFront(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::FRONT);

			//������Ƀ��C���΂��B
			CastRay(arg_newPos, arg_newPos, m_transform.GetUp(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::TOP);

			//�������Ƀ��C���΂��B
			CastRay(arg_newPos, arg_newPos, -m_transform.GetUp(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::BOTTOM);

			//=================================================
		}
	}

	//�M�~�b�N�Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//���f�����擾
		auto model = terrian->GetModel();
		//�����擾�B
		arg_castRayArgment.m_stageType = terrian->GetType();
		//�X�e�[�W����ۑ��B
		arg_castRayArgment.m_stage = terrian;

		//���b�V���𑖍�
		for (auto& modelMesh : model.lock()->m_meshes)
		{
			//���b�V�����擾
			auto& mesh = modelMesh.mesh;

			//CastRay�ɓn���������X�V�B
			arg_castRayArgment.m_mesh = terrian->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//���聫============================================

			//�ړ����������v���C���[���痣�������ɓ����Ă����玀�S������΂��B
			if (terrian->GetType() == StageParts::MOVE_SCAFFOLD) {

				float oldPosDistance = (arg_newPos - terrian->GetOldPos()).Length();
				float nowPosDistance = (arg_newPos - terrian->GetNowPos()).Length();

				if (oldPosDistance < nowPosDistance) {
					continue;
				}

			}

			//�E�����Ƀ��C���΂��B
			CastRay(arg_newPos, arg_newPos, m_transform.GetRight(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::RIGHT);

			//�������Ƀ��C���΂��B
			CastRay(arg_newPos, arg_newPos, -m_transform.GetRight(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::LEFT);

			//�������Ƀ��C���΂��B
			CastRay(arg_newPos, arg_newPos, -m_transform.GetFront(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::BEHIND);

			//���ʕ����Ƀ��C���΂��B
			CastRay(arg_newPos, arg_newPos, m_transform.GetFront(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::FRONT);

			//������Ƀ��C���΂��B
			CastRay(arg_newPos, arg_newPos, m_transform.GetUp(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::TOP);

			//�������Ƀ��C���΂��B
			CastRay(arg_newPos, arg_newPos, -m_transform.GetUp(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::BOTTOM);

			//=================================================
		}
	}

}

void Player::CheckHitAround(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, Player::CastRayArgument& arg_castRayArgment) {

	//�n�`�z�񑖍�
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//���f�����擾
		auto model = terrian.m_model.lock();
		//�����擾�B
		arg_castRayArgment.m_stageType = StageParts::TERRIAN;

		//���b�V���𑖍�
		for (auto& modelMesh : model->m_meshes)
		{
			//���b�V�����擾
			auto& mesh = modelMesh.mesh;

			//CastRay�ɓn���������X�V�B
			arg_castRayArgment.m_mesh = terrian.m_collisionMesh[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//���聫============================================

			//�E�����Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_newPos, arg_newPos, m_transform.GetRight(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_newPos, arg_newPos, -m_transform.GetRight(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_newPos, arg_newPos, -m_transform.GetFront(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//���ʕ����Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_newPos, arg_newPos, m_transform.GetFront(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//=================================================
		}
	}

	//�M�~�b�N�Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//���f�����擾
		auto model = terrian->GetModel();
		//�����擾�B
		arg_castRayArgment.m_stageType = terrian->GetType();
		//�X�e�[�W����ۑ��B
		arg_castRayArgment.m_stage = terrian;

		//���b�V���𑖍�
		for (auto& modelMesh : model.lock()->m_meshes)
		{
			//���b�V�����擾
			auto& mesh = modelMesh.mesh;

			//CastRay�ɓn���������X�V�B
			arg_castRayArgment.m_mesh = terrian->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//���聫============================================

			//�E�����Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_newPos, arg_newPos, m_transform.GetRight(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_newPos, arg_newPos, -m_transform.GetRight(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_newPos, arg_newPos, -m_transform.GetFront(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//���ʕ����Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(arg_newPos, arg_newPos, m_transform.GetFront(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//=================================================
		}
	}

	//���͂̏Փ˓_����������A����̍ŒZ���������߂ăW�����v������߂�B
	int impactPointSize = static_cast<int>(arg_castRayArgment.m_impactPoint.size());
	if (0 < impactPointSize) {

		//�S�Ă̏Փ˓_�ɂ��ĊR�𒴂��Ă��Ȃ������`�F�b�N����B
		for (auto& index : arg_castRayArgment.m_impactPoint) {

			//�R�`�F�b�N
			CheckCliff(index, arg_nowStage);

		}

		//�ŏ��l�ۑ��p�ϐ�
		float minDistance = std::numeric_limits<float>().max();
		int minIndex = -1;

		//�S�Փ˓_�̒�����ŒZ�̈ʒu�ɂ�����̂���������B
		for (auto& index : arg_castRayArgment.m_impactPoint) {

			//�Փ˓_���L��������Ă��Ȃ������珈�����΂��B
			if (!index.m_isActive) continue;

			//�������ۑ�����Ă������傫�������珈�����΂��B
			float distance = (arg_newPos - index.m_impactPos).Length();
			if (minDistance < distance) continue;

			//������C���f�b�N�X�̃f�[�^��ۑ��B
			minDistance = distance;
			minIndex = static_cast<int>(&index - &arg_castRayArgment.m_impactPoint[0]);

		}

		//�n�_���ۑ�����Ă��Ȃ������珈�����΂��B
		if (minIndex != -1) {

			//�W�����v���ł����Ԃ�������W�����v����B
			if (m_canJump) {

				//�ŒZ�̏Փ˓_�����߂���A������W�����v��ɂ���B
				arg_hitInfo->m_terrianNormal = arg_castRayArgment.m_impactPoint[minIndex].m_normal;

				//�W�����v�̃p�����[�^�[�����߂�B
				m_playerMoveStatus = PLAYER_MOVE_STATUS::JUMP;
				m_jumpTimer = 0;
				m_jumpStartPos = arg_newPos;
				m_bezierCurveControlPos = m_jumpStartPos + m_transform.GetUp() * WALL_JUMP_LENGTH;
				m_jumpEndPos = arg_castRayArgment.m_impactPoint[minIndex].m_impactPos + arg_castRayArgment.m_impactPoint[minIndex].m_normal * m_transform.GetScale().x;
				m_jumpEndPos += m_transform.GetUp() * WALL_JUMP_LENGTH;

				//�W�����v�����̂Ń^�C�}�[��������
				m_canJumpDelayTimer = 0;

			}
			//�W�����v���ł��Ȃ���Ԃ������牟���߂��B
			else {

				arg_newPos = arg_from + m_gimmickVel;
				arg_hitInfo->m_terrianNormal = m_transform.GetUp();

				//�����|����̃^�C�}�[���X�V ���̒l�����l����������W�����v�ł���悤�ɂȂ�
				++m_canJumpDelayTimer;

			}

			return;

		}

	}



	//�ǂ��Ƃ��������Ă��Ȃ������猻�݂̏�x�N�g����n�`�̏�x�N�g���Ƃ��Ă݂�B
	arg_hitInfo->m_terrianNormal = m_transform.GetUp();

	//�ǂ��ɂ������������ĂȂ��̂Ń^�C�}�[������������B
	m_canJumpDelayTimer = 0;


}

void Player::CheckHitGround(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, Player::CastRayArgument& arg_castRayArgment) {

	//�n�`�z�񑖍�
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//���f�����擾
		auto model = terrian.m_model.lock();
		//�����擾�B
		arg_castRayArgment.m_stageType = StageParts::TERRIAN;

		//���b�V���𑖍�
		for (auto& modelMesh : model->m_meshes)
		{
			//���b�V�����擾
			auto& mesh = modelMesh.mesh;

			//CastRay�ɓn���������X�V�B
			arg_castRayArgment.m_mesh = terrian.m_collisionMesh[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//���聫============================================

			//�������Ƀ��C���΂��B����͒n�ʂƂ̉����߂��p�B
			CastRay(arg_newPos, arg_newPos, -m_transform.GetUp(), m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);

			//=================================================
		}
	}
	//�M�~�b�N�Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//���f�����擾
		auto model = terrian->GetModel();
		//�����擾�B
		arg_castRayArgment.m_stageType = terrian->GetType();
		//�X�e�[�W����ۑ��B
		arg_castRayArgment.m_stage = terrian;

		//���b�V���𑖍�
		for (auto& modelMesh : model.lock()->m_meshes)
		{
			//���b�V�����擾
			auto& mesh = modelMesh.mesh;

			//CastRay�ɓn���������X�V�B
			arg_castRayArgment.m_mesh = terrian->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//���聫============================================

			//�v���C���[�������������Ƀ��C���΂��ʒu�����炷�B
			KuroEngine::Vec3<float> moveVec = (arg_newPos - arg_from);

			//�������Ƀ��C���΂��B����͒n�ʂƂ̉����߂��p�B
			CastRay(arg_newPos, arg_newPos, -m_transform.GetUp(), m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);

			//���炵�������ɖ߂��B
			arg_newPos - moveVec;

			//=================================================
		}
	}

	//�ڒn�t���O��ۑ�
	m_prevOnGround = m_onGround;
	m_onGround = arg_castRayArgment.m_onGround;

	//�܂����n���Ă��Ȃ������璅�n�ɂ���B
	if (!m_isFirstOnGround && m_onGround) {
		m_isFirstOnGround = true;
	}

	//�ڒn���Ă��Ȃ�������ړ��O�̈ʒu�ɖ߂��B
	if (m_isFirstOnGround && !m_onGround) {
		//�M�~�b�N�̏�ɂ����炻����l�������ʒu�ɖ߂��B
		if (0 < m_gimmickVel.Length()) {
			arg_newPos = arg_from + m_gimmickVel;
		}
		else {
			arg_newPos = arg_from;
		}
		//�O�t���[���ŃM�~�b�N�̏�ɂ�����A�����M�~�b�N�ɂ��锻��ɂ���B(�������Ȃ��ƈړ��̃M�~�b�N�̏�ɂ���Ƃ��ɕǍۂɍs���ƈ�u�~�肽����ɂȂ�A�M�~�b�N���ғ����Ă��܂�����B)
		if (m_prevOnGimmick) m_onGimmick = true;
		//���ɕ�Ԃ����x�N�g���ƃv���C���[�̏�x�N�g���ɂ���B
		arg_hitInfo->m_terrianNormal = m_transform.GetUp();
	}

}

void Player::CheckCliff(Player::ImpactPointData& arg_impactPointData, std::weak_ptr<Stage> arg_nowStage)
{

	//�L��������Ă��Ȃ������珈�����΂��B
	if (!arg_impactPointData.m_isActive) return;

	//�R����p�̃��C�̒���
	const float CLIFF_RAY_LENGTH = WALL_JUMP_LENGTH + m_transform.GetScale().Length();

	//�n�`�z�񑖍�
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//���f�����擾
		auto model = terrian.m_model.lock();

		//���b�V���𑖍�
		for (auto& modelMesh : model->m_meshes)
		{

			//���聫============================================

			//�����蔻����s�����b�V���B
			std::vector<StageParts::Polygon> mesh = terrian.m_collisionMesh[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//�������Ƀ��C���΂��B
			MeshCollisionOutput output = MeshCollision(arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * 0.5f, -m_transform.GetUp(), mesh);

			//���C�����b�V���ɏՓ˂��Ă���A�Փ˒n�_�܂ł̋��������C�̒�����菬����������Փ˂��Ă���B
			if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

				//�ǂɓ����������_�ŊR�ł͂Ȃ��̂ŏ������΂��B
				return;

			}

			//=================================================
		}
	}

	//�M�~�b�N�Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//���f�����擾
		auto model = terrian->GetModel();

		//���b�V���𑖍�
		for (auto& modelMesh : model.lock()->m_meshes) {

			//���聫============================================

			//�����蔻����s�����b�V���B
			std::vector<StageParts::Polygon> mesh = terrian->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//�������Ƀ��C���΂��B
			MeshCollisionOutput output = MeshCollision(arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * 0.5f, -m_transform.GetUp(), mesh);

			//���C�����b�V���ɏՓ˂��Ă���A�Փ˒n�_�܂ł̋��������C�̒�����菬����������Փ˂��Ă���B
			if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

				//�ǂɓ����������_�ŊR�ł͂Ȃ��̂ŏ������΂��B
				return;

			}

			//=================================================
		}
	}

	//�Ō�܂ŕǂɓ������ĂȂ�������R�𒴂��Ă���̂Ŗ���������B
	arg_impactPointData.m_isActive = false;

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
	m_isFirstOnGround = false;
	m_onGimmick = false;
	m_prevOnGimmick = false;

}

void Player::Init(KuroEngine::Transform arg_initTransform)
{
	m_transform = arg_initTransform;
	m_camController.Init();
	m_cameraRotY = 0;
	m_cameraRotYStorage = 0;
	m_cameraJumpLerpAmount = 0;
	m_cameraJumpLerpStorage = 0;
	m_cameraQ = DirectX::XMQuaternionIdentity();
	m_canJumpDelayTimer = 0;

	m_moveSpeed = KuroEngine::Vec3<float>();
	m_gimmickVel = KuroEngine::Vec3<float>();
	m_isFirstOnGround = false;
	m_onGimmick = false;
	m_isCameraModeFar = true;
	m_prevOnGimmick = false;
	m_isDeath = false;
	m_playerMoveStatus = PLAYER_MOVE_STATUS::MOVE;
}

void Player::Update(const std::weak_ptr<Stage>arg_nowStage)
{
	using namespace KuroEngine;

	//�g�����X�t�H�[����ۑ��B
	m_prevTransform = m_transform;

	//�ʒu���֌W
	auto beforePos = m_transform.GetPos();
	auto newPos = beforePos;

	//���͂��ꂽ�����ړ��p�x�ʂ��擾
	auto scopeMove = OperationConfig::Instance()->GetScopeMove();

	//�W�����v���ł��邩�ǂ����B	��莞�Ԓn�`�Ɉ����|�����Ă���W�����v�ł���B
	m_canJump = CAN_JUMP_DELAY <= m_canJumpDelayTimer;

	//�J�������[�h��؂�ւ���B
	if (UsersInput::Instance()->KeyOffTrigger(DIK_RETURN) || UsersInput::Instance()->ControllerOnTrigger(0, KuroEngine::X)) {
		m_isCameraModeFar = m_isCameraModeFar ? false : true;
	}

	//�ړ��X�e�[�^�X�ɂ���ď�����ς���B
	switch (m_playerMoveStatus)
	{
	case Player::PLAYER_MOVE_STATUS::MOVE:
	{

		//�J�����̉�]��ۑ��B
		m_cameraRotYStorage += scopeMove.x;

		//�v���C���[�̉�]���J������ɂ���B(�ړ������̊���J�����̊p�x�Ȃ���)
		m_transform.SetRotate(m_cameraQ);

		//���͂��ꂽ�ړ��ʂ��擾
		m_rowMoveVec = OperationConfig::Instance()->GetMoveVecFuna(XMQuaternionIdentity());	//���̓��͕������擾�B�v���C���[����͕����ɉ�]������ۂɁAXZ���ʂł̒l���g�p����������B

		//���͗ʂ����ȉ���������0�ɂ���B
		const float DEADLINE = 0.8f;
		if (m_rowMoveVec.Length() <= DEADLINE) {
			m_rowMoveVec = {};
		}

		//�V��ɂ�����
		if (m_transform.GetUp().y < -0.9f) {
			//X�̈ړ������𔽓]
			m_rowMoveVec.x *= -1.0f;
		}

		//�ړ�������B
		Move(newPos);

		//���͂��Ȃ�������
		if (m_rowMoveVec.Length() <= 0) {

			//�J�����̉�]��ۑ��B
			m_cameraRotY = m_cameraRotYStorage;

		}
		else {

			//�ړ�����������ۑ��B
			m_playerRotY = atan2f(m_rowMoveVec.x, m_rowMoveVec.z);

		}

		//�����蔻��
		CheckHit(beforePos, newPos, arg_nowStage);

	}
	break;
	case Player::PLAYER_MOVE_STATUS::JUMP:
	{

		//�^�C�}�[���X�V�B
		m_jumpTimer = std::clamp(m_jumpTimer + JUMP_TIMER, 0.0f, 1.0f);

		float easeAmount = KuroEngine::Math::Ease(Out, Sine, m_jumpTimer, 0.0f, 1.0f);

		//�J�����̉�]���Ԃ���B
		m_cameraRotYStorage = m_cameraJumpLerpStorage + easeAmount * m_cameraJumpLerpAmount;
		m_cameraRotY = m_cameraJumpLerpStorage + easeAmount * m_cameraJumpLerpAmount;

		//���W���Ԃ���B
		newPos = CalculateBezierPoint(easeAmount, m_jumpStartPos, m_jumpEndPos, m_bezierCurveControlPos);

		//��]��⊮����B
		m_transform.SetRotate(DirectX::XMQuaternionSlerp(m_jumpStartQ, m_jumpEndQ, easeAmount));

		//����ɒB���Ă�����W�����v���I����B
		if (1.0f <= m_jumpTimer) {
			m_playerMoveStatus = PLAYER_MOVE_STATUS::MOVE;
			m_cameraJumpLerpAmount = 0;
		}

	}
	break;
	default:
		break;
	}


	//���W�ω��K�p
	m_transform.SetPos(newPos);
	m_ptLig.SetPos(newPos);

	//�J��������
	m_camController.Update(scopeMove, m_transform.GetPosWorld(), m_cameraRotYStorage, m_isCameraModeFar ? CAMERA_MODE_FAR : CAMERA_MODE_NEAR);

	//�M�~�b�N�̈ړ���ł������B
	m_gimmickVel = KuroEngine::Vec3<float>();
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

	BasicDraw::Instance()->Draw_Player(
		arg_cam,
		arg_ligMgr,
		m_model,
		m_transform,
		drawParam);

	/*
	KuroEngine::DrawFunc3D::DrawNonShadingModel(
		m_axisModel,
		m_transform,
		arg_cam);
	*/

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

Player::MeshCollisionOutput Player::MeshCollision(const KuroEngine::Vec3<float>& arg_rayPos, const KuroEngine::Vec3<float>& arg_rayDir, std::vector<Terrian::Polygon>& arg_targetMesh) {


	/*===== ���b�V���ƃ��C�̓����蔻�� =====*/


	/*-- �@ �|���S����@���������ƂɃJ�����O���� --*/

	//�@���ƃ��C�̕����̓��ς�0���傫�������ꍇ�A���̃|���S���͔w�ʂȂ̂ŃJ�����O����B
	for (auto& index : arg_targetMesh) {

		index.m_isActive = true;

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

bool Player::CastRay(KuroEngine::Vec3<float>& arg_charaPos, const KuroEngine::Vec3<float>& arg_rayCastPos, const KuroEngine::Vec3<float>& arg_rayDir, float arg_rayLength, CastRayArgument& arg_collisionData, RAY_ID arg_rayID, RAY_DIR_ID arg_rayDirID)
{

	/*===== �����蔻��p�̃��C������ =====*/

	//���C���΂��B
	MeshCollisionOutput output = MeshCollision(arg_rayCastPos, arg_rayDir, arg_collisionData.m_mesh);

	//���C�����b�V���ɏՓ˂��Ă���A�Փ˒n�_�܂ł̋��������C�̒�����菬����������Փ˂��Ă���B
	if (output.m_isHit && std::fabs(output.m_distance) < arg_rayLength) {

		//�҂����艟���߂��Ă��܂��Əd�͂̊֌W�ŃK�N�K�N���Ă��܂��̂ŁA�����ɂ߂荞�܂��ĉ����߂��B
		static const float OFFSET = 0.01f;

		//���C�̎�ނɂ���ĕۑ�����f�[�^��ς���B
		switch (arg_rayID)
		{
		case Player::RAY_ID::GROUND:

			//�O���ɓn���p�̃f�[�^��ۑ�
			arg_collisionData.m_bottomTerrianNormal = output.m_normal;
			arg_collisionData.m_onGround = true;

			//�����߂��B
			arg_charaPos += output.m_normal * (std::fabs(output.m_distance - arg_rayLength) - OFFSET);

			//�n�`����������������L��������B
			if (arg_collisionData.m_stageType == StageParts::MOVE_SCAFFOLD) {

				//�M�~�b�N�ɓ������Ă��锻��
				m_onGimmick = true;

				//����ɃM�~�b�N�ɓ��������g���K�[��������M�~�b�N��L����������B
				if (!m_prevOnGimmick) {
					arg_collisionData.m_stage.lock()->Activate();
				}

			}

			break;

		case Player::RAY_ID::AROUND:

			//���C�̏Փ˒n�_��ۑ��B
			arg_collisionData.m_impactPoint.emplace_back(ImpactPointData(output.m_pos, output.m_normal));

			//��������������߂荞��ł��܂��̂ŉ����߂��B
			if (arg_collisionData.m_stageType == StageParts::MOVE_SCAFFOLD) {

				arg_charaPos += output.m_normal * (std::fabs(output.m_distance - arg_rayLength) - OFFSET);

			}

			break;

		case Player::RAY_ID::CHECK_DEATH:

			arg_collisionData.m_checkDeathCounter[static_cast<int>(arg_rayDirID)] = true;

			break;


		default:
			break;
		}

		//��������
		return true;


	}
	else {

		//������Ȃ�����
		return false;

	}

}

void Player::Move(KuroEngine::Vec3<float>& arg_newPos) {

	//�������͓��͂𖳌����B
	if (!m_onGround) {
		m_rowMoveVec = KuroEngine::Vec3<float>();
	}
	m_moveSpeed = m_rowMoveVec * m_maxSpeed;

	//�ړ����x���N�����v�B
	m_moveSpeed.x = std::clamp(m_moveSpeed.x, -m_maxSpeed, m_maxSpeed);
	m_moveSpeed.z = std::clamp(m_moveSpeed.z, -m_maxSpeed, m_maxSpeed);

	//���͂��ꂽ�l������������ړ����x�����炷�B
	if (std::fabs(m_rowMoveVec.x) < 0.001f) {

		m_moveSpeed.x = 0;

	}
	else {
		int a = 0;
	}

	if (std::fabs(m_rowMoveVec.z) < 0.001f) {

		m_moveSpeed.z = 0;

	}

	//���[�J�����̈ړ��������v���C���[�̉�]�ɍ��킹�ē������B
	auto moveAmount = KuroEngine::Math::TransformVec3(m_moveSpeed, m_transform.GetRotate());

	//�ړ��ʉ��Z
	arg_newPos += moveAmount;

	//�M�~�b�N�̈ړ��ʂ����Z�B
	arg_newPos += m_gimmickVel;

	//�n�ʂɒ���t����p�̏d�́B
	if (!m_onGround) {
		arg_newPos -= m_transform.GetUp() * (m_transform.GetScale().y / 2.0f);
	}

}

void Player::CheckHit(KuroEngine::Vec3<float>& arg_frompos, KuroEngine::Vec3<float>& arg_nowpos, std::weak_ptr<Stage>arg_nowStage) {

	HitCheckResult hitResult;
	if (!HitCheckAndPushBack(arg_frompos, arg_nowpos, arg_nowStage, &hitResult))return;

	//�n�`�̖@�����^���������Ă���Ƃ��Ɍ덷�ł��ꂢ��0,-1,0�ɂȂ��Ă���Ȃ������ł��܂������Ȃ��̂ŋ���̍�B
	if (hitResult.m_terrianNormal.y < -0.9f) {
		hitResult.m_terrianNormal = { 0,-1,0 };
	}

	//�J��������������B
	AdjustCaneraRotY(m_transform.GetUp(), hitResult.m_terrianNormal);

	//�@������������N�H�[�^�j�I��
	m_normalSpinQ = KuroEngine::Math::GetLookAtQuaternion({ 0,1,0 }, hitResult.m_terrianNormal);

	//�J�����̉�]��Y����]������N�H�[�^�j�I���B�ړ������ɉ�]���Ă���悤�Ɍ��������邽�߂̂��́Bm_cameraJumpLerpAmount�͕�Ԍ�̃J�����Ɍ������ĕ�Ԃ��邽�߁B
	DirectX::XMVECTOR ySpin;
	if (hitResult.m_terrianNormal.y < -0.9f) {
		ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, -(m_cameraRotY + m_cameraJumpLerpAmount) + DirectX::XM_PI);
	}
	else {
		ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_cameraRotY + m_cameraJumpLerpAmount);
	}

	//�v���C���[�̈ړ�������Y����]������N�H�[�^�j�I���B�ړ������ɉ�]���Ă���悤�Ɍ��������邽�߂̂��́B
	DirectX::XMVECTOR playerYSpin;
	playerYSpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_playerRotY);

	//�J���������ł̃N�H�[�^�j�I�������߂�B�i�ޕ����Ȃǂ𔻒f����̂Ɏg�p����̂͂������BF�̈�ԍŏ��ɂ��̒l�����邱�Ƃ�playerYSpin�̉�]��ł������B
	m_cameraQ = DirectX::XMQuaternionMultiply(m_normalSpinQ, ySpin);

	//�v���C���[�̈ړ�������Y����]������N�H�[�^�j�I�����J�����̃N�H�[�^�j�I���ɂ����āA�v���C���[���ړ������Ɍ�������B
	m_moveQ = DirectX::XMQuaternionMultiply(m_cameraQ, playerYSpin);

	//�W�����v��Ԃ�������
	if (m_playerMoveStatus == PLAYER_MOVE_STATUS::JUMP) {

		//�W�����v��ɉ�]����悤�ɂ���B

		//�N�H�[�^�j�I����ۑ��B
		m_jumpEndQ = m_moveQ;
		m_jumpStartQ = m_prevTransform.GetRotate();
		m_transform.SetRotate(m_prevTransform.GetRotate());

	}
	else {

		//���������ʊ�̉�]�ɂ���B
		m_transform.SetRotate(m_moveQ);

	}

}

KuroEngine::Vec3<float> Player::CalculateBezierPoint(float arg_time, KuroEngine::Vec3<float> arg_startPoint, KuroEngine::Vec3<float> arg_endPoint, KuroEngine::Vec3<float> arg_controlPoint) {

	float oneMinusT = 1.0f - arg_time;
	float oneMinusTSquared = oneMinusT * oneMinusT;
	float tSquared = arg_time * arg_time;

	float x = oneMinusTSquared * arg_startPoint.x + 2 * oneMinusT * arg_time * arg_controlPoint.x + tSquared * arg_endPoint.x;
	float y = oneMinusTSquared * arg_startPoint.y + 2 * oneMinusT * arg_time * arg_controlPoint.y + tSquared * arg_endPoint.y;
	float z = oneMinusTSquared * arg_startPoint.z + 2 * oneMinusT * arg_time * arg_controlPoint.z + tSquared * arg_endPoint.z;

	return KuroEngine::Vec3<float>(x, y, z);

}

void Player::AdjustCaneraRotY(const KuroEngine::Vec3<float>& arg_nowUp, const KuroEngine::Vec3<float>& arg_nextUp) {

	// �ړ��������������邽�߂̋���̍�

	// ����:���̊֐��ŏ����Ă�������͏����ʒu(�@��(0,1,0)��(0,0,1)�������Ă�����)�ł̂��̂ł��B

	//��]���̒l��ۑ��B
	m_cameraJumpLerpStorage = m_cameraRotYStorage;

	//�p�x���ς���ĂȂ��������΂��B
	if (0.9f <= arg_nowUp.Dot(arg_nextUp)) return;

	//�v���C���[���E���̕ǂɂ���ꍇ
	if (arg_nowUp.x <= -0.9f) {

		//��̕ǂɈړ�������
		if (arg_nextUp.y <= -0.9f) {

			m_cameraJumpLerpAmount += DirectX::XM_PI;

		}
		//���ʂ̕ǂɈړ�������
		if (arg_nextUp.z <= -0.9f) {

			m_cameraJumpLerpAmount -= DirectX::XM_PIDIV2;

		}
		//���̕ǂɈړ�������
		if (0.9f <= arg_nextUp.z) {

			m_cameraJumpLerpAmount += DirectX::XM_PIDIV2;

		}

	}

	//�v���C���[�������̕ǂɂ���ꍇ
	if (0.9f <= arg_nowUp.x) {

		//��̕ǂɈړ�������
		if (arg_nextUp.y <= -0.9f) {

			m_cameraJumpLerpAmount -= DirectX::XM_PI;

		}
		//���ʂ̕ǂɈړ�������
		if (arg_nextUp.z <= -0.9f) {

			m_cameraJumpLerpAmount += DirectX::XM_PIDIV2;

		}
		//���̕ǂɈړ�������
		if (0.9f <= arg_nextUp.z) {

			m_cameraJumpLerpAmount -= DirectX::XM_PIDIV2;

		}

	}

	//�v���C���[�����ʂ̕ǂɂ���ꍇ
	if (arg_nowUp.z <= -0.9f) {

		//�E���̕ǂɈړ�������
		if (arg_nextUp.x <= -0.9f) {

			m_cameraJumpLerpAmount += DirectX::XM_PIDIV2;

		}
		//�����̕ǂɈړ�������
		if (0.9f <= arg_nextUp.x) {

			m_cameraJumpLerpAmount -= DirectX::XM_PIDIV2;

		}

	}

	//�v���C���[����둤�̕ǂɂ���ꍇ
	if (0.9f <= arg_nowUp.z) {

		//�E���̕ǂɈړ�������
		if (arg_nextUp.x <= -0.9f) {

			m_cameraJumpLerpAmount -= DirectX::XM_PIDIV2;

		}
		//�����̕ǂɈړ�������
		if (0.9f <= arg_nextUp.x) {

			m_cameraJumpLerpAmount += DirectX::XM_PIDIV2;

		}

	}

	//�v���C���[���㑤�̕ǂɂ���ꍇ
	if (arg_nowUp.y <= -0.9f) {

		//�E���̕ǂɈړ�������
		if (arg_nextUp.x <= -0.9f) {

			m_cameraJumpLerpAmount -= DirectX::XM_PI;

		}
		//�����̕ǂɈړ�������
		if (0.9f <= arg_nextUp.x) {

			m_cameraJumpLerpAmount += DirectX::XM_PI;

		}

	}

}