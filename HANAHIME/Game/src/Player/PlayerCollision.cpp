#include "PlayerCollision.h"
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
#include"PlayerCollision.h"
#include"CollisionDetectionOfRayAndMesh.h"

bool PlayerCollision::HitCheckAndPushBack(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo)
{
	/*
	arg_from �c �ړ��O�̍��W
	arg_to �c �ړ���̍��W
	arg_terrianArray �c �n�`�̔z��
	arg_terrianNormal �c ���������n�`�̃��b�V���̖@���A�i�[��
	*/

	//CastRay�ɓn������
	PlayerCollision::CastRayArgument castRayArgument;
	castRayArgument.m_stageType = StageParts::TERRIAN;
	for (auto& index : castRayArgument.m_checkDeathCounter) {
		index = 0;
	}
	for (auto& index : castRayArgument.m_checkHitAround) {
		index = false;
	}

	//�n�ʂƂ̓����蔻��
	CheckHitGround(arg_from, arg_newPos, arg_nowStage, arg_hitInfo, castRayArgument);

	//���񂾂�(���܂��Ă��邩)�ǂ����𔻒�
	CheckDeath(arg_from, arg_newPos, arg_nowStage, arg_hitInfo, castRayArgument);

	//���͂̕ǂƂ̓����蔻��
	CheckHitAround(arg_from, arg_newPos, arg_nowStage, arg_hitInfo, castRayArgument);

	//�W�b�v���C���Ƃ̓����蔻��
	CheckZipline(arg_newPos, arg_nowStage);

	//����ł����珈�����΂��B
	bool isDeath = false;
	isDeath = false;
	isDeath |= (castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::RIGHT)] && castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::LEFT)]);
	isDeath |= (castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::TOP)] && castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::BOTTOM)]);
	isDeath |= (castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::FRONT)] && castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::BEHIND)]);
	if (isDeath) {
		++m_refPlayer->m_deathTimer;
	}
	else {
		m_refPlayer->m_deathTimer = 0;
	}

	//����ł�����
	if (m_refPlayer->DEATH_TIMER < m_refPlayer->m_deathTimer) {
		m_refPlayer->m_isDeath = true;
		return false;

	}

	return true;
}

void PlayerCollision::CheckDeath(const KuroEngine::Vec3<float> arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, PlayerCollision::CastRayArgument& arg_castRayArgment)
{

	const float DEATH_LENGTH = 1.0f;

	//�S�����Ƀ��C���΂��Ď��S�`�F�b�N�B
	CheckHitAllObject(&PlayerCollision::CheckHitDeath_Around, arg_newPos, arg_from, arg_castRayArgment, arg_nowStage);

}

void PlayerCollision::CheckHitAround(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, PlayerCollision::CastRayArgument& arg_castRayArgment) {


	//�v���C���[������ł���Ƃ��́A������Ƀ��C���΂��ď�ɍ򂪂Ȃ������`�F�b�N����B
	m_refPlayer->m_canOldUnderGroundRelease = m_refPlayer->m_canUnderGroundRelease;
	m_refPlayer->m_canUnderGroundRelease = true;
	//�t�F���X�Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::SPLATOON_FENCE)continue;

		//�����ɂ���ăJ�����O
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 2.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

		//��������Ƃ��ăL���X�g
		auto ivyBlock = dynamic_pointer_cast<SplatoonFence>(terrian);

		//���f�����擾
		auto model = terrian->GetModel();

		//���b�V���𑖍�
		for (auto& modelMesh : model.lock()->m_meshes)
		{

			//CastRay�ɓn���������X�V�B
			auto hitmesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//���聫============================================

			//�����蔻������s
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(m_refPlayer->GetNowPos(), m_refPlayer->GetTransform().GetUp(), hitmesh);

			if (output.m_isHit && fabs(output.m_distance) < m_refPlayer->GetTransform().GetScale().x * 2.0f) {

				m_refPlayer->m_canUnderGroundRelease = false;

				//�v���C���[���n���ɐ����Ă��Ȃ���������߂�B
				//if (!m_refPlayer->GetIsUnderGround()) {
				m_refPlayer->m_isUnderGround = true;
				m_refPlayer->m_underGroundEaseTimer = 1.0f;
				//}

			}

			//=================================================
		}
	}



	//�l���Ƀ��C���΂��ĉ����߂�
	CheckHitAllObject(&PlayerCollision::CheckHitAround_Around, arg_newPos, arg_from, arg_castRayArgment, arg_nowStage);

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
				m_refPlayer->m_canJump = true;
			}

			if (arg_castRayArgment.m_impactPoint[minIndex].m_isAppearWall) {
				arg_hitInfo->m_terrianNormal = m_refPlayer->m_transform.GetUp();
			}
			//�W�����v���ł����Ԃ�������W�����v����B
			else if (m_refPlayer->m_canJump) {

				//�ŒZ�̏Փ˓_�����߂���A������W�����v��ɂ���B
				arg_hitInfo->m_terrianNormal = arg_castRayArgment.m_impactPoint[minIndex].m_normal;

				//�W�����v�̃p�����[�^�[�����߂�B
				m_refPlayer->m_playerMoveStatus = Player::PLAYER_MOVE_STATUS::JUMP;
				m_refPlayer->m_jumpTimer = 0;
				m_refPlayer->m_jumpStartPos = arg_newPos;
				m_refPlayer->m_bezierCurveControlPos = m_refPlayer->m_jumpStartPos + m_refPlayer->m_transform.GetUp() * m_refPlayer->WALL_JUMP_LENGTH;
				m_refPlayer->m_jumpEndPos = arg_castRayArgment.m_impactPoint[minIndex].m_impactPos + arg_castRayArgment.m_impactPoint[minIndex].m_normal * (m_refPlayer->m_transform.GetScale().x / 2.0f);
				m_refPlayer->m_jumpEndPos += m_refPlayer->m_transform.GetUp() * m_refPlayer->WALL_JUMP_LENGTH;

				//�W�����v�����u�Ԃ̓��͂�ۑ����Ă����B
				m_refPlayer->m_jumpTrrigerRowMoveVec = m_refPlayer->m_jumpRowMoveVec;

				//�v���C���[�̓��͂𔽓]������B
				bool isHitCeiling = arg_hitInfo->m_terrianNormal.y < -0.9f && !m_refPlayer->m_isCameraUpInverse;
				bool isHitGround = 0.9f < arg_hitInfo->m_terrianNormal.y && m_refPlayer->m_isCameraUpInverse;
				if (isHitCeiling || isHitGround) {
					m_refPlayer->m_isCameraInvX = true;
				}

				//�V�䂩����������J�����𔽓]������B
				if (arg_hitInfo->m_terrianNormal.y < -0.9f) {

					m_refPlayer->m_isCameraUpInverse = true;

				}
				else if (0.9f < arg_hitInfo->m_terrianNormal.y) {

					m_refPlayer->m_isCameraUpInverse = false;

				}

				//�W�����v�����̂Ń^�C�}�[��������
				m_refPlayer->m_canJumpDelayTimer = 0;

			}
			//�W�����v���ł��Ȃ���Ԃ������牟���߂��B
			else {

				arg_newPos = arg_from + m_refPlayer->m_gimmickVel;
				arg_hitInfo->m_terrianNormal = m_refPlayer->m_transform.GetUp();

				//�����|����̃^�C�}�[���X�V ���̒l�����l����������W�����v�ł���悤�ɂȂ�
				++m_refPlayer->m_canJumpDelayTimer;

			}

			return;

		}

	}



	//�ǂ��Ƃ��������Ă��Ȃ������猻�݂̏�x�N�g����n�`�̏�x�N�g���Ƃ��Ă݂�B
	arg_hitInfo->m_terrianNormal = m_refPlayer->m_transform.GetUp();

	//�ǂ��ɂ������������ĂȂ��̂Ń^�C�}�[������������B
	m_refPlayer->m_canJumpDelayTimer = 0;


}

