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
#include"../SoundConfig.h"

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
	castRayArgument.m_stageType = StageParts::TERRIAN;
	for (auto& index : castRayArgument.m_checkDeathCounter) {
		index = 0;
	}
	for (auto& index : castRayArgument.m_checkHitAround) {
		index = false;
	}

	//���͂̕ǂƂ̓����蔻��
	CheckHitAround(arg_from, arg_newPos, arg_nowStage, arg_hitInfo, castRayArgument);

	//�n�ʂƂ̓����蔻��
	CheckHitGround(arg_from, arg_newPos, arg_nowStage, arg_hitInfo, castRayArgument);

	//�W�b�v���C���Ƃ̓����蔻��
	CheckZipline(arg_newPos, arg_nowStage);

	//���񂾂�(���܂��Ă��邩)�ǂ����𔻒�
	CheckDeath(arg_from, arg_newPos, arg_nowStage, arg_hitInfo, castRayArgument);

	//����ł����珈�����΂��B
	m_isDeath = false;
	m_isDeath |= (castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::RIGHT)] && castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::LEFT)]);
	m_isDeath |= (castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::TOP)] && castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::BOTTOM)]);
	m_isDeath |= (castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::FRONT)] && castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::BEHIND)]);
	if (m_isDeath) {
		++m_deathTimer;
	}
	else {
		m_deathTimer = 0;
	}

	//����ł�����
	if (DEATH_TIMER < m_deathTimer) {
		m_isDeath = true;
		return false;

	}

	m_isDeath = false;
	return true;
}