void PlayerCollision::CheckHitGround(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, PlayerCollision::CastRayArgument& arg_castRayArgment) {

	//�v���C���[�̉�]���l�����Ȃ��A�@����񂾂��������ꍇ�̃g�����X�t�H�[���B
	KuroEngine::Transform localTrans;
	localTrans.SetRotate(m_refPlayer->m_normalSpinQ);


	//�v���C���[�̉�]���l�����Ȃ���]�s�񂩂烌�C���΂��������擾����B
	KuroEngine::Vec3<float> rightDir = localTrans.GetRight();
	KuroEngine::Vec3<float> frontDir = localTrans.GetFront();

	//�M�~�b�N�ɓ������Ă��邩�ǂ����̕ϐ���������
	m_refPlayer->m_prevOnGimmick = m_refPlayer->m_onGimmick;
	m_refPlayer->m_onGimmick = false;

	//�ڒn�t���O��ۑ�
	m_refPlayer->m_prevOnGround = m_refPlayer->m_onGround;
	m_refPlayer->m_onGround = false;

	//�R�`�F�b�N�ŉ������Ƀ��C��L�΂��ۂ̃��C�̒���
	m_checkUnderRayLength = m_refPlayer->m_transform.GetScale().y * 2.0f;
	m_checkCliffRayLength = m_refPlayer->m_transform.GetScale().y * 10.0f;

	//�R����p�t���O
	m_isHitCliff = { false,false,false,false };

	//�l�����牺�����Ƀ��C���΂��ĊR�`�F�b�N�B
	CheckHitAllObject(&PlayerCollision::CheckHitCliff_Under, arg_newPos, arg_from, arg_castRayArgment, arg_nowStage);

	//�������Ă��Ȃ�������v���C���[�����Ƀ��C���΂��ďՓ˓_���L�^����B
	m_impactPoint = std::array<std::vector<KuroEngine::Vec3<float>>, 4>();
	m_impactPointBuff = KuroEngine::Vec3<float>();	//���W�ꎞ�ۑ��p
	CheckHitAllObject(&PlayerCollision::CheckHitCliff_SearchImpactPoint, arg_newPos, arg_from, arg_castRayArgment, arg_nowStage);

	//���߂�ꂽ�Փ˓_�̒�����e�����̈�ԋ߂��Փ˓_��������B
	std::vector<KuroEngine::Vec3<float>> nearPos;
	std::vector<KuroEngine::Vec3<float>> nearestPos;
	//�܂��͉E����
	for (auto& index : m_impactPoint[static_cast<int>(RAY_DIR_ID::RIGHT)]) {

		float minLength = std::numeric_limits<float>().max();
		KuroEngine::Vec3<float> castPos = arg_newPos + rightDir * m_refPlayer->WALL_JUMP_LENGTH - m_refPlayer->m_transform.GetUp() * m_checkUnderRayLength;
		float length = KuroEngine::Vec3<float>(castPos - index).Length();
		if (length < minLength) {
			minLength = length;
			nearPos.emplace_back(index);
		}

	}
	if (0 < nearPos.size())nearestPos.emplace_back(nearPos.back());
	nearPos.clear();
	//���͍�
	for (auto& index : m_impactPoint[static_cast<int>(RAY_DIR_ID::LEFT)]) {

		float minLength = std::numeric_limits<float>().max();
		KuroEngine::Vec3<float> castPos = arg_newPos - rightDir * m_refPlayer->WALL_JUMP_LENGTH - m_refPlayer->m_transform.GetUp() * m_checkUnderRayLength;
		float length = KuroEngine::Vec3<float>(castPos - index).Length();
		if (length < minLength) {
			minLength = length;
			nearPos.emplace_back(index);
		}

	}
	if (0 < nearPos.size())nearestPos.emplace_back(nearPos.back());
	nearPos.clear();
	//���͑O
	for (auto& index : m_impactPoint[static_cast<int>(RAY_DIR_ID::FRONT)]) {

		float minLength = std::numeric_limits<float>().max();
		KuroEngine::Vec3<float> castPos = arg_newPos + frontDir * m_refPlayer->WALL_JUMP_LENGTH - m_refPlayer->m_transform.GetUp() * m_checkUnderRayLength;
		float length = KuroEngine::Vec3<float>(castPos - index).Length();
		if (length < minLength) {
			minLength = length;
			nearPos.emplace_back(index);
		}
	}
	if (0 < nearPos.size())nearestPos.emplace_back(nearPos.back());
	nearPos.clear();
	//�Ō�Ɍ��
	for (auto& index : m_impactPoint[static_cast<int>(RAY_DIR_ID::BEHIND)]) {

		float minLength = std::numeric_limits<float>().max();
		KuroEngine::Vec3<float> castPos = arg_newPos - frontDir * m_refPlayer->WALL_JUMP_LENGTH - m_refPlayer->m_transform.GetUp() * m_checkUnderRayLength;
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
			moveQtransform.SetRotate(m_refPlayer->m_moveQ);

			m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos + moveQtransform.GetFront() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
			m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos - moveQtransform.GetFront() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
			m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos + moveQtransform.GetRight() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
			m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos - moveQtransform.GetRight() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);

			//=================================================
		}
	}
	//��������Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::MOVE_SCAFFOLD)continue;

		//�����ɂ���ăJ�����O
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 100.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

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
			moveQtransform.SetRotate(m_refPlayer->m_moveQ);

			m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos + moveQtransform.GetFront() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
			m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos - moveQtransform.GetFront() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
			m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos + moveQtransform.GetRight() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
			m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos - moveQtransform.GetRight() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);


			//�܂��͐^���Ƀ��C���΂��B
			bool isHit = CastRay(arg_newPos, arg_newPos, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

			//���ɓ����������̌�둤���烌�C���΂��ē������Ă�����M�~�b�N���N������B
			if (isHit) {

				m_refPlayer->m_onGimmick = true;

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

		//�����ɂ���ăJ�����O
		const float DEADLINE = 100.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

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
				moveQtransform.SetRotate(m_refPlayer->m_moveQ);

				m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos + moveQtransform.GetFront() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
				m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos - moveQtransform.GetFront() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
				m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos + moveQtransform.GetRight() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
				m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos - moveQtransform.GetRight() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);

				//�o�����Ă�����
				if (ivyBlock->GetIsAppear()) {

					//�ړ�������̌��̕��Ƀ��C���΂��B
					bool isHit = CastRay(arg_newPos, arg_newPos - moveQtransform.GetFront() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

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

void PlayerCollision::CheckCliff(PlayerCollision::ImpactPointData& arg_impactPointData, std::weak_ptr<Stage> arg_nowStage)
{

	//�L��������Ă��Ȃ������珈�����΂��B
	if (!arg_impactPointData.m_isActive) return;

	//�R����p�̃��C�̒���
	const float CLIFF_RAY_LENGTH = m_refPlayer->WALL_JUMP_LENGTH + m_refPlayer->m_transform.GetScale().Length();

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
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * 0.5f, -m_refPlayer->m_transform.GetUp(), mesh);

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

		//�����ɂ���ăJ�����O
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 100.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

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
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * 0.5f, -m_refPlayer->m_transform.GetUp(), mesh);

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

		//�����ɂ���ăJ�����O
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 5.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

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
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * 0.5f, -m_refPlayer->m_transform.GetUp(), mesh);

			//���C�����b�V���ɏՓ˂��Ă���A�Փ˒n�_�܂ł̋��������C�̒�����菬����������Փ˂��Ă���B
			if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

				//�ǂɓ����������_�ŊR�ł͂Ȃ��̂ŏ������΂��B
				return;

			}
			//=================================================
		}
	}

	//�����Ȃ��ǂƂ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::APPEARANCE)continue;

		//��������Ƃ��ăL���X�g
		auto ivyBlock = dynamic_pointer_cast<Appearance>(terrian);

		//���f�����擾
		auto model = terrian->GetModel();

		//���b�V���𑖍�
		for (auto& modelMesh : model.lock()->m_meshes)
		{

			//�����蔻����s�����b�V���B
			std::vector<TerrianHitPolygon> mesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//�������Ƀ��C���΂��B
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * 0.5f, -m_refPlayer->m_transform.GetUp(), mesh);

			//���C�����b�V���ɏՓ˂��Ă���A�Փ˒n�_�܂ł̋��������C�̒�����菬����������Փ˂��Ă���B
			if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

				//�ǂɓ����������_�ŊR�ł͂Ȃ��̂ŏ������΂��B
				return;

			}
			//=================================================
		}
	}

	//�t�F���X�Ƃ̓����蔻��
	if (!m_refPlayer->GetIsUnderGround()) {
		for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
		{
			//��������łȂ�
			if (terrian->GetType() != StageParts::SPLATOON_FENCE)continue;

			//�����ɂ���ăJ�����O
			const float DEADLINE = terrian->GetTransform().GetScale().Length() * 2.0f;
			float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
			if (DEADLINE < distance) continue;

			//��������Ƃ��ăL���X�g
			auto ivyBlock = dynamic_pointer_cast<SplatoonFence>(terrian);

			//���f�����擾
			auto model = terrian->GetModel();

			//���b�V���𑖍�
			for (auto& modelMesh : model.lock()->m_meshes)
			{

				//�����蔻����s�����b�V���B
				std::vector<TerrianHitPolygon> mesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

				//�������Ƀ��C���΂��B
				CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * 0.5f, -m_refPlayer->m_transform.GetUp(), mesh);

				//���C�����b�V���ɏՓ˂��Ă���A�Փ˒n�_�܂ł̋��������C�̒�����菬����������Փ˂��Ă���B
				if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

					//�ǂɓ����������_�ŊR�ł͂Ȃ��̂ŏ������΂��B
					return;

				}
				//=================================================
			}
		}
	}

	//�Ō�܂ŕǂɓ������ĂȂ�������R�𒴂��Ă���̂Ŗ���������B
	arg_impactPointData.m_isActive = false;

}

void PlayerCollision::CheckCanJump(PlayerCollision::ImpactPointData& arg_impactPointData, std::weak_ptr<Stage> arg_nowStage)
{

	//�L��������Ă��Ȃ������珈�����΂��B
	if (!arg_impactPointData.m_isActive) return;

	//�R����p�̃��C�̒���
	const float CLIFF_RAY_LENGTH = m_refPlayer->WALL_JUMP_LENGTH + m_refPlayer->m_transform.GetScale().Length();

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
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision((arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * m_refPlayer->m_transform.GetScale().x) + m_refPlayer->m_transform.GetUp() * m_refPlayer->WALL_JUMP_LENGTH, -arg_impactPointData.m_normal, mesh);

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

		//�����ɂ���ăJ�����O
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 100.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

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
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision((arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * m_refPlayer->m_transform.GetScale().x) + m_refPlayer->m_transform.GetUp() * m_refPlayer->WALL_JUMP_LENGTH, -arg_impactPointData.m_normal, mesh);

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

		//�����ɂ���ăJ�����O
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 5.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

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
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision((arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * m_refPlayer->m_transform.GetScale().x) + m_refPlayer->m_transform.GetUp() * m_refPlayer->WALL_JUMP_LENGTH, -arg_impactPointData.m_normal, mesh);

			//���C�����b�V���ɏՓ˂��Ă���A�Փ˒n�_�܂ł̋��������C�̒�����菬����������Փ˂��Ă���B
			if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

				//�ǂɓ����������_�ŊR�ł͂Ȃ��̂ŏ������΂��B
				return;

			}

			//=================================================
		}
	}

	//�����Ȃ��ǂƂ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::APPEARANCE)continue;

		//�����ɂ���ăJ�����O
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 100.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

		//�����Ȃ��ǂƂ��ăL���X�g
		auto appearWall = dynamic_pointer_cast<Appearance>(terrian);

		//���f�����擾
		auto model = terrian->GetModel();

		//���b�V���𑖍�
		for (auto& modelMesh : model.lock()->m_meshes) {

			//���聫============================================

			//�����蔻����s�����b�V���B
			std::vector<TerrianHitPolygon> mesh = appearWall->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//�������Ƀ��C���΂��B
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision((arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * m_refPlayer->m_transform.GetScale().x) + m_refPlayer->m_transform.GetUp() * m_refPlayer->WALL_JUMP_LENGTH, -arg_impactPointData.m_normal, mesh);

			//���C�����b�V���ɏՓ˂��Ă���A�Փ˒n�_�܂ł̋��������C�̒�����菬����������Փ˂��Ă���B
			if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

				//�ǂɓ����������_�ŊR�ł͂Ȃ��̂ŏ������΂��B
				return;

			}

			//=================================================
		}
	}

	//�t�F���X�Ƃ̓����蔻��
	if (!m_refPlayer->GetIsUnderGround()) {
		for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
		{
			//��������łȂ�
			if (terrian->GetType() != StageParts::SPLATOON_FENCE)continue;

			//�����ɂ���ăJ�����O
			const float DEADLINE = terrian->GetTransform().GetScale().Length() * 2.0f;
			float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
			if (DEADLINE < distance) continue;

			//�����Ȃ��ǂƂ��ăL���X�g
			auto appearWall = dynamic_pointer_cast<SplatoonFence>(terrian);

			//���f�����擾
			auto model = terrian->GetModel();

			//���b�V���𑖍�
			for (auto& modelMesh : model.lock()->m_meshes) {

				//���聫============================================

				//�����蔻����s�����b�V���B
				std::vector<TerrianHitPolygon> mesh = appearWall->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

				//�������Ƀ��C���΂��B
				CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision((arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * m_refPlayer->m_transform.GetScale().x) + m_refPlayer->m_transform.GetUp() * m_refPlayer->WALL_JUMP_LENGTH, -arg_impactPointData.m_normal, mesh);

				//���C�����b�V���ɏՓ˂��Ă���A�Փ˒n�_�܂ł̋��������C�̒�����菬����������Փ˂��Ă���B
				if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

					//�ǂɓ����������_�ŊR�ł͂Ȃ��̂ŏ������΂��B
					return;

				}

				//=================================================
			}
		}
	}

	//�Ō�܂ŕǂɓ������ĂȂ�������R�𒴂��Ă���̂Ŗ���������B
	arg_impactPointData.m_isActive = false;

}