void Player::CheckDeath(const KuroEngine::Vec3<float> arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, Player::CastRayArgument& arg_castRayArgment)
{

	//�n�`�z�񑖍�
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//���f�����擾
		auto model = terrian.GetModel().lock();
		//�����擾�B
		arg_castRayArgment.m_stageType = StageParts::TERRIAN;

		//���b�V���𑖍�
		for (auto& modelMesh : model->m_meshes)
		{
			//���b�V�����擾
			auto& mesh = modelMesh.mesh;

			//CastRay�ɓn���������X�V�B
			arg_castRayArgment.m_mesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];

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

	//��������Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::MOVE_SCAFFOLD)continue;

		//��������Ƃ��ăL���X�g
		auto moveScaffold = dynamic_pointer_cast<MoveScaffold>(terrian);

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
			arg_castRayArgment.m_mesh = moveScaffold->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//���聫============================================

			//�ړ����������v���C���[���痣�������ɓ����Ă����玀�S������΂��B
			float oldPosDistance = (arg_newPos - moveScaffold->GetOldPos()).Length();
			float nowPosDistance = (arg_newPos - moveScaffold->GetNowPos()).Length();

			if (oldPosDistance < nowPosDistance) {
				continue;
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

	//�Ӄu���b�N�Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::IVY_BLOCK)continue;

		//��������Ƃ��ăL���X�g
		auto ivyBlock = dynamic_pointer_cast<IvyBlock>(terrian);

		//�o����Ԃ���Ȃ������珈�����΂��B
		if (!ivyBlock->GetIsAppear()) continue;

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
			arg_castRayArgment.m_mesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

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

}

void Player::CheckHitAround(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, Player::CastRayArgument& arg_castRayArgment) {

	//�v���C���[�̉�]���l�����Ȃ��A�@����񂾂��������ꍇ�̃g�����X�t�H�[���B
	KuroEngine::Transform localTrans;
	localTrans.SetRotate(m_normalSpinQ);

	//�v���C���[�̉�]���l�����Ȃ���]�s�񂩂烌�C���΂��������擾����B
	KuroEngine::Vec3<float> rightDir = localTrans.GetRight();
	KuroEngine::Vec3<float> frontDir = localTrans.GetFront();

	//�n�`�z�񑖍�
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//���f�����擾
		auto model = terrian.GetModel().lock();
		//�����擾�B
		arg_castRayArgment.m_stageType = StageParts::TERRIAN;

		//���b�V���𑖍�
		for (auto& modelMesh : model->m_meshes)
		{
			//���b�V�����擾
			auto& mesh = modelMesh.mesh;

			//CastRay�ɓn���������X�V�B
			arg_castRayArgment.m_mesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//���聫============================================

			//�E�����Ƀ��C���΂��B����͕ǂɂ������p�B
			arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::RIGHT)] |= CastRay(arg_newPos, arg_from, rightDir, WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::LEFT)] |= CastRay(arg_newPos, arg_from, -rightDir, WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::BEHIND)] |= CastRay(arg_newPos, arg_from, -frontDir, WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//���ʕ����Ƀ��C���΂��B����͕ǂɂ������p�B
			arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::FRONT)] |= CastRay(arg_newPos, arg_from, frontDir, WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//=================================================
		}
	}

	//��������Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::MOVE_SCAFFOLD)continue;

		//��������Ƃ��ăL���X�g
		auto moveScaffold = dynamic_pointer_cast<MoveScaffold>(terrian);

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
			arg_castRayArgment.m_mesh = moveScaffold->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//���聫============================================

			//�E�����Ƀ��C���΂��B����͕ǂɂ������p�B
			arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::RIGHT)] |= CastRay(arg_newPos, arg_from, rightDir, WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::LEFT)] |= CastRay(arg_newPos, arg_from, -rightDir, WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::BEHIND)] |= CastRay(arg_newPos, arg_from, -frontDir, WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//���ʕ����Ƀ��C���΂��B����͕ǂɂ������p�B
			arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::FRONT)] |= CastRay(arg_newPos, arg_from, frontDir, WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//=================================================
		}
	}


	//�Ӄu���b�N�Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::IVY_BLOCK)continue;

		//��������Ƃ��ăL���X�g
		auto ivyBlock = dynamic_pointer_cast<IvyBlock>(terrian);

		//�o����Ԃ���Ȃ������珈�����΂��B
		if (!ivyBlock->GetIsAppear()) continue;

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
			arg_castRayArgment.m_mesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//���聫============================================

			//�E�����Ƀ��C���΂��B����͕ǂɂ������p�B
			arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::RIGHT)] |= CastRay(arg_newPos, arg_from, rightDir, WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::LEFT)] |= CastRay(arg_newPos, arg_from, -rightDir, WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::BEHIND)] |= CastRay(arg_newPos, arg_from, -frontDir, WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//���ʕ����Ƀ��C���΂��B����͕ǂɂ������p�B
			arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::FRONT)] |= CastRay(arg_newPos, arg_from, frontDir, WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

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

			//�{���ɔ�ׂ�̂��`�F�b�N
			CheckCanJump(index, arg_nowStage);

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

			//�����|�����Ă���I�u�W�F�N�g�������ɃW�����v�ł�����������t���O���X�V�B
			if (arg_castRayArgment.m_impactPoint[minIndex].m_isFastJump) {
				m_canJump = true;
			}

			//�W�����v���ł����Ԃ�������W�����v����B
			if (m_canJump) {

				//�ŒZ�̏Փ˓_�����߂���A������W�����v��ɂ���B
				arg_hitInfo->m_terrianNormal = arg_castRayArgment.m_impactPoint[minIndex].m_normal;

				//�W�����v�̃p�����[�^�[�����߂�B
				m_playerMoveStatus = PLAYER_MOVE_STATUS::JUMP;
				m_jumpTimer = 0;
				m_jumpStartPos = arg_newPos;
				m_bezierCurveControlPos = m_jumpStartPos + m_transform.GetUp() * WALL_JUMP_LENGTH;
				m_jumpEndPos = arg_castRayArgment.m_impactPoint[minIndex].m_impactPos + arg_castRayArgment.m_impactPoint[minIndex].m_normal * (m_transform.GetScale().x / 2.0f);
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

	m_growPlantPtLig.Active();

}

void Player::CheckHitGround(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, Player::CastRayArgument& arg_castRayArgment) {

	//�v���C���[�̉�]���l�����Ȃ��A�@����񂾂��������ꍇ�̃g�����X�t�H�[���B
	KuroEngine::Transform localTrans;
	localTrans.SetRotate(m_normalSpinQ);

	//�v���C���[�̉�]���l�����Ȃ���]�s�񂩂烌�C���΂��������擾����B
	KuroEngine::Vec3<float> rightDir = localTrans.GetRight();
	KuroEngine::Vec3<float> frontDir = localTrans.GetFront();

	//�M�~�b�N�ɓ������Ă��邩�ǂ����̕ϐ���������
	m_prevOnGimmick = m_onGimmick;
	m_onGimmick = false;

	//�ڒn�t���O��ۑ�
	m_prevOnGround = m_onGround;
	m_onGround = false;

	//�R�`�F�b�N�ŉ������Ƀ��C��L�΂��ۂ̃��C�̒���
	const float CHECK_UNDERRAY_LENGTH = m_transform.GetScale().y * 2.0f;
	const float CHECK_CLIFFRAY_LENGTH = m_transform.GetScale().y * 10.0f;

	//�R����p�t���O
	std::array<bool, 4> isHitCliff = { false,false,false,false };

	//�n�`�z�񑖍�
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//���f�����擾
		auto model = terrian.GetModel().lock();
		//�����擾�B
		arg_castRayArgment.m_stageType = StageParts::TERRIAN;

		//���b�V���𑖍�
		for (auto& modelMesh : model->m_meshes)
		{
			//���b�V�����擾
			auto& mesh = modelMesh.mesh;

			//CastRay�ɓn���������X�V�B
			arg_castRayArgment.m_mesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//���聫============================================

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::RIGHT)]) {

				//�������Ƀ��C���΂��Ă������R�����`�F�b�N����B
				KuroEngine::Vec3<float> rayPos = arg_newPos + rightDir * WALL_JUMP_LENGTH;
				isHitCliff[static_cast<int>(RAY_DIR_ID::RIGHT)] |= CastRay(arg_newPos, rayPos, -m_transform.GetUp(), CHECK_UNDERRAY_LENGTH, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

			}

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::LEFT)]) {

				//�������Ƀ��C���΂��Ă������R�����`�F�b�N����B
				KuroEngine::Vec3<float> rayPos = arg_newPos - rightDir * WALL_JUMP_LENGTH;
				isHitCliff[static_cast<int>(RAY_DIR_ID::LEFT)] |= CastRay(arg_newPos, rayPos, -m_transform.GetUp(), CHECK_UNDERRAY_LENGTH, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

			}

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::FRONT)]) {

				//�������Ƀ��C���΂��Ă������R�����`�F�b�N����B
				KuroEngine::Vec3<float> rayPos = arg_newPos + frontDir * WALL_JUMP_LENGTH;
				isHitCliff[static_cast<int>(RAY_DIR_ID::FRONT)] |= CastRay(arg_newPos, rayPos, -m_transform.GetUp(), CHECK_UNDERRAY_LENGTH, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

			}

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::BEHIND)]) {

				//�������Ƀ��C���΂��Ă������R�����`�F�b�N����B
				KuroEngine::Vec3<float> rayPos = arg_newPos - frontDir * WALL_JUMP_LENGTH;
				isHitCliff[static_cast<int>(RAY_DIR_ID::BEHIND)] |= CastRay(arg_newPos, rayPos, -m_transform.GetUp(), CHECK_UNDERRAY_LENGTH, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

			}

			//=================================================
		}
	}
	//��������Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::MOVE_SCAFFOLD)continue;

		//��������Ƃ��ăL���X�g
		auto moveScaffold = dynamic_pointer_cast<MoveScaffold>(terrian);

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
			arg_castRayArgment.m_mesh = moveScaffold->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//���聫============================================

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::RIGHT)]) {

				//�������Ƀ��C���΂��Ă������R�����`�F�b�N����B
				KuroEngine::Vec3<float> rayPos = arg_newPos + rightDir * WALL_JUMP_LENGTH;
				isHitCliff[static_cast<int>(RAY_DIR_ID::RIGHT)] |= CastRay(arg_newPos, rayPos, -m_transform.GetUp(), CHECK_UNDERRAY_LENGTH, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

			}

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::LEFT)]) {

				//�������Ƀ��C���΂��Ă������R�����`�F�b�N����B
				KuroEngine::Vec3<float> rayPos = arg_newPos - rightDir * WALL_JUMP_LENGTH;
				isHitCliff[static_cast<int>(RAY_DIR_ID::LEFT)] |= CastRay(arg_newPos, rayPos, -m_transform.GetUp(), CHECK_UNDERRAY_LENGTH, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

			}

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::FRONT)]) {

				//�������Ƀ��C���΂��Ă������R�����`�F�b�N����B
				KuroEngine::Vec3<float> rayPos = arg_newPos + frontDir * WALL_JUMP_LENGTH;
				isHitCliff[static_cast<int>(RAY_DIR_ID::FRONT)] |= CastRay(arg_newPos, rayPos, -m_transform.GetUp(), CHECK_UNDERRAY_LENGTH, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

			}

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::BEHIND)]) {

				//�������Ƀ��C���΂��Ă������R�����`�F�b�N����B
				KuroEngine::Vec3<float> rayPos = arg_newPos - frontDir * WALL_JUMP_LENGTH;
				isHitCliff[static_cast<int>(RAY_DIR_ID::BEHIND)] |= CastRay(arg_newPos, rayPos, -m_transform.GetUp(), CHECK_UNDERRAY_LENGTH, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

			}

			//=================================================
		}
	}

	//�Ӄu���b�N�Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::IVY_BLOCK)continue;

		//��������Ƃ��ăL���X�g
		auto ivyBlock = dynamic_pointer_cast<IvyBlock>(terrian);

		//�o����Ԃ���Ȃ������珈�����΂��B
		if (!ivyBlock->GetIsAppear()) continue;

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
			arg_castRayArgment.m_mesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//���聫============================================

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::RIGHT)]) {

				//�������Ƀ��C���΂��Ă������R�����`�F�b�N����B
				KuroEngine::Vec3<float> rayPos = arg_newPos + rightDir * WALL_JUMP_LENGTH;
				isHitCliff[static_cast<int>(RAY_DIR_ID::RIGHT)] |= CastRay(arg_newPos, rayPos, -m_transform.GetUp(), CHECK_UNDERRAY_LENGTH, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

			}

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::LEFT)]) {

				//�������Ƀ��C���΂��Ă������R�����`�F�b�N����B
				KuroEngine::Vec3<float> rayPos = arg_newPos - rightDir * WALL_JUMP_LENGTH;
				isHitCliff[static_cast<int>(RAY_DIR_ID::LEFT)] |= CastRay(arg_newPos, rayPos, -m_transform.GetUp(), CHECK_UNDERRAY_LENGTH, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

			}

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::FRONT)]) {

				//�������Ƀ��C���΂��Ă������R�����`�F�b�N����B
				KuroEngine::Vec3<float> rayPos = arg_newPos + frontDir * WALL_JUMP_LENGTH;
				isHitCliff[static_cast<int>(RAY_DIR_ID::FRONT)] |= CastRay(arg_newPos, rayPos, -m_transform.GetUp(), CHECK_UNDERRAY_LENGTH, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

			}

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::BEHIND)]) {

				//�������Ƀ��C���΂��Ă������R�����`�F�b�N����B
				KuroEngine::Vec3<float> rayPos = arg_newPos - frontDir * WALL_JUMP_LENGTH;
				isHitCliff[static_cast<int>(RAY_DIR_ID::BEHIND)] |= CastRay(arg_newPos, rayPos, -m_transform.GetUp(), CHECK_UNDERRAY_LENGTH, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

			}

			//=================================================
		}
	}

	//�������Ă��Ȃ�������v���C���[�����Ƀ��C���΂��ďՓ˓_���L�^����B
	std::array<std::vector<KuroEngine::Vec3<float>>, 4> impactPoint;
	KuroEngine::Vec3<float> impactPointBuff;	//���W�ꎞ�ۑ��p
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//���f�����擾
		auto model = terrian.GetModel().lock();
		//�����擾�B
		arg_castRayArgment.m_stageType = StageParts::TERRIAN;

		//���b�V���𑖍�
		for (auto& modelMesh : model->m_meshes)
		{
			//���b�V�����擾
			auto& mesh = modelMesh.mesh;

			//CastRay�ɓn���������X�V�B
			arg_castRayArgment.m_mesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//���聫============================================

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!isHitCliff[static_cast<int>(RAY_DIR_ID::RIGHT)]) {

				KuroEngine::Vec3<float> rayPos = arg_newPos + rightDir * WALL_JUMP_LENGTH;
				if (CastRay(impactPointBuff, rayPos - m_transform.GetUp() * CHECK_UNDERRAY_LENGTH, -rightDir, CHECK_CLIFFRAY_LENGTH, arg_castRayArgment, RAY_ID::CLIFF)) {
					impactPoint[static_cast<int>(RAY_DIR_ID::RIGHT)].emplace_back(impactPointBuff);
				}

			}

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!isHitCliff[static_cast<int>(RAY_DIR_ID::LEFT)]) {

				KuroEngine::Vec3<float> rayPos = arg_newPos - rightDir * WALL_JUMP_LENGTH;
				if (CastRay(impactPointBuff, rayPos - m_transform.GetUp() * CHECK_UNDERRAY_LENGTH, rightDir, CHECK_CLIFFRAY_LENGTH, arg_castRayArgment, RAY_ID::CLIFF)) {
					impactPoint[static_cast<int>(RAY_DIR_ID::LEFT)].emplace_back(impactPointBuff);
				}

			}


			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!isHitCliff[static_cast<int>(RAY_DIR_ID::FRONT)]) {

				KuroEngine::Vec3<float> rayPos = arg_newPos + frontDir * WALL_JUMP_LENGTH;
				if (CastRay(impactPointBuff, rayPos - m_transform.GetUp() * CHECK_UNDERRAY_LENGTH, -frontDir, CHECK_CLIFFRAY_LENGTH, arg_castRayArgment, RAY_ID::CLIFF)) {
					impactPoint[static_cast<int>(RAY_DIR_ID::FRONT)].emplace_back(impactPointBuff);
				}

			}

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!isHitCliff[static_cast<int>(RAY_DIR_ID::BEHIND)]) {

				KuroEngine::Vec3<float> rayPos = arg_newPos - frontDir * WALL_JUMP_LENGTH;
				if (CastRay(impactPointBuff, rayPos - m_transform.GetUp() * CHECK_UNDERRAY_LENGTH, frontDir, CHECK_CLIFFRAY_LENGTH, arg_castRayArgment, RAY_ID::CLIFF)) {
					impactPoint[static_cast<int>(RAY_DIR_ID::BEHIND)].emplace_back(impactPointBuff);
				}

			}

			//=================================================
		}
	}
	//��������Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::MOVE_SCAFFOLD)continue;

		//��������Ƃ��ăL���X�g
		auto moveScaffold = dynamic_pointer_cast<MoveScaffold>(terrian);

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
			arg_castRayArgment.m_mesh = moveScaffold->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//���聫============================================

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!isHitCliff[static_cast<int>(RAY_DIR_ID::RIGHT)]) {

				KuroEngine::Vec3<float> rayPos = arg_newPos + rightDir * WALL_JUMP_LENGTH;
				if (CastRay(impactPointBuff, rayPos - m_transform.GetUp() * CHECK_UNDERRAY_LENGTH, -rightDir, CHECK_CLIFFRAY_LENGTH, arg_castRayArgment, RAY_ID::CLIFF)) {
					impactPoint[static_cast<int>(RAY_DIR_ID::RIGHT)].emplace_back(impactPointBuff);
				}

			}

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!isHitCliff[static_cast<int>(RAY_DIR_ID::LEFT)]) {

				KuroEngine::Vec3<float> rayPos = arg_newPos - rightDir * WALL_JUMP_LENGTH;
				if (CastRay(impactPointBuff, rayPos - m_transform.GetUp() * CHECK_UNDERRAY_LENGTH, rightDir, CHECK_CLIFFRAY_LENGTH, arg_castRayArgment, RAY_ID::CLIFF)) {
					impactPoint[static_cast<int>(RAY_DIR_ID::LEFT)].emplace_back(impactPointBuff);
				}

			}


			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!isHitCliff[static_cast<int>(RAY_DIR_ID::FRONT)]) {

				KuroEngine::Vec3<float> rayPos = arg_newPos + frontDir * WALL_JUMP_LENGTH;
				if (CastRay(impactPointBuff, rayPos - m_transform.GetUp() * CHECK_UNDERRAY_LENGTH, -frontDir, CHECK_CLIFFRAY_LENGTH, arg_castRayArgment, RAY_ID::CLIFF)) {
					impactPoint[static_cast<int>(RAY_DIR_ID::FRONT)].emplace_back(impactPointBuff);
				}

			}

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!isHitCliff[static_cast<int>(RAY_DIR_ID::BEHIND)]) {

				KuroEngine::Vec3<float> rayPos = arg_newPos - frontDir * WALL_JUMP_LENGTH;
				if (CastRay(impactPointBuff, rayPos - m_transform.GetUp() * CHECK_UNDERRAY_LENGTH, frontDir, CHECK_CLIFFRAY_LENGTH, arg_castRayArgment, RAY_ID::CLIFF)) {
					impactPoint[static_cast<int>(RAY_DIR_ID::BEHIND)].emplace_back(impactPointBuff);
				}

			}

			//=================================================
		}
	}

	//�Ӄu���b�N�Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::IVY_BLOCK)continue;

		//��������Ƃ��ăL���X�g
		auto ivyBlock = dynamic_pointer_cast<IvyBlock>(terrian);

		//�o����Ԃ���Ȃ������珈�����΂��B
		if (!ivyBlock->GetIsAppear()) continue;

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
			arg_castRayArgment.m_mesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//���聫============================================

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!isHitCliff[static_cast<int>(RAY_DIR_ID::RIGHT)]) {

				KuroEngine::Vec3<float> rayPos = arg_newPos + rightDir * WALL_JUMP_LENGTH;
				if (CastRay(impactPointBuff, rayPos - m_transform.GetUp() * CHECK_UNDERRAY_LENGTH, -rightDir, CHECK_CLIFFRAY_LENGTH, arg_castRayArgment, RAY_ID::CLIFF)) {
					impactPoint[static_cast<int>(RAY_DIR_ID::RIGHT)].emplace_back(impactPointBuff);
				}

			}

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!isHitCliff[static_cast<int>(RAY_DIR_ID::LEFT)]) {

				KuroEngine::Vec3<float> rayPos = arg_newPos - rightDir * WALL_JUMP_LENGTH;
				if (CastRay(impactPointBuff, rayPos - m_transform.GetUp() * CHECK_UNDERRAY_LENGTH, rightDir, CHECK_CLIFFRAY_LENGTH, arg_castRayArgment, RAY_ID::CLIFF)) {
					impactPoint[static_cast<int>(RAY_DIR_ID::LEFT)].emplace_back(impactPointBuff);
				}

			}


			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!isHitCliff[static_cast<int>(RAY_DIR_ID::FRONT)]) {

				KuroEngine::Vec3<float> rayPos = arg_newPos + frontDir * WALL_JUMP_LENGTH;
				if (CastRay(impactPointBuff, rayPos - m_transform.GetUp() * CHECK_UNDERRAY_LENGTH, -frontDir, CHECK_CLIFFRAY_LENGTH, arg_castRayArgment, RAY_ID::CLIFF)) {
					impactPoint[static_cast<int>(RAY_DIR_ID::FRONT)].emplace_back(impactPointBuff);
				}

			}

			//�E���̎��͂̃��C���������Ă��Ȃ�������B
			if (!isHitCliff[static_cast<int>(RAY_DIR_ID::BEHIND)]) {

				KuroEngine::Vec3<float> rayPos = arg_newPos - frontDir * WALL_JUMP_LENGTH;
				if (CastRay(impactPointBuff, rayPos - m_transform.GetUp() * CHECK_UNDERRAY_LENGTH, frontDir, CHECK_CLIFFRAY_LENGTH, arg_castRayArgment, RAY_ID::CLIFF)) {
					impactPoint[static_cast<int>(RAY_DIR_ID::BEHIND)].emplace_back(impactPointBuff);
				}

			}

			//=================================================
		}
	}

	//���߂�ꂽ�Փ˓_�̒�����e�����̈�ԋ߂��Փ˓_��������B
	std::vector<KuroEngine::Vec3<float>> nearPos;
	std::vector<KuroEngine::Vec3<float>> nearestPos;
	//�܂��͉E����
	for (auto& index : impactPoint[static_cast<int>(RAY_DIR_ID::RIGHT)]) {

		float minLength = std::numeric_limits<float>().max();
		KuroEngine::Vec3<float> castPos = arg_newPos + rightDir * WALL_JUMP_LENGTH - m_transform.GetUp() * CHECK_UNDERRAY_LENGTH;
		float length = KuroEngine::Vec3<float>(castPos - index).Length();
		if (length < minLength) {
			minLength = length;
			nearPos.emplace_back(index);
		}

	}
	if (0 < nearPos.size())nearestPos.emplace_back(nearPos.back());
	nearPos.clear();
	//���͍�
	for (auto& index : impactPoint[static_cast<int>(RAY_DIR_ID::LEFT)]) {

		float minLength = std::numeric_limits<float>().max();
		KuroEngine::Vec3<float> castPos = arg_newPos - rightDir * WALL_JUMP_LENGTH - m_transform.GetUp() * CHECK_UNDERRAY_LENGTH;
		float length = KuroEngine::Vec3<float>(castPos - index).Length();
		if (length < minLength) {
			minLength = length;
			nearPos.emplace_back(index);
		}

	}
	if (0 < nearPos.size())nearestPos.emplace_back(nearPos.back());
	nearPos.clear();
	//���͑O
	for (auto& index : impactPoint[static_cast<int>(RAY_DIR_ID::FRONT)]) {

		float minLength = std::numeric_limits<float>().max();
		KuroEngine::Vec3<float> castPos = arg_newPos + frontDir * WALL_JUMP_LENGTH - m_transform.GetUp() * CHECK_UNDERRAY_LENGTH;
		float length = KuroEngine::Vec3<float>(castPos - index).Length();
		if (length < minLength) {
			minLength = length;
			nearPos.emplace_back(index);
		}
	}
	if (0 < nearPos.size())nearestPos.emplace_back(nearPos.back());
	nearPos.clear();
	//�Ō�Ɍ��
	for (auto& index : impactPoint[static_cast<int>(RAY_DIR_ID::BEHIND)]) {

		float minLength = std::numeric_limits<float>().max();
		KuroEngine::Vec3<float> castPos = arg_newPos - frontDir * WALL_JUMP_LENGTH - m_transform.GetUp() * CHECK_UNDERRAY_LENGTH;
		float length = KuroEngine::Vec3<float>(castPos - index).Length();
		if (length < minLength) {
			minLength = length;
			nearPos.emplace_back(index);
		}
	}
	if (0 < nearPos.size())nearestPos.emplace_back(nearPos.back());
	nearPos.clear();

	//�Փ˓_�̕��ϒl�������߂��ꏊ
	if (0 < nearestPos.size()) {
		KuroEngine::Vec3<float> pushBackPos;
		for (auto& index : nearestPos) {
			pushBackPos += index;
		}
		if (1 < nearestPos.size()) {
			int a = 0;
		}
		pushBackPos /= static_cast<float>(nearestPos.size());
		arg_newPos = pushBackPos;
	}


	//�ŏI�I�Ȍ��ʂ����Ƃɉ������Ƀ��C���΂��Đڒn������s���B

	//�n�`�z�񑖍�
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//���f�����擾
		auto model = terrian.GetModel().lock();
		//�����擾�B
		arg_castRayArgment.m_stageType = StageParts::TERRIAN;

		//���b�V���𑖍�
		for (auto& modelMesh : model->m_meshes)
		{
			//���b�V�����擾
			auto& mesh = modelMesh.mesh;

			//CastRay�ɓn���������X�V�B
			arg_castRayArgment.m_mesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//���聫============================================

			//������������̎p��
			KuroEngine::Transform moveQtransform;
			moveQtransform.SetRotate(m_moveQ);

			m_onGround |= CastRay(arg_newPos, arg_newPos + moveQtransform.GetFront() * m_transform.GetScale().x, -m_transform.GetUp(), m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
			m_onGround |= CastRay(arg_newPos, arg_newPos - moveQtransform.GetFront() * m_transform.GetScale().x, -m_transform.GetUp(), m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
			m_onGround |= CastRay(arg_newPos, arg_newPos + moveQtransform.GetRight() * m_transform.GetScale().x, -m_transform.GetUp(), m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
			m_onGround |= CastRay(arg_newPos, arg_newPos - moveQtransform.GetRight() * m_transform.GetScale().x, -m_transform.GetUp(), m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);

			//=================================================
		}
	}
	//��������Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::MOVE_SCAFFOLD)continue;

		//��������Ƃ��ăL���X�g
		auto moveScaffold = dynamic_pointer_cast<MoveScaffold>(terrian);

		//�v���C���[������Ă��邩�̃t���O����U����B
		moveScaffold->SetOnPlayer(false);

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
			arg_castRayArgment.m_mesh = moveScaffold->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//���聫============================================

			//������������̎p��
			KuroEngine::Transform moveQtransform;
			moveQtransform.SetRotate(m_moveQ);

			m_onGround |= CastRay(arg_newPos, arg_newPos + moveQtransform.GetFront() * m_transform.GetScale().x, -m_transform.GetUp(), m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
			m_onGround |= CastRay(arg_newPos, arg_newPos - moveQtransform.GetFront() * m_transform.GetScale().x, -m_transform.GetUp(), m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
			m_onGround |= CastRay(arg_newPos, arg_newPos + moveQtransform.GetRight() * m_transform.GetScale().x, -m_transform.GetUp(), m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
			m_onGround |= CastRay(arg_newPos, arg_newPos - moveQtransform.GetRight() * m_transform.GetScale().x, -m_transform.GetUp(), m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);


			//�܂��͐^���Ƀ��C���΂��B
			bool isHit = CastRay(arg_newPos, arg_newPos, -m_transform.GetUp(), m_transform.GetScale().y, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

			//���ɓ����������̌�둤���烌�C���΂��ē������Ă�����M�~�b�N���N������B
			if (isHit) {

				m_onGimmick = true;

				//�v���C���[����������Ƃ��M�~�b�N���ɓ`����B
				moveScaffold->OnPlayer();

			}

			//=================================================
		}
	}

	//�Ӄu���b�N�Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::IVY_BLOCK)continue;

		//��������Ƃ��ăL���X�g
		auto ivyBlock = dynamic_pointer_cast<IvyBlock>(terrian);

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
			arg_castRayArgment.m_mesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//���聫============================================

			//�u���b�N���o�����̂Ƃ��̏���
			if (ivyBlock->GetIsAppear()) {

				//������������̎p��
				KuroEngine::Transform moveQtransform;
				moveQtransform.SetRotate(m_moveQ);

				m_onGround |= CastRay(arg_newPos, arg_newPos + moveQtransform.GetFront() * m_transform.GetScale().x, -m_transform.GetUp(), m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
				m_onGround |= CastRay(arg_newPos, arg_newPos - moveQtransform.GetFront() * m_transform.GetScale().x, -m_transform.GetUp(), m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
				m_onGround |= CastRay(arg_newPos, arg_newPos + moveQtransform.GetRight() * m_transform.GetScale().x, -m_transform.GetUp(), m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
				m_onGround |= CastRay(arg_newPos, arg_newPos - moveQtransform.GetRight() * m_transform.GetScale().x, -m_transform.GetUp(), m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);

				//�o�����Ă�����
				if (ivyBlock->GetIsAppear()) {

					//�ړ�������̌��̕��Ƀ��C���΂��B
					bool isHit = CastRay(arg_newPos, arg_newPos - moveQtransform.GetFront() * m_transform.GetScale().x, -m_transform.GetUp(), m_transform.GetScale().y, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

					//�v���C���[������Ă��锻��̂Ƃ��Ƀ��C��������Ȃ�������A�o����Ԃ�؂�B
					if (ivyBlock->GetOnPlayer() && !isHit) {

						ivyBlock->Disappear();

						//SE��炷�B
						SoundConfig::Instance()->Play(SoundConfig::SE_GRASS);

					}
					//�v���C���[������Ă��锻��B
					else if (isHit) {

						ivyBlock->OnPlayer();

					}

				}

			}
			//�o��������Ȃ��Ƃ��̏���
			else {

				//�v���C���[�Ƀq�b�g���Ă�����A�傫�߂̓����蔻����s���A�������Ă��Ȃ�������o��������B
				if (ivyBlock->GetOnPlayer()) {
					bool isHit = KuroEngine::Vec3<float>(arg_newPos - ivyBlock->GetPos()).Length() <= ivyBlock->GetHitScaleMax();
					if (!isHit) {

						ivyBlock->Appear();
						ivyBlock->OffPlayer();

						//SE��炷�B
						SoundConfig::Instance()->Play(SoundConfig::SE_GRASS);

					}
				}
				//�v���C���[�̃q�b�g���Ă��Ȃ�������A�����߂̓����蔻����s���A�o����������悤�ɂ���B
				else {
					bool isHit = KuroEngine::Vec3<float>(arg_newPos - ivyBlock->GetPos()).Length() <= ivyBlock->GetHitScaleMin();
					if (isHit) {

						ivyBlock->OnPlayer();

					}
				}


			}


			//=================================================
		}
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
		auto model = terrian.GetModel().lock();

		//���b�V���𑖍�
		for (auto& modelMesh : model->m_meshes)
		{

			//���聫============================================

			//�����蔻����s�����b�V���B
			std::vector<TerrianHitPolygon> mesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];

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

	//��������Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::MOVE_SCAFFOLD)continue;

		//��������Ƃ��ăL���X�g
		auto moveScaffold = dynamic_pointer_cast<MoveScaffold>(terrian);

		//���f�����擾
		auto model = terrian->GetModel();

		//���b�V���𑖍�
		for (auto& modelMesh : model.lock()->m_meshes) {

			//���聫============================================

			//�����蔻����s�����b�V���B
			std::vector<TerrianHitPolygon> mesh = moveScaffold->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

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

	//�Ӄu���b�N�Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::IVY_BLOCK)continue;

		//��������Ƃ��ăL���X�g
		auto ivyBlock = dynamic_pointer_cast<IvyBlock>(terrian);

		//�o����Ԃ���Ȃ������珈�����΂��B
		if (!ivyBlock->GetIsAppear()) continue;

		//���f�����擾
		auto model = terrian->GetModel();

		//���b�V���𑖍�
		for (auto& modelMesh : model.lock()->m_meshes)
		{

			//�����蔻����s�����b�V���B
			std::vector<TerrianHitPolygon> mesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

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

void Player::CheckCanJump(Player::ImpactPointData& arg_impactPointData, std::weak_ptr<Stage> arg_nowStage)
{

	//�L��������Ă��Ȃ������珈�����΂��B
	if (!arg_impactPointData.m_isActive) return;

	//�R����p�̃��C�̒���
	const float CLIFF_RAY_LENGTH = WALL_JUMP_LENGTH + m_transform.GetScale().Length();

	//�n�`�z�񑖍�
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//���f�����擾
		auto model = terrian.GetModel().lock();

		//���b�V���𑖍�
		for (auto& modelMesh : model->m_meshes)
		{

			//���聫============================================

			//�����蔻����s�����b�V���B
			std::vector<TerrianHitPolygon> mesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//�������Ƀ��C���΂��B
			MeshCollisionOutput output = MeshCollision((arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * m_transform.GetScale().x) + m_transform.GetUp() * WALL_JUMP_LENGTH, -arg_impactPointData.m_normal, mesh);

			//���C�����b�V���ɏՓ˂��Ă���A�Փ˒n�_�܂ł̋��������C�̒�����菬����������Փ˂��Ă���B
			if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

				//�ǂɓ����������_�ŊR�ł͂Ȃ��̂ŏ������΂��B
				return;

			}

			//=================================================
		}
	}

	//��������Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::MOVE_SCAFFOLD)continue;

		//��������Ƃ��ăL���X�g
		auto moveScaffold = dynamic_pointer_cast<MoveScaffold>(terrian);

		//���f�����擾
		auto model = terrian->GetModel();

		//���b�V���𑖍�
		for (auto& modelMesh : model.lock()->m_meshes) {

			//���聫============================================

			//�����蔻����s�����b�V���B
			std::vector<TerrianHitPolygon> mesh = moveScaffold->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//�������Ƀ��C���΂��B
			MeshCollisionOutput output = MeshCollision((arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * m_transform.GetScale().x) + m_transform.GetUp() * WALL_JUMP_LENGTH, -arg_impactPointData.m_normal, mesh);

			//���C�����b�V���ɏՓ˂��Ă���A�Փ˒n�_�܂ł̋��������C�̒�����菬����������Փ˂��Ă���B
			if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

				//�ǂɓ����������_�ŊR�ł͂Ȃ��̂ŏ������΂��B
				return;

			}

			//=================================================
		}
	}

	//�Ӄu���b�N�Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::IVY_BLOCK)continue;

		//��������Ƃ��ăL���X�g
		auto ivyBlock = dynamic_pointer_cast<IvyBlock>(terrian);

		//�o����Ԃ���Ȃ������珈�����΂��B
		if (!ivyBlock->GetIsAppear()) continue;

		//���f�����擾
		auto model = terrian->GetModel();

		//���b�V���𑖍�
		for (auto& modelMesh : model.lock()->m_meshes) {

			//���聫============================================

			//�����蔻����s�����b�V���B
			std::vector<TerrianHitPolygon> mesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//�������Ƀ��C���΂��B
			MeshCollisionOutput output = MeshCollision((arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * m_transform.GetScale().x) + m_transform.GetUp() * WALL_JUMP_LENGTH, -arg_impactPointData.m_normal, mesh);

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
	:KuroEngine::Debugger("Player", true, true), m_growPlantPtLig(8.0f, &m_transform)
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
	m_initTransform = arg_initTransform;
	m_transform = arg_initTransform;
	m_camController.Init();
	m_cameraRotY = 0;
	m_cameraRotYStorage = 0;
	m_cameraRotMove = 0;
	m_cameraJumpLerpAmount = 0;
	m_cameraJumpLerpStorage = 0;
	m_cameraQ = DirectX::XMQuaternionIdentity();
	m_canJumpDelayTimer = 0;
	m_deathTimer = 0;

	m_moveSpeed = KuroEngine::Vec3<float>();
	m_gimmickVel = KuroEngine::Vec3<float>();
	m_isFirstOnGround = false;
	m_onGimmick = false;
	m_cameraMode = 1;
	m_prevOnGimmick = false;
	m_isDeath = false;
	m_canZip = false;
	m_playerMoveStatus = PLAYER_MOVE_STATUS::MOVE;

	m_growPlantPtLig.Register();
}

void Player::Update(const std::weak_ptr<Stage>arg_nowStage)
{
	using namespace KuroEngine;

	//�g�����X�t�H�[����ۑ��B
	m_prevTransform = m_transform;

	//�X�e�[�W��ۑ��B
	m_stage = arg_nowStage;

	//�ʒu���֌W
	auto beforePos = m_transform.GetPos();
	auto newPos = beforePos;

	//���͂��ꂽ�����ړ��p�x�ʂ��擾
	auto scopeMove = OperationConfig::Instance()->GetScopeMove();

	//�W�����v���ł��邩�ǂ����B	��莞�Ԓn�`�Ɉ����|�����Ă���W�����v�ł���B
	m_canJump = CAN_JUMP_DELAY <= m_canJumpDelayTimer;

	//�J�������[�h��؂�ւ���B
	if (UsersInput::Instance()->KeyOffTrigger(DIK_RETURN) || UsersInput::Instance()->ControllerOnTrigger(0, KuroEngine::X)) {
		++m_cameraMode;
		if (static_cast<int>(CAMERA_MODE.size()) <= m_cameraMode) {
			m_cameraMode = 0;
		}

		//SE��炷�B
		SoundConfig::Instance()->Play(SoundConfig::SE_CAM_MODE_CHANGE, -1, m_cameraMode);
	}

	//�W�b�v���C��
	m_canZip = UsersInput::Instance()->KeyOnTrigger(DIK_SPACE);

	//�ړ��X�e�[�^�X�ɂ���ď�����ς���B
	switch (m_playerMoveStatus)
	{
	case Player::PLAYER_MOVE_STATUS::MOVE:
	{

		//�v���C���[�̉�]���J������ɂ���B(�ړ������̊���J�����̊p�x�Ȃ���)
		m_transform.SetRotate(m_cameraQ);

		//���͂��ꂽ�ړ��ʂ��擾
		m_rowMoveVec = OperationConfig::Instance()->GetMoveVecFuna(XMQuaternionIdentity());	//���̓��͕������擾�B�v���C���[����͕����ɉ�]������ۂɁAXZ���ʂł̒l���g�p����������B

		//�J�����̉�]��ۑ��B
		m_cameraRotYStorage += scopeMove.x;

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
			m_cameraRotMove = m_cameraRotYStorage;

		}
		else {

			//�ړ�����������ۑ��B
			m_playerRotY = atan2f(m_rowMoveVec.x, m_rowMoveVec.z);

		}

		//�����蔻��
		CheckHit(beforePos, newPos, arg_nowStage);

		m_transform.SetPos(newPos);

	}
	break;
	case Player::PLAYER_MOVE_STATUS::JUMP:
	{

		//�^�C�}�[���X�V�B
		m_jumpTimer = std::clamp(m_jumpTimer + JUMP_TIMER, 0.0f, 1.0f);

		float easeAmount = KuroEngine::Math::Ease(Out, Sine, m_jumpTimer, 0.0f, 1.0f);

		//�J�����̉�]���Ԃ���B
		m_cameraRotMove = m_cameraJumpLerpStorage + easeAmount * m_cameraJumpLerpAmount;

		//���W���Ԃ���B
		newPos = CalculateBezierPoint(easeAmount, m_jumpStartPos, m_jumpEndPos, m_bezierCurveControlPos);

		//��]��⊮����B
		m_transform.SetRotate(DirectX::XMQuaternionSlerp(m_jumpStartQ, m_jumpEndQ, easeAmount));

		//����ɒB���Ă�����W�����v���I����B
		if (1.0f <= m_jumpTimer) {
			m_playerMoveStatus = PLAYER_MOVE_STATUS::MOVE;
			m_cameraJumpLerpAmount = 0;

			//�ʈړ�SE��炷�B
			SoundConfig::Instance()->Play(SoundConfig::SE_SURFACE_JUMP);

		}
		m_transform.SetPos(newPos);

	}
	break;
	case PLAYER_MOVE_STATUS::ZIP:
	{

		//�W�b�v���C���̍X�V����
		UpdateZipline();

	}
	break;
	default:
		break;
	}


	//���W�ω��K�p
	m_ptLig.SetPos(newPos);

	//�J��������
	m_camController.Update(scopeMove, m_transform.GetPosWorld(), m_cameraRotYStorage, CAMERA_MODE[m_cameraMode]);

	//�M�~�b�N�̈ړ���ł������B
	m_gimmickVel = KuroEngine::Vec3<float>();

	m_growPlantPtLig.Active();
}

void Player::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, bool arg_cameraDraw)
{
	/*
	KuroEngine::DrawFunc3D::DrawNonShadingModel(
		m_model,
		m_transform,
		arg_cam);
	*/

	BasicDraw::Instance()->Draw_Player(
		arg_cam,
		arg_ligMgr,
		m_model,
		m_transform,
		IndividualDrawParameter::GetDefault());

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

void Player::FinishGimmickMove()
{

	//�S�����ɒn�ʗp�̃��C���΂��Ēn�ʔ��������B

	m_gimmickExitPos.clear();
	m_gimmickExitNormal.clear();

	//�����߂����W
	KuroEngine::Vec3<float> pos = m_transform.GetPosWorld();

	Player::CastRayArgument castRayArgument;

	//���C�̒���
	const float RAY_LENGTH = 10.0f;

	//�n�`�z�񑖍�
	for (auto& terrian : m_stage.lock()->GetTerrianArray())
	{
		//���f�����擾
		auto model = terrian.GetModel().lock();
		//�����擾�B
		castRayArgument.m_stageType = StageParts::TERRIAN;

		//���b�V���𑖍�
		for (auto& modelMesh : model->m_meshes)
		{
			//���b�V�����擾
			auto& mesh = modelMesh.mesh;

			//CastRay�ɓn���������X�V�B
			castRayArgument.m_mesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//���聫============================================

			//�E�����Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, m_transform.GetRight(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, -m_transform.GetRight(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, -m_transform.GetFront(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//���ʕ����Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, m_transform.GetFront(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, -m_transform.GetUp(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, m_transform.GetUp(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//=================================================
		}
	}

	//��������Ƃ̓����蔻��
	for (auto& terrian : m_stage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::MOVE_SCAFFOLD)continue;

		//��������Ƃ��ăL���X�g
		auto moveScaffold = dynamic_pointer_cast<MoveScaffold>(terrian);

		//���f�����擾
		auto model = terrian->GetModel();
		//�����擾�B
		castRayArgument.m_stageType = terrian->GetType();
		//�X�e�[�W����ۑ��B
		castRayArgument.m_stage = terrian;

		//���b�V���𑖍�
		for (auto& modelMesh : model.lock()->m_meshes)
		{
			//���b�V�����擾
			auto& mesh = modelMesh.mesh;

			//CastRay�ɓn���������X�V�B
			castRayArgument.m_mesh = moveScaffold->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//���聫============================================

			//�E�����Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, m_transform.GetRight(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, -m_transform.GetRight(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, -m_transform.GetFront(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//���ʕ����Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, m_transform.GetFront(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, -m_transform.GetUp(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, m_transform.GetUp(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//=================================================
		}
	}

	//�ŒZ�̂��̂���������B
	KuroEngine::Vec3<float> minPos = pos;
	KuroEngine::Vec3<float> normal = { 0,1,0 };
	float minLength = std::numeric_limits<float>().max();
	for (int index = 0; index < static_cast<int>(m_gimmickExitNormal.size()); ++index) {
		float length = KuroEngine::Vec3<float>(pos - m_gimmickExitPos[index]).Length();
		if (length < minLength) {
			minLength = length;
			minPos = m_gimmickExitPos[index];
			normal = m_gimmickExitNormal[index];
		}
	}

	m_zipInOutPos = minPos;

	//�n�`�̖@�����^���������Ă���Ƃ��Ɍ덷�ł��ꂢ��0,-1,0�ɂȂ��Ă���Ȃ������ł��܂������Ȃ��̂ŋ���̍�B
	if (normal.y < -0.9f) {
		normal = { 0,-1,0 };
	}

	//�J��������������B
	AdjustCaneraRotY(m_transform.GetUp(), normal);

	//�@������������N�H�[�^�j�I��
	m_normalSpinQ = KuroEngine::Math::GetLookAtQuaternion({ 0,1,0 }, normal);

	//�J�����̉�]��Y����]������N�H�[�^�j�I���B�ړ������ɉ�]���Ă���悤�Ɍ��������邽�߂̂��́Bm_cameraJumpLerpAmount�͕�Ԍ�̃J�����Ɍ������ĕ�Ԃ��邽�߁B
	DirectX::XMVECTOR ySpin;
	if (normal.y < -0.9f) {
		ySpin = DirectX::XMQuaternionRotationNormal(normal, -(m_cameraRotMove + m_cameraJumpLerpAmount) + DirectX::XM_PI);
	}
	else {
		ySpin = DirectX::XMQuaternionRotationNormal(normal, m_cameraRotMove + m_cameraJumpLerpAmount);
	}

	//�v���C���[�̈ړ�������Y����]������N�H�[�^�j�I���B�ړ������ɉ�]���Ă���悤�Ɍ��������邽�߂̂��́B
	DirectX::XMVECTOR playerYSpin;
	playerYSpin = DirectX::XMQuaternionRotationNormal(normal, m_playerRotY);

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

	m_gimmickStatus = GIMMICK_STATUS::EXIT;

	//SE��炷�B
	SoundConfig::Instance()->Play(SoundConfig::SE_ZIP_LINE_GET_ON);
}

Player::MeshCollisionOutput Player::MeshCollision(const KuroEngine::Vec3<float>& arg_rayPos, const KuroEngine::Vec3<float>& arg_rayDir, std::vector<TerrianHitPolygon>& arg_targetMesh) {


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
	std::vector<std::pair<Player::MeshCollisionOutput, TerrianHitPolygon>> hitDataContainer;

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
		static const float OFFSET = 0.1f;

		//���C�̎�ނɂ���ĕۑ�����f�[�^��ς���B
		switch (arg_rayID)
		{
		case Player::RAY_ID::GROUND:

			//�O���ɓn���p�̃f�[�^��ۑ�
			arg_collisionData.m_bottomTerrianNormal = output.m_normal;

			//�����߂��B
			arg_charaPos += output.m_normal * (std::fabs(output.m_distance - m_transform.GetScale().x) - OFFSET);

			break;

		case Player::RAY_ID::AROUND:

			//���C�̏Փ˒n�_��ۑ��B
			arg_collisionData.m_impactPoint.emplace_back(ImpactPointData(output.m_pos, output.m_normal));

			//��������������߂荞��ł��܂��̂ŉ����߂��B
			if (arg_collisionData.m_stageType == StageParts::MOVE_SCAFFOLD) {

				arg_charaPos += output.m_normal * (std::fabs(output.m_distance - m_transform.GetScale().x) - OFFSET);

			}
			else if (m_prevOnGimmick) {

				arg_collisionData.m_impactPoint.back().m_isFastJump = true;

			}

			break;

		case Player::RAY_ID::CLIFF:
		{

			//�O���ɓn���p�̃f�[�^��ۑ�
			arg_collisionData.m_bottomTerrianNormal = m_transform.GetUp();

			//�Փ˒n�_���������Ɉʒu�����炷�B
			const float CHECK_UNDERRAY_LENGTH = m_transform.GetScale().y * 2.0f;
			output.m_pos += m_transform.GetUp() * CHECK_UNDERRAY_LENGTH;

			//�����߂����ʒu�ɍ��W��ݒ�B
			arg_charaPos = output.m_pos - (output.m_normal * m_transform.GetScale().x * (WALL_JUMP_LENGTH - OFFSET));

		}
		break;

		case Player::RAY_ID::CHECK_GIMMICK:

			m_onGimmick = true;

			//����ɃM�~�b�N�ɓ��������g���K�[��������M�~�b�N��L����������B
			if (!m_prevOnGimmick) {
				dynamic_pointer_cast<MoveScaffold>(arg_collisionData.m_stage.lock())->Activate();
			}


			break;

		case Player::RAY_ID::CHECK_DEATH:

			arg_collisionData.m_checkDeathCounter[static_cast<int>(arg_rayDirID)] = true;

			break;

		case Player::RAY_ID::CHECK_CLIFF:

			break;

		case Player::RAY_ID::CHECK_IVY:

			m_gimmickExitPos.emplace_back(output.m_pos + output.m_normal);
			m_gimmickExitNormal.emplace_back(output.m_normal);

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
		ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, -(m_cameraRotMove + m_cameraJumpLerpAmount) + DirectX::XM_PI);
	}
	else {
		ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_cameraRotMove + m_cameraJumpLerpAmount);
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
	m_cameraJumpLerpStorage = m_cameraRotMove;

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

void Player::CheckZipline(const KuroEngine::Vec3<float> arg_newPos, std::weak_ptr<Stage> arg_nowStage) {

	//�W�b�v���C���Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//�W�b�v���C���ł͂Ȃ�
		if (terrian->GetType() != StageParts::IVY_ZIP_LINE)continue;

		//�W�b�v���C���Ƃ��ăL���X�g
		auto zipline = dynamic_pointer_cast<IvyZipLine>(terrian);

		//�W�b�v���C���ɓo�^����Ă��钸�_��1�ȉ��������珈�����΂��B
		if (static_cast<int>(zipline->GetTranslationArraySize() <= 1)) continue;

		//���聫============================================

		//�n�_�Ƃ̓����蔻��
		bool isHit = KuroEngine::Vec3<float>(zipline->GetStartPoint() - arg_newPos).Length() <= (m_transform.GetScale().x + zipline->JUMP_SCALE);
		if (isHit && m_canZip) {
			m_gimmickStatus = GIMMICK_STATUS::APPEAR;
			m_playerMoveStatus = PLAYER_MOVE_STATUS::ZIP;
			m_ziplineMoveTimer = 0;
			zipline->CheckHit(true);
			m_refZipline = zipline;
			m_zipInOutPos = arg_newPos;

			//SE��炷�B
			SoundConfig::Instance()->Play(SoundConfig::SE_ZIP_LINE_GET_ON);
		}

		//�I�_�Ƃ̓����蔻��
		isHit = KuroEngine::Vec3<float>(zipline->GetEndPoint() - arg_newPos).Length() <= (m_transform.GetScale().x + zipline->JUMP_SCALE);
		if (isHit && m_canZip) {
			m_gimmickStatus = GIMMICK_STATUS::APPEAR;
			m_playerMoveStatus = PLAYER_MOVE_STATUS::ZIP;
			m_ziplineMoveTimer = 0;
			zipline->CheckHit(false);
			m_refZipline = zipline;
			m_zipInOutPos = arg_newPos;

			//SE��炷�B
			SoundConfig::Instance()->Play(SoundConfig::SE_ZIP_LINE_GET_ON);
		}

		//=================================================

	}

}

void Player::UpdateZipline() {

	switch (m_gimmickStatus)
	{
	case Player::GIMMICK_STATUS::APPEAR:
	{

		//�W�b�v���C���̒��ɓ����Ă����^�C�}�[���X�V
		m_ziplineMoveTimer = std::clamp(m_ziplineMoveTimer + 1, 0, ZIP_LINE_MOVE_TIMER_START);

		//�C�[�W���O�̗ʂ����߂�B
		float timerRate = static_cast<float>(m_ziplineMoveTimer) / static_cast<float>(ZIP_LINE_MOVE_TIMER_START);

		//�ړ��ʂ̃C�[�W���O
		float moveEaseRate = KuroEngine::Math::Ease(KuroEngine::EASE_CHANGE_TYPE::In, KuroEngine::EASING_TYPE::Circ, timerRate, 0.0f, 1.0f);

		//�ړ�������B
		m_transform.SetPos(m_zipInOutPos + (m_refZipline.lock()->GetPoint(true) - m_zipInOutPos) * moveEaseRate);

		//�X�P�[���̃C�[�W���O
		float scaleEaseRate = KuroEngine::Math::Ease(KuroEngine::EASE_CHANGE_TYPE::In, KuroEngine::EASING_TYPE::Back, timerRate, 0.0f, 1.0f);

		//����������B
		m_transform.SetScale(1.0f - scaleEaseRate);

		if (ZIP_LINE_MOVE_TIMER_START <= m_ziplineMoveTimer) {

			//�W�b�v���C���𓮂����B
			m_refZipline.lock()->CanMovePlayer();

			//NORMAL�ɂ��ăv���C���[�͉������Ȃ��悤�ɂ���B
			m_gimmickStatus = GIMMICK_STATUS::NORMAL;

			m_ziplineMoveTimer = 0;

		}

	}
	break;
	case Player::GIMMICK_STATUS::NORMAL:
	{
		//m_zipInOutPos = m_transform.GetPosWorld();
	}
	break;
	case Player::GIMMICK_STATUS::EXIT:
	{

		//�W�b�v���C���̒��ɓ����Ă����^�C�}�[���X�V
		m_ziplineMoveTimer = std::clamp(m_ziplineMoveTimer + 1, 0, ZIP_LINE_MOVE_TIMER_END);

		//�C�[�W���O�̗ʂ����߂�B
		float timerRate = static_cast<float>(m_ziplineMoveTimer) / static_cast<float>(ZIP_LINE_MOVE_TIMER_END);

		//�ړ��ʂ̃C�[�W���O
		float moveEaseRate = KuroEngine::Math::Ease(KuroEngine::EASE_CHANGE_TYPE::Out, KuroEngine::EASING_TYPE::Circ, timerRate, 0.0f, 1.0f);

		//�ړ�������B
		m_transform.SetPos(m_refZipline.lock()->GetPoint(false) + (m_zipInOutPos - m_refZipline.lock()->GetPoint(false)) * moveEaseRate);

		//�X�P�[���̃C�[�W���O
		float scaleEaseRate = KuroEngine::Math::Ease(KuroEngine::EASE_CHANGE_TYPE::Out, KuroEngine::EASING_TYPE::Back, timerRate, 0.0f, 1.0f);

		//����������B
		m_transform.SetScale(scaleEaseRate);

		if (ZIP_LINE_MOVE_TIMER_END <= m_ziplineMoveTimer) {

			//�v���C���[�����ɖ߂��B
			m_playerMoveStatus = PLAYER_MOVE_STATUS::MOVE;

			m_ziplineMoveTimer = 0;

		}

	}
	break;
	default:
		break;
	}

}