bool PlayerCollision::CastRay(KuroEngine::Vec3<float>& arg_charaPos, const KuroEngine::Vec3<float>& arg_rayCastPos, const KuroEngine::Vec3<float>& arg_rayDir, float arg_rayLength, CastRayArgument& arg_collisionData, RAY_ID arg_rayID, RAY_DIR_ID arg_rayDirID)
{

	/*===== �����蔻��p�̃��C������ =====*/

	//���C���΂��B
	CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_rayCastPos, arg_rayDir, arg_collisionData.m_mesh);

	//���C�����b�V���ɏՓ˂��Ă���A�Փ˒n�_�܂ł̋��������C�̒�����菬����������Փ˂��Ă���B
	if (output.m_isHit && std::fabs(output.m_distance) < arg_rayLength) {

		//�҂����艟���߂��Ă��܂��Əd�͂̊֌W�ŃK�N�K�N���Ă��܂��̂ŁA�����ɂ߂荞�܂��ĉ����߂��B
		static const float OFFSET = 0.1f;

		//���C�̎�ނɂ���ĕۑ�����f�[�^��ς���B
		switch (arg_rayID)
		{
		case RAY_ID::GROUND:

			//�O���ɓn���p�̃f�[�^��ۑ�
			arg_collisionData.m_bottomTerrianNormal = output.m_normal;

			//�����߂��B
			arg_charaPos += output.m_normal * (std::fabs(output.m_distance - m_refPlayer->m_transform.GetScale().x) - OFFSET);

			m_refPlayer->m_underRayHitPosition = output.m_pos;

			break;

		case RAY_ID::AROUND:

			//���C�̏Փ˒n�_��ۑ��B
			arg_collisionData.m_impactPoint.emplace_back(ImpactPointData(output.m_pos, output.m_normal));

			//��������������߂荞��ł��܂��̂ŉ����߂��B
			if (arg_collisionData.m_stageType == StageParts::MOVE_SCAFFOLD) {

				arg_charaPos += output.m_normal * (std::fabs(output.m_distance - m_refPlayer->m_transform.GetScale().x) - OFFSET);

			}
			//�����Ȃ��ǂ��t�F���X��������
			else if (arg_collisionData.m_stageType == StageParts::APPEARANCE || arg_collisionData.m_stageType == StageParts::SPLATOON_FENCE) {

				arg_charaPos += output.m_normal * (std::fabs(output.m_distance - arg_rayLength) - OFFSET);
				arg_collisionData.m_impactPoint.back().m_isAppearWall = true;
				arg_collisionData.m_bottomTerrianNormal = m_refPlayer->m_transform.GetUp();

			}
			else if (m_refPlayer->m_prevOnGimmick) {

				arg_collisionData.m_impactPoint.back().m_isFastJump = true;

			}

			break;

		case RAY_ID::CLIFF:
		{

			//�O���ɓn���p�̃f�[�^��ۑ�
			arg_collisionData.m_bottomTerrianNormal = m_refPlayer->m_transform.GetUp();

			//�Փ˒n�_���������Ɉʒu�����炷�B
			const float m_checkUnderRayLength = m_refPlayer->m_transform.GetScale().y * 2.0f;
			output.m_pos += m_refPlayer->m_transform.GetUp() * m_checkUnderRayLength;

			//�����߂����ʒu�ɍ��W��ݒ�B
			arg_charaPos = output.m_pos - (output.m_normal * m_refPlayer->m_transform.GetScale().x * (m_refPlayer->WALL_JUMP_LENGTH - OFFSET));

		}
		break;

		case RAY_ID::CHECK_GIMMICK:

			m_refPlayer->m_onGimmick = true;

			//����ɃM�~�b�N�ɓ��������g���K�[��������M�~�b�N��L����������B
			if (!m_refPlayer->m_prevOnGimmick) {
				dynamic_pointer_cast<MoveScaffold>(arg_collisionData.m_stage.lock())->Activate();
			}


			break;

		case RAY_ID::CHECK_DEATH:

			arg_collisionData.m_checkDeathCounter[static_cast<int>(arg_rayDirID)] = true;

			break;

		case RAY_ID::CHECK_CLIFF:

			break;

		case RAY_ID::CHECK_IVY:

			m_refPlayer->m_gimmickExitPos.emplace_back(output.m_pos + output.m_normal);
			m_refPlayer->m_gimmickExitNormal.emplace_back(output.m_normal);

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

void PlayerCollision::CheckHit(KuroEngine::Vec3<float>& arg_frompos, KuroEngine::Vec3<float>& arg_nowpos, std::weak_ptr<Stage>arg_nowStage) {

	HitCheckResult hitResult;
	if (!HitCheckAndPushBack(arg_frompos, arg_nowpos, arg_nowStage, &hitResult))return;

	//�n�`�̖@�����^���������Ă���Ƃ��Ɍ덷�ł��ꂢ��0,-1,0�ɂȂ��Ă���Ȃ������ł��܂������Ȃ��̂ŋ���̍�B
	if (hitResult.m_terrianNormal.y < -0.9f) {
		hitResult.m_terrianNormal = { 0,-1,0 };
	}

	//�@������������N�H�[�^�j�I��
	m_refPlayer->m_normalSpinQ = KuroEngine::Math::GetLookAtQuaternion({ 0,1,0 }, hitResult.m_terrianNormal);

	//�J�����̉�]��Y����]������N�H�[�^�j�I���B�ړ������ɉ�]���Ă���悤�Ɍ��������邽�߂̂��́Bm_refPlayer->m_cameraJumpLerpAmount�͕�Ԍ�̃J�����Ɍ������ĕ�Ԃ��邽�߁B
	DirectX::XMVECTOR ySpin;
	if (m_refPlayer->m_isCameraUpInverse) {
		ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, -(m_refPlayer->m_cameraRotMove + m_refPlayer->m_cameraJumpLerpAmount) + DirectX::XM_PI);
	}
	else {
		ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_refPlayer->m_cameraRotMove + m_refPlayer->m_cameraJumpLerpAmount);
	}

	//�v���C���[�̈ړ�������Y����]������N�H�[�^�j�I���B�ړ������ɉ�]���Ă���悤�Ɍ��������邽�߂̂��́B
	DirectX::XMVECTOR playerYSpin;
	playerYSpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_refPlayer->m_playerRotY);

	//�J���������ł̃N�H�[�^�j�I�������߂�B�i�ޕ����Ȃǂ𔻒f����̂Ɏg�p����̂͂������BF�̈�ԍŏ��ɂ��̒l�����邱�Ƃ�playerYSpin�̉�]��ł������B
	m_refPlayer->m_cameraQ = DirectX::XMQuaternionMultiply(m_refPlayer->m_normalSpinQ, ySpin);

	//�v���C���[�̈ړ�������Y����]������N�H�[�^�j�I�����J�����̃N�H�[�^�j�I���ɂ����āA�v���C���[���ړ������Ɍ�������B
	m_refPlayer->m_moveQ = DirectX::XMQuaternionMultiply(m_refPlayer->m_cameraQ, playerYSpin);

	//�W�����v��Ԃ�������
	if (m_refPlayer->m_playerMoveStatus == Player::PLAYER_MOVE_STATUS::JUMP) {

		//�W�����v��ɉ�]����悤�ɂ���B

		//�N�H�[�^�j�I����ۑ��B
		m_refPlayer->m_jumpEndQ = m_refPlayer->m_moveQ;
		m_refPlayer->m_jumpStartQ = m_refPlayer->m_prevTransform.GetRotate();
		m_refPlayer->m_transform.SetRotate(m_refPlayer->m_prevTransform.GetRotate());

	}
	else {

		//���������ʊ�̉�]�ɂ���B
		m_refPlayer->m_transform.SetRotate(m_refPlayer->m_moveQ);

	}

}

void PlayerCollision::CheckZipline(const KuroEngine::Vec3<float> arg_newPos, std::weak_ptr<Stage> arg_nowStage) {

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
		bool isHit = KuroEngine::Vec3<float>(zipline->GetStartPoint() - arg_newPos).Length() <= (m_refPlayer->m_transform.GetScale().x + zipline->JUMP_SCALE);
		if (isHit && m_refPlayer->m_canZip) {
			m_refPlayer->m_gimmickStatus = Player::GIMMICK_STATUS::APPEAR;
			m_refPlayer->m_playerMoveStatus = Player::PLAYER_MOVE_STATUS::ZIP;
			m_refPlayer->m_ziplineMoveTimer = 0;
			zipline->CheckHit(true);
			m_refPlayer->m_refZipline = zipline;
			m_refPlayer->m_zipInOutPos = arg_newPos;

			//SE��炷�B
			SoundConfig::Instance()->Play(SoundConfig::SE_ZIP_LINE_GET_ON);
		}

		//�I�_�Ƃ̓����蔻��
		isHit = KuroEngine::Vec3<float>(zipline->GetEndPoint() - arg_newPos).Length() <= (m_refPlayer->m_transform.GetScale().x + zipline->JUMP_SCALE);
		if (isHit && m_refPlayer->m_canZip) {
			m_refPlayer->m_gimmickStatus = Player::GIMMICK_STATUS::APPEAR;
			m_refPlayer->m_playerMoveStatus = Player::PLAYER_MOVE_STATUS::ZIP;
			m_refPlayer->m_ziplineMoveTimer = 0;
			zipline->CheckHit(false);
			m_refPlayer->m_refZipline = zipline;
			m_refPlayer->m_zipInOutPos = arg_newPos;

			//SE��炷�B
			SoundConfig::Instance()->Play(SoundConfig::SE_ZIP_LINE_GET_ON);
		}

		//=================================================

	}

}

void PlayerCollision::FinishGimmickMove()
{

	//�S�����ɒn�ʗp�̃��C���΂��Ēn�ʔ��������B

	m_refPlayer->m_gimmickExitPos.clear();
	m_refPlayer->m_gimmickExitNormal.clear();

	//�����߂����W
	KuroEngine::Vec3<float> pos = m_refPlayer->m_transform.GetPosWorld();

	PlayerCollision::CastRayArgument castRayArgument;

	//���C�̒���
	const float RAY_LENGTH = 10.0f;

	//�n�`�z�񑖍�
	for (auto& terrian : m_refPlayer->m_stage.lock()->GetTerrianArray())
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
			CastRay(pos, pos, m_refPlayer->m_transform.GetRight(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, -m_refPlayer->m_transform.GetRight(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, -m_refPlayer->m_transform.GetFront(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//���ʕ����Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, m_refPlayer->m_transform.GetFront(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, -m_refPlayer->m_transform.GetUp(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, m_refPlayer->m_transform.GetUp(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//=================================================
		}
	}

	//��������Ƃ̓����蔻��
	for (auto& terrian : m_refPlayer->m_stage.lock()->GetGimmickArray())
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
			CastRay(pos, pos, m_refPlayer->m_transform.GetRight(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, -m_refPlayer->m_transform.GetRight(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, -m_refPlayer->m_transform.GetFront(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//���ʕ����Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, m_refPlayer->m_transform.GetFront(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//�������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, -m_refPlayer->m_transform.GetUp(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//������Ƀ��C���΂��B����͕ǂɂ������p�B
			CastRay(pos, pos, m_refPlayer->m_transform.GetUp(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//=================================================
		}
	}

	//�ŒZ�̂��̂���������B
	KuroEngine::Vec3<float> minPos = pos;
	KuroEngine::Vec3<float> normal = { 0,1,0 };
	float minLength = std::numeric_limits<float>().max();
	for (int index = 0; index < static_cast<int>(m_refPlayer->m_gimmickExitNormal.size()); ++index) {
		float length = KuroEngine::Vec3<float>(pos - m_refPlayer->m_gimmickExitPos[index]).Length();
		if (length < minLength) {
			minLength = length;
			minPos = m_refPlayer->m_gimmickExitPos[index];
			normal = m_refPlayer->m_gimmickExitNormal[index];
		}
	}

	m_refPlayer->m_zipInOutPos = minPos;

	//�n�`�̖@�����^���������Ă���Ƃ��Ɍ덷�ł��ꂢ��0,-1,0�ɂȂ��Ă���Ȃ������ł��܂������Ȃ��̂ŋ���̍�B
	if (normal.y < -0.9f) {
		normal = { 0,-1,0 };
	}

	//�@������������N�H�[�^�j�I��
	m_refPlayer->m_normalSpinQ = KuroEngine::Math::GetLookAtQuaternion({ 0,1,0 }, normal);

	//�J�����̉�]��Y����]������N�H�[�^�j�I���B�ړ������ɉ�]���Ă���悤�Ɍ��������邽�߂̂��́Bm_refPlayer->m_cameraJumpLerpAmount�͕�Ԍ�̃J�����Ɍ������ĕ�Ԃ��邽�߁B
	DirectX::XMVECTOR ySpin;
	if (normal.y < -0.9f) {
		ySpin = DirectX::XMQuaternionRotationNormal(normal, -(m_refPlayer->m_cameraRotMove + m_refPlayer->m_cameraJumpLerpAmount) + DirectX::XM_PI);
	}
	else {
		ySpin = DirectX::XMQuaternionRotationNormal(normal, m_refPlayer->m_cameraRotMove + m_refPlayer->m_cameraJumpLerpAmount);
	}

	//�v���C���[�̈ړ�������Y����]������N�H�[�^�j�I���B�ړ������ɉ�]���Ă���悤�Ɍ��������邽�߂̂��́B
	DirectX::XMVECTOR playerYSpin;
	playerYSpin = DirectX::XMQuaternionRotationNormal(normal, m_refPlayer->m_playerRotY);

	//�J���������ł̃N�H�[�^�j�I�������߂�B�i�ޕ����Ȃǂ𔻒f����̂Ɏg�p����̂͂������BF�̈�ԍŏ��ɂ��̒l�����邱�Ƃ�playerYSpin�̉�]��ł������B
	m_refPlayer->m_cameraQ = DirectX::XMQuaternionMultiply(m_refPlayer->m_normalSpinQ, ySpin);

	//�v���C���[�̈ړ�������Y����]������N�H�[�^�j�I�����J�����̃N�H�[�^�j�I���ɂ����āA�v���C���[���ړ������Ɍ�������B
	m_refPlayer->m_moveQ = DirectX::XMQuaternionMultiply(m_refPlayer->m_cameraQ, playerYSpin);

	//�W�����v��Ԃ�������
	if (m_refPlayer->m_playerMoveStatus == Player::PLAYER_MOVE_STATUS::JUMP) {

		//�W�����v��ɉ�]����悤�ɂ���B

		//�N�H�[�^�j�I����ۑ��B
		m_refPlayer->m_jumpEndQ = m_refPlayer->m_moveQ;
		m_refPlayer->m_jumpStartQ = m_refPlayer->m_prevTransform.GetRotate();
		m_refPlayer->m_transform.SetRotate(m_refPlayer->m_prevTransform.GetRotate());

	}
	else {

		//���������ʊ�̉�]�ɂ���B
		m_refPlayer->m_transform.SetRotate(m_refPlayer->m_moveQ);

	}

	m_refPlayer->m_gimmickStatus = Player::GIMMICK_STATUS::EXIT;

	//SE��炷�B
	SoundConfig::Instance()->Play(SoundConfig::SE_ZIP_LINE_GET_ON);
}

void PlayerCollision::CheckHitCliff_Under(KuroEngine::Vec3<float>& arg_newPos, const KuroEngine::Vec3<float>& arg_from, PlayerCollision::CastRayArgument& arg_castRayArgment, std::weak_ptr<Stage> arg_nowStage)
{

	//�v���C���[�̉�]���l�����Ȃ��A�@����񂾂��������ꍇ�̃g�����X�t�H�[���B
	KuroEngine::Transform localTrans;
	localTrans.SetRotate(m_refPlayer->m_normalSpinQ);

	//�v���C���[�̉�]���l�����Ȃ���]�s�񂩂烌�C���΂��������擾����B
	KuroEngine::Vec3<float> rightDir = localTrans.GetRight();
	KuroEngine::Vec3<float> frontDir = localTrans.GetFront();

	//�E���̎��͂̃��C���������Ă��Ȃ�������B
	if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::RIGHT)]) {

		//�������Ƀ��C���΂��Ă������R�����`�F�b�N����B
		KuroEngine::Vec3<float> rayPos = arg_newPos + rightDir * m_refPlayer->WALL_JUMP_LENGTH;
		m_isHitCliff[static_cast<int>(RAY_DIR_ID::RIGHT)] |= CastRay(arg_newPos, rayPos, -m_refPlayer->m_transform.GetUp(), m_checkUnderRayLength, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

	}

	//�E���̎��͂̃��C���������Ă��Ȃ�������B
	if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::LEFT)]) {

		//�������Ƀ��C���΂��Ă������R�����`�F�b�N����B
		KuroEngine::Vec3<float> rayPos = arg_newPos - rightDir * m_refPlayer->WALL_JUMP_LENGTH;
		m_isHitCliff[static_cast<int>(RAY_DIR_ID::LEFT)] |= CastRay(arg_newPos, rayPos, -m_refPlayer->m_transform.GetUp(), m_checkUnderRayLength, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

	}

	//�E���̎��͂̃��C���������Ă��Ȃ�������B
	if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::FRONT)]) {

		//�������Ƀ��C���΂��Ă������R�����`�F�b�N����B
		KuroEngine::Vec3<float> rayPos = arg_newPos + frontDir * m_refPlayer->WALL_JUMP_LENGTH;
		m_isHitCliff[static_cast<int>(RAY_DIR_ID::FRONT)] |= CastRay(arg_newPos, rayPos, -m_refPlayer->m_transform.GetUp(), m_checkUnderRayLength, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

	}

	//�E���̎��͂̃��C���������Ă��Ȃ�������B
	if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::BEHIND)]) {

		//�������Ƀ��C���΂��Ă������R�����`�F�b�N����B
		KuroEngine::Vec3<float> rayPos = arg_newPos - frontDir * m_refPlayer->WALL_JUMP_LENGTH;
		m_isHitCliff[static_cast<int>(RAY_DIR_ID::BEHIND)] |= CastRay(arg_newPos, rayPos, -m_refPlayer->m_transform.GetUp(), m_checkUnderRayLength, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

	}

}

void PlayerCollision::CheckHitCliff_SearchImpactPoint(KuroEngine::Vec3<float>& arg_newPos, const KuroEngine::Vec3<float>& arg_from, PlayerCollision::CastRayArgument& arg_castRayArgment, std::weak_ptr<Stage> arg_nowStage)
{

	//�v���C���[�̉�]���l�����Ȃ��A�@����񂾂��������ꍇ�̃g�����X�t�H�[���B
	KuroEngine::Transform localTrans;
	localTrans.SetRotate(m_refPlayer->m_normalSpinQ);

	//�v���C���[�̉�]���l�����Ȃ���]�s�񂩂烌�C���΂��������擾����B
	KuroEngine::Vec3<float> rightDir = localTrans.GetRight();
	KuroEngine::Vec3<float> frontDir = localTrans.GetFront();

	//�E���̎��͂̃��C���������Ă��Ȃ�������B
	if (!m_isHitCliff[static_cast<int>(RAY_DIR_ID::RIGHT)]) {

		KuroEngine::Vec3<float> rayPos = arg_newPos + rightDir * m_refPlayer->WALL_JUMP_LENGTH;
		if (CastRay(m_impactPointBuff, rayPos - m_refPlayer->m_transform.GetUp() * m_checkUnderRayLength, -rightDir, m_checkCliffRayLength, arg_castRayArgment, RAY_ID::CLIFF)) {
			m_impactPoint[static_cast<int>(RAY_DIR_ID::RIGHT)].emplace_back(m_impactPointBuff);
		}

	}

	//�E���̎��͂̃��C���������Ă��Ȃ�������B
	if (!m_isHitCliff[static_cast<int>(RAY_DIR_ID::LEFT)]) {

		KuroEngine::Vec3<float> rayPos = arg_newPos - rightDir * m_refPlayer->WALL_JUMP_LENGTH;
		if (CastRay(m_impactPointBuff, rayPos - m_refPlayer->m_transform.GetUp() * m_checkUnderRayLength, rightDir, m_checkCliffRayLength, arg_castRayArgment, RAY_ID::CLIFF)) {
			m_impactPoint[static_cast<int>(RAY_DIR_ID::LEFT)].emplace_back(m_impactPointBuff);
		}

	}


	//�E���̎��͂̃��C���������Ă��Ȃ�������B
	if (!m_isHitCliff[static_cast<int>(RAY_DIR_ID::FRONT)]) {

		KuroEngine::Vec3<float> rayPos = arg_newPos + frontDir * m_refPlayer->WALL_JUMP_LENGTH;
		if (CastRay(m_impactPointBuff, rayPos - m_refPlayer->m_transform.GetUp() * m_checkUnderRayLength, -frontDir, m_checkCliffRayLength, arg_castRayArgment, RAY_ID::CLIFF)) {
			m_impactPoint[static_cast<int>(RAY_DIR_ID::FRONT)].emplace_back(m_impactPointBuff);
		}

	}

	//�E���̎��͂̃��C���������Ă��Ȃ�������B
	if (!m_isHitCliff[static_cast<int>(RAY_DIR_ID::BEHIND)]) {

		KuroEngine::Vec3<float> rayPos = arg_newPos - frontDir * m_refPlayer->WALL_JUMP_LENGTH;
		if (CastRay(m_impactPointBuff, rayPos - m_refPlayer->m_transform.GetUp() * m_checkUnderRayLength, frontDir, m_checkCliffRayLength, arg_castRayArgment, RAY_ID::CLIFF)) {
			m_impactPoint[static_cast<int>(RAY_DIR_ID::BEHIND)].emplace_back(m_impactPointBuff);
		}

	}

}

void PlayerCollision::CheckHitDeath_Around(KuroEngine::Vec3<float>& arg_newPos, const KuroEngine::Vec3<float>& arg_from, PlayerCollision::CastRayArgument& arg_castRayArgment, std::weak_ptr<Stage> arg_nowStage)
{

	const float DEATH_LENGTH = 1.0f;

	//�X�v���t�F���X�͎��S������s��Ȃ��B
	if (arg_castRayArgment.m_stageType == StageParts::SPLATOON_FENCE) return;

	//�E�����Ƀ��C���΂��B
	CastRay(arg_newPos, arg_newPos, m_refPlayer->m_transform.GetRight(), DEATH_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::RIGHT);

	//�������Ƀ��C���΂��B
	CastRay(arg_newPos, arg_newPos, -m_refPlayer->m_transform.GetRight(), DEATH_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::LEFT);

	//�������Ƀ��C���΂��B
	CastRay(arg_newPos, arg_newPos, -m_refPlayer->m_transform.GetFront(), DEATH_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::BEHIND);

	//���ʕ����Ƀ��C���΂��B
	CastRay(arg_newPos, arg_newPos, m_refPlayer->m_transform.GetFront(), DEATH_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::FRONT);

	//������Ƀ��C���΂��B
	CastRay(arg_newPos, arg_newPos, m_refPlayer->m_transform.GetUp(), DEATH_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::TOP);

	CastRay(arg_newPos, arg_newPos, -m_refPlayer->m_transform.GetUp(), DEATH_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::BOTTOM);

}

void PlayerCollision::CheckHitAround_Around(KuroEngine::Vec3<float>& arg_newPos, const KuroEngine::Vec3<float>& arg_from, PlayerCollision::CastRayArgument& arg_castRayArgment, std::weak_ptr<Stage> arg_nowStage)
{

	//�v���C���[�̉�]���l�����Ȃ��A�@����񂾂��������ꍇ�̃g�����X�t�H�[���B
	KuroEngine::Transform localTrans;
	localTrans.SetRotate(m_refPlayer->m_normalSpinQ);

	//�v���C���[�̉�]���l�����Ȃ���]�s�񂩂烌�C���΂��������擾����B
	KuroEngine::Vec3<float> rightDir = localTrans.GetRight();
	KuroEngine::Vec3<float> frontDir = localTrans.GetFront();

	//�E�����Ƀ��C���΂��B����͕ǂɂ������p�B
	arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::RIGHT)] |= CastRay(arg_newPos, arg_from, rightDir, m_refPlayer->WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

	//�������Ƀ��C���΂��B����͕ǂɂ������p�B
	arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::LEFT)] |= CastRay(arg_newPos, arg_from, -rightDir, m_refPlayer->WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

	//�������Ƀ��C���΂��B����͕ǂɂ������p�B
	arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::BEHIND)] |= CastRay(arg_newPos, arg_from, -frontDir, m_refPlayer->WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

	//���ʕ����Ƀ��C���΂��B����͕ǂɂ������p�B
	arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::FRONT)] |= CastRay(arg_newPos, arg_from, frontDir, m_refPlayer->WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

}

template<typename Func>
inline void PlayerCollision::CheckHitAllObject(Func arg_func, KuroEngine::Vec3<float>& arg_newPos, const KuroEngine::Vec3<float>& arg_from, PlayerCollision::CastRayArgument& arg_castRayArgment, std::weak_ptr<Stage> arg_nowStage)
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

			//�����蔻������s
			(this->*arg_func)(arg_newPos, arg_from, arg_castRayArgment, arg_nowStage);

			//=================================================
		}
	}

	//��������Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::MOVE_SCAFFOLD)continue;

		//�����ɂ���ăJ�����O
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 100.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

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

			//�����蔻������s
			(this->*arg_func)(arg_newPos, arg_from, arg_castRayArgment, arg_nowStage);

			//=================================================
		}
	}

	//�Ӄu���b�N�Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::IVY_BLOCK)continue;

		//�����ɂ���ăJ�����O
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 5.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

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

			//�����蔻������s
			(this->*arg_func)(arg_newPos, arg_from, arg_castRayArgment, arg_nowStage);

			//=================================================
		}
	}

	//�����Ȃ��ǂƂ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::APPEARANCE)continue;

		//��������Ƃ��ăL���X�g
		auto ivyBlock = dynamic_pointer_cast<Appearance>(terrian);

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

			//�����蔻������s
			(this->*arg_func)(arg_newPos, arg_from, arg_castRayArgment, arg_nowStage);

			//=================================================
		}
	}

	//�v���C���[�������Ă����疳��
	if (m_refPlayer->GetIsUnderGround()) return;

	//�t�F���X�Ƃ̓����蔻��
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//��������łȂ�
		if (terrian->GetType() != StageParts::SPLATOON_FENCE)continue;

		//�����ɂ���ăJ�����O
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 2.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

		//��������Ƃ��ăL���X�g
		auto ivyBlock = dynamic_pointer_cast<SplatoonFence>(terrian);

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

			//�����蔻������s
			(this->*arg_func)(arg_newPos, arg_from, arg_castRayArgment, arg_nowStage);

			//=================================================
		}
	}

}