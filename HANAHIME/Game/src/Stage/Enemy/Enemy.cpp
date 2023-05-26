#include "Enemy.h"
#include"../../Player/Player.h"
#include"FrameWork/UsersInput.h"
#include"../../Graphics/BasicDraw.h"
#include"../../OperationConfig.h"

int MiniBug::ENEMY_MAX_ID = 0;


#pragma region MiniBug
void MiniBug::OnInit()
{
	m_nowStatus = SEARCH;
	m_prevStatus = SEARCH;
	m_limitIndex = 0;
	m_deadFlag = false;
	m_startDeadMotionFlag = false;
	m_deadTimer.Reset(120);
	m_scale = 1.0f;

	m_hitBox.m_centerPos = &m_pos;
	m_hitBox.m_radius = &m_scale;

	m_shadowInfluenceRange = SHADOW_INFLUENCE_RANGE;

	m_patrol->Init(m_limitIndex);
	m_pos = m_patrol->GetLimitPos(m_limitIndex);

	m_dashEffect.Finalize();
	m_finalizeFlag = false;


	m_sightArea.Init(&m_transform);
	track.Init(0.5f);

	m_nowStatus = SEARCH;
	m_prevStatus = NONE;
	m_limitIndex = 0;
	m_deadFlag = false;
	m_startDeadMotionFlag = false;
	m_deadTimer.Reset(120);

	m_hitBoxSize = m_transform.GetScale().x;
	m_hitBox.m_centerPos = &m_transform.GetPos();
	m_hitBox.m_radius = &m_hitBoxSize;

	m_shadowInfluenceRange = SHADOW_INFLUENCE_RANGE;
}

void MiniBug::Update(Player &arg_player)
{

	m_dashEffect.Update(m_larpPos, m_nowStatus == MiniBug::ATTACK && m_jumpMotion.IsDone());
	m_eyeEffect.Update(m_larpPos);

	//���ʏ���
	if (m_deadFlag)
	{
		m_reaction->Update(m_pos);

		if (!m_finalizeFlag)
		{
			m_dashEffect.Finalize();
			m_finalizeFlag = true;
		}
		return;
	}
	else
	{
	}


	//���S��������
	if (m_startDeadMotionFlag && !m_deadFlag)
	{
		//����ł�����ۉe������������B
		m_shadowInfluenceRange = KuroEngine::Math::Lerp(m_shadowInfluenceRange, 0.0f, 0.01f);

		if (m_deadTimer.UpdateTimer() && m_deadTimer.GetElaspedTime() != 0.0f)
		{
			m_deadTimer.Reset(120);
			m_deadFlag = true;
		}

		//���S���̍��W
		KuroEngine::Vec3<float>vel = m_initializedTransform.GetUp();
		m_pos += vel * 0.1f;
		m_transform.SetPos(m_pos);

		//���S���̃X�P�[��
		m_scale = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Back, m_deadTimer.GetTimeRate(), m_initializedTransform.GetScale().x, 0.0f);
		m_transform.SetScale(m_scale);

		//���S���̉�]
		DirectX::XMVECTOR vec = { 0.0f,0.0f,1.0f,1.0f };
		m_larpRotation = DirectX::XMQuaternionRotationAxis(vec, KuroEngine::Angle::ConvertToRadian(360.0f));
		KuroEngine::Quaternion rotation = m_transform.GetRotate();
		rotation = DirectX::XMQuaternionSlerp(m_transform.GetRotate(), m_larpRotation, 0.1f);
		m_transform.SetRotate(rotation);

		m_reaction->Update(m_pos);

		return;
	}

	//�����Ă�����ۉe�����ɖ߂��B
	m_shadowInfluenceRange = KuroEngine::Math::Lerp(m_shadowInfluenceRange, SHADOW_INFLUENCE_RANGE, 0.1f);

	//�G������(�v���C���[�����E�ɓ�����)
	if (OperationConfig::Instance()->DebugKeyInputOnTrigger(DIK_1))
	{
		OnInit();
	}

	bool findFlag = m_sightArea.IsFind(arg_player.GetTransform().GetPos(), 180.0f);
	//�v���C���[���Ⴄ�@���̖ʂɂ����猩�Ȃ��悤�ɂ���B
	bool isDifferentWall = m_transform.GetUp().Dot(arg_player.GetTransform().GetUpWorld()) <= 0.5f;
	bool isPlayerWallChange = arg_player.GetIsJump();
	bool isAttackOrNotice = m_nowStatus == MiniBug::ATTACK || m_nowStatus == MiniBug::NOTICE;
	if ((isDifferentWall || isPlayerWallChange) && isAttackOrNotice) {
		findFlag = false;
		m_nowStatus = MiniBug::NOTICE;
	}
	const bool isMoveFlag = 0.1f < KuroEngine::Vec3<float>(arg_player.GetNowPos() - arg_player.GetOldPos()).Length();
	if (findFlag && arg_player.GetIsUnderGround() && m_nowStatus != MiniBug::RETURN && isMoveFlag)
	{
		m_nowStatus = MiniBug::NOTICE;
	}
	else if (findFlag && !arg_player.GetIsUnderGround() && !isDifferentWall)
	{
		m_nowStatus = MiniBug::ATTACK;
	}

	//������---------------------------------------------------
	if (m_nowStatus != m_prevStatus)
	{
		int index = 0;
		float min = std::numeric_limits<float>().max();
		float prevMin = 0.0f;
		switch (m_nowStatus)
		{
			//�ł��߂�����n�_���烋�[�v����
		case MiniBug::SEARCH:

			//�v���C���[���Ⴄ�@���̖ʂɂ����猩�Ȃ��悤�ɂ���B
			if (m_transform.GetUp().Dot(arg_player.GetTransform().GetUpWorld()) <= 0.5f) break;

			m_patrol->Init(m_limitIndex);
			m_pos = m_patrol->GetLimitPos(m_limitIndex);

			break;
		case MiniBug::ATTACK:
			m_attackIntervalTimer.Reset(120);
			m_readyToGoToPlayerTimer.Reset(120);
			m_sightArea.Init(&m_transform);
			track.Init(0.5f);

			m_jumpMotion.Init(m_pos, m_pos + KuroEngine::Vec3<float>(0.0f, 5.0f, 0.0f), 0.5f);

			//Done�t���O��false�ɂ��āA���o���I����ĂȂ���Ԃɂ���B
			m_jumpMotion.UnDone();

			break;
		case MiniBug::NOTICE:
			m_reaction->Init(LOOK, m_transform.GetUp());
			break;
		case MiniBug::RETURN:
			//�ł��߂�����_�ɖ߂�
			m_aPointPos = m_pos;
			for (int i = 0; i < m_posArray.size(); ++i)
			{
				min = std::min(m_posArray[i].Distance(m_pos), min);
				if (min != prevMin)
				{
					index = i;
					prevMin = min;
				}
			}
			m_bPointPos = m_posArray[index];

			m_trackPlayer.Init(m_pos, m_bPointPos, 0.1f);

			m_limitIndex = index;
			break;
		default:
			break;
		}
		m_thinkTimer.Reset(120);
		m_prevStatus = m_nowStatus;
	}
	//������---------------------------------------------------


	float distance = 0.0f;

	//�X�V����---------------------------------------
	KuroEngine::Vec3<float>vel = { 0.0f,0.0f,0.0f };
	switch (m_nowStatus)
	{
	case MiniBug::SEARCH:

		//�v���C���[���Ⴄ�@���̖ʂɂ����猩�Ȃ��悤�ɂ���B
		//if (m_transform.GetUp().Dot(arg_player.GetTransform().GetUpWorld()) <= 0.5f) break;

		vel = m_patrol->Update(m_pos);
		m_dir = vel;

		break;
	case MiniBug::ATTACK:

		//���������̃��A�N�V��������
		//if (!m_readyToGoToPlayerTimer.UpdateTimer()) ���ԂŐ؂�ւ���
		if (arg_player.GetIsUnderGround())
		{
			m_nowStatus = MiniBug::NOTICE;
		}

		if (!m_jumpMotion.IsDone())	//���[�V�����Ő؂�ւ���
		{
			//����
			//��������̃A�N�V����������
			vel = m_jumpMotion.GetVel(m_pos);
			m_dir = KuroEngine::Vec3<float>(arg_player.GetTransform().GetPos() - m_pos).GetNormal();
			m_dir.y = 0.0f;
			m_reaction->Init(FIND, m_transform.GetUp());
			break;
		}
		else
		{
			//�I����Ă��Ԃɂ���B
			m_jumpMotion.Done();
		}


		distance = arg_player.GetTransform().GetPos().Distance(m_pos);


		//�v���C���[�ƈ�苗���܂ŋ߂Â�����U���\�����������
		if (m_attackCoolTimer.UpdateTimer() && distance <= 5.0f && !m_attackFlag)
		{
			m_attackFlag = true;
			m_attackMotion.Init(m_pos, m_pos + KuroEngine::Vec3<float>(0.0f, 2.0f, 0.0f), 0.5f);
		}

		//�U���\�����삪�I����čU�����s�����B
		if (m_attackFlag && m_attackIntervalTimer.UpdateTimer())
		{
			//�v���C���[�ƓG�̓����蔻��̏����������ɏ���
			m_attackIntervalTimer.Reset(120);
			m_attackCoolTimer.Reset(120);
			m_attackFlag = false;
		}

		//�v���C���[�ƈ��ȏ㋗�������ꂽ�ꍇ
		if (30.0f <= distance)
		{
			//�b���~�܂�A�����Ȃ���Ύv�l��؂�ւ���B
			if (m_thinkTimer.UpdateTimer())
			{
				m_nowStatus = MiniBug::RETURN;
			}
			m_aPointPos = m_pos;
		}
		//�v���C���[��ǔ���
		else if (!m_attackFlag)
		{
			m_thinkTimer.Reset(120);
			vel = track.Update(m_pos, arg_player.GetTransform().GetPos());
			m_dir = track.Update(m_pos, arg_player.GetTransform().GetPos()).GetNormal();
		}

		break;
	case MiniBug::NOTICE:
		//�b���҂��ē����Ȃ�������ʂ̏ꏊ�Ɍ�����
		if (m_thinkTimer.UpdateTimer())
		{
			m_nowStatus = MiniBug::RETURN;
		}
		//�������璍������
		if (isMoveFlag)
		{
			m_dir = KuroEngine::Vec3<float>(arg_player.GetTransform().GetPos() - m_pos).GetNormal();
			m_thinkTimer.Reset(120);
			m_reaction->Init(LOOK, m_transform.GetUp());
		}
		break;
	case MiniBug::RETURN:
		//���Ԓ�
		m_thinkTimer.Reset(120);
		vel = m_trackPlayer.Update();
		m_dir = vel.GetNormal();
		if (m_trackPlayer.IsArrive(arg_player.GetTransform().GetPos()))
		{
			m_nowStatus = MiniBug::SEARCH;
		}
		break;
	default:
		break;
	}
	//�X�V����---------------------------------------


	//�v���C���[�ƓG�̔���
	if (Collision::Instance()->CheckCircleAndCircle(arg_player.m_sphere, m_hitBox))
	{
		arg_player.Damage();
	}

	//���̓����蔻��
	if (arg_player.CheckHitGrassSphere(m_transform.GetPosWorld(), m_transform.GetUpWorld(), m_transform.GetScale().Length()) != Player::CHECK_HIT_GRASS_STATUS::NOHIT && !m_startDeadMotionFlag)
	{
		m_startDeadMotionFlag = true;
	}

	m_reaction->Update(m_pos);

	//���W�ړ�
	m_pos += vel;
	m_prevPos = m_pos;

	KuroEngine::Vec3<float>frontVec = m_transform.GetFront();

	//�ړ������Ɛ��ʃx�N�g����G��̎p���ɓ��e�B
	KuroEngine::Vec2<float> frontVec2D = Project3Dto2D(frontVec, m_transform.GetFront(), m_transform.GetRight());
	KuroEngine::Vec2<float> moveDir2D = Project3Dto2D(m_dir, m_transform.GetFront(), m_transform.GetRight());

	float rptaVel = acosf(frontVec2D.Dot(moveDir2D));
	rptaVel *= (0 < frontVec2D.Cross(moveDir2D)) ? 1.0f : -1.0f;

	//�v���C���[���Ⴄ�ʂɂ邩�A�W�����v�ŕǖʈړ����̓v���C���[�̕������Ȃ��B
	if ((isDifferentWall || isPlayerWallChange) && isAttackOrNotice) {

	}
	else
	{



		//���݂̍��W����v���C���[�Ɍ�������]�����߂�B
		KuroEngine::Vec3<float> axisZ = m_dir;
		axisZ.Normalize();

		//�v���C���[�̖@���Ƃ̊O�ς��牼��X�x�N�g���𓾂�B
		KuroEngine::Vec3<float> axisX = m_initializedTransform.GetUp().Cross(axisZ);

		//X�x�N�g�������x�N�g���𓾂�B
		KuroEngine::Vec3<float> axisY = axisZ.Cross(axisX);

		//�p���𓾂�B
		DirectX::XMMATRIX matWorld = DirectX::XMMatrixIdentity();
		matWorld.r[0] = { axisX.x, axisX.y, axisX.z, 0.0f };
		matWorld.r[1] = { axisY.x, axisY.y, axisY.z, 0.0f };
		matWorld.r[2] = { axisZ.x, axisZ.y, axisZ.z, 0.0f };

		XMVECTOR rotate, scale, position;
		DirectX::XMMatrixDecompose(&scale, &rotate, &position, matWorld);

		m_transform.SetRotate(DirectX::XMQuaternionSlerp(m_transform.GetRotate(), rotate, 0.08f));
	}

	m_larpPos = KuroEngine::Math::Lerp(m_larpPos, m_pos, 0.1f);

	m_transform.SetPos(m_larpPos);

}

void MiniBug::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	if (m_deadFlag)
	{
		return;
	}

	IndividualDrawParameter edgeColor = IndividualDrawParameter::GetDefault();
	edgeColor.m_edgeColor = KuroEngine::Color(0.0f, 0.0f, 0.0f, 1.0f);

	//BasicDraw::Instance()->Draw_Player(
	//	arg_cam,
	//	arg_ligMgr,
	//	m_model,
	//	m_transform,
	//	edgeColor);

	m_reaction->Draw(arg_cam);

	m_dashEffect.Draw(arg_cam);
	m_eyeEffect.Draw(arg_cam);
	//DebugDraw(arg_cam);

}

void MiniBug::DebugDraw(KuroEngine::Camera &camera)
{
#ifdef _DEBUG

	//return;

	switch (m_nowStatus)
	{
	case MiniBug::SEARCH:
		m_patrol->DebugDraw();
		break;
	case MiniBug::ATTACK:
		break;
	case MiniBug::NOTICE:
		break;
	case MiniBug::RETURN:
		break;
	default:
		break;
	}

	m_sightArea.DebugDraw(camera);

#endif // _DEBUG

}
#pragma endregion


#pragma region Dossun
void DossunRing::OnInit()
{
	m_attackHitBoxRadius = 0.0f;
	m_findPlayerFlag = false;
	m_preFindPlayerFlag = false;

	m_sightRange = m_attackhitBoxRadiusMax;

	//���E�̔���---------------------------------------
	m_sightHitBox.m_centerPos = &m_transform.GetPos();
	m_sightHitBox.m_radius = &m_sightRange;
	m_sightArea.Init(m_sightHitBox);
	//���E�̔���---------------------------------------

	m_maxAttackIntervalTime = 60 * 2;
	m_maxAttackTime = 60 * 5;

	m_attackInterval.Reset(m_maxAttackIntervalTime);
	m_attackTimer.Reset(m_maxAttackTime);

	//���S����---------------------------------------
	m_deadFlag = false;
	m_startDeadMotionFlag = false;
	m_deadTimer.Reset(120);
	//���S����---------------------------------------

	m_hitBox.m_centerPos = &m_transform.GetPos();
	m_hitBox.m_radius = &m_attackHitBoxRadius;
}

void DossunRing::Update(Player &arg_player)
{
	if (m_deadFlag && IsActive(m_transform, arg_player.GetTransform()))
	{
		return;
	}
	else
	{
		m_findPlayerFlag = false;
	}
	//���S��������
	if (m_startDeadMotionFlag && !m_deadFlag)
	{
		m_scale = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Back, m_deadTimer.GetTimeRate(), 1.0f, 0.0f);
		m_transform.SetScale(m_scale);

		//����ł�����ۉe������������B
		m_shadowInfluenceRange = KuroEngine::Math::Lerp(m_shadowInfluenceRange, 0.0f, 0.01f);

		if (m_deadTimer.UpdateTimer() && m_deadTimer.GetElaspedTime() != 0.0f)
		{
			m_deadTimer.Reset(120);
			m_deadFlag = true;
		}

		DirectX::XMVECTOR vec = { 0.0f,0.0f,1.0f,1.0f };
		m_larpRotation = DirectX::XMQuaternionRotationAxis(vec, KuroEngine::Angle::ConvertToRadian(90.0f));
		KuroEngine::Quaternion rotation = m_transform.GetRotate();
		rotation = DirectX::XMQuaternionSlerp(m_transform.GetRotate(), m_larpRotation, 0.1f);
		m_transform.SetRotate(rotation);

		return;
	}


	//�v���C���[�ƓG�̓����蔻��̏����������ɏ���
	if (arg_player.CheckHitGrassSphere(m_transform.GetPosWorld(), m_transform.GetUpWorld(), m_transform.GetScale().Length()) != Player::CHECK_HIT_GRASS_STATUS::NOHIT)
	{
		m_startDeadMotionFlag = true;
		return;
	}

	//�����Ă�����ۉe�����ɖ߂��B
	m_shadowInfluenceRange = KuroEngine::Math::Lerp(m_shadowInfluenceRange, SHADOW_INFLUENCE_RANGE, 0.1f);


	if (m_sightArea.IsFind(arg_player.m_sphere) && !arg_player.GetIsUnderGround())
	{
		m_findPlayerFlag = true;
	}
	//�U�����I����ăv���C���[�������Ȃ��Ȃ�����U���I��
	else if (!m_attackFlag)
	{
		m_findPlayerFlag = false;
	}

	//����̏ꍇ�͏�ɍU�����郂�[�V����������
	switch (m_nowStatus)
	{
	case ENEMY_ATTACK_PATTERN_ALWAYS:
		m_findPlayerFlag = true;
		break;
	default:
		break;
	}

	if (!m_findPlayerFlag && !m_attackFlag)
	{
		return;
	}
	//�ȍ~�v���C���[���������ꂽ����---------------------------------------

	//�������A�N�V����
	if (m_findPlayerFlag && !m_preFindPlayerFlag)
	{
		m_reaction->Init(FIND, m_transform.GetUp());
	}
	m_preFindPlayerFlag = m_findPlayerFlag;

	//�U���\�����쒆
	if (m_attackInterval.UpdateTimer() && !m_attackFlag)
	{
		m_attackTimer.Reset(m_maxAttackTime);
		m_attackFlag = true;
	}

	//�U���J�n
	if (m_attackFlag)
	{
		//�����蔻��̍L����
		m_attackHitBoxRadius = m_attackTimer.GetTimeRate() * m_attackhitBoxRadiusMax;
		//�L����؂�����C���^�[�o���ɖ߂�
		if (m_attackTimer.UpdateTimer())
		{
			m_attackHitBoxRadius = 0.0f;
			m_attackInterval.Reset(m_maxAttackIntervalTime);
			m_attackFlag = false;
			m_findPlayerFlag = false;
		}

		KuroEngine::Vec3<float>playerPos(arg_player.GetTransform().GetPos());
		KuroEngine::Vec3<float>playerUpVec(arg_player.GetTransform().GetUp());
		KuroEngine::Vec3<float>enemyPlayerVec(arg_player.GetTransform().GetPos() - *(m_hitBox.m_centerPos));
		enemyPlayerVec.Normalize();

		if (Collision::Instance()->CheckPointAndEdgeOfCircle(
			m_hitBox,
			playerPos,
			playerUpVec,
			enemyPlayerVec) &&
			!arg_player.GetIsUnderGround())
		{
			arg_player.Damage();
		}
	}

	m_reaction->Update(m_transform.GetPos());
}

void DossunRing::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	if (m_deadFlag)
	{
		return;
	}

	IndividualDrawParameter edgeColor = IndividualDrawParameter::GetDefault();
	edgeColor.m_edgeColor = KuroEngine::Color(0.0f, 0.0f, 0.0f, 1.0f);

	//BasicDraw::Instance()->Draw_Player(
	//	arg_cam,
	//	arg_ligMgr,
	//	m_model,
	//	m_transform,
	//	edgeColor);

	KuroEngine::Transform transform = m_transform;
	transform.SetPos(*(m_hitBox.m_centerPos));
	float scale = *(m_hitBox.m_radius);
	transform.SetScale(KuroEngine::Vec3<float>(scale, 1.0f, scale));
	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_attackRingModel,
		transform,
		edgeColor);

	m_reaction->Draw(arg_cam);

	//DebugDraw(arg_cam);
}

void DossunRing::DebugDraw(KuroEngine::Camera &camera)
{
#ifdef _DEBUG

	//return;

	if (m_attackFlag)
	{
		/*KuroEngine::Transform transform;
		transform.SetPos(*m_hitBox.m_centerPos);
		transform.SetScale(*m_hitBox.m_radius);
		KuroEngine::DrawFunc3D::DrawNonShadingModel(
			m_hitBoxModel,
			transform.GetMatWorld(),
			camera,
			0.5f
		);*/
	}
	else
	{
		m_sightArea.DebugDraw(camera);
	}

#endif // _DEBUG

}
#pragma endregion


#pragma region Battery
Battery::Battery(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, std::vector<KuroEngine::Vec3<float>> arg_posArray, float arg_bulletScale, ENEMY_BARREL_PATTERN arg_barrelPattern)
	:StageParts(BATTERY, arg_model, arg_initTransform), m_posArray(arg_posArray), m_barrelPattern(arg_barrelPattern)
{
	//���W�z�񂪋�Ȃ炻�̏�ɂƂǂ܂�
	if (m_posArray.empty())
	{
		m_posArray.emplace_back(arg_initTransform.GetPosWorld());
		m_pos = m_transform.GetPos();
	}
	else if (2 <= m_posArray.size())
	{
		m_patrol = std::make_unique<PatrolBasedOnControlPoint>(m_posArray, 0, true);
		m_patrol->Init(0);
		m_pos = m_posArray[0];
	}
	m_transform = arg_initTransform;
	m_initTransform = arg_initTransform;
	m_transform.SetPos(m_pos);

	m_upVec = arg_initTransform.GetUp();

	OnInit();
}

void Battery::OnInit()
{
	m_bulletDir = m_transform.GetFront();
	m_bulletManager.Init(&m_pos, 5.0f, &m_bulletDir, 120.0f);

	m_radius = m_transform.GetScale().x;
	m_hitBox.m_centerPos = &m_pos;
	m_hitBox.m_radius = &m_radius;

	m_startDeadMotionFlag = false;
	m_deadFlag = false;
	m_noticeFlag = false;
}

void Battery::Update(Player &arg_player)
{
	if (m_deadFlag)
	{
		return;
	}
	if (m_startDeadMotionFlag)
	{
		m_deadFlag = true;
	}

	KuroEngine::Vec3<float>vel = {};
	//����_����ȏ゠��ꍇ�͌��݂ɓ���
	if (m_patrol)
	{
		vel = m_patrol->Update(m_transform.GetPos());
	}

	KuroEngine::Vec3<float>dir(arg_player.GetTransform().GetPos() - m_transform.GetPos());
	float distance = arg_player.GetTransform().GetPos().Distance(m_transform.GetPos());

	bool isDiffrentWallFlag = m_transform.GetUp().Dot(arg_player.GetTransform().GetUpWorld()) <= 0.5f;

	//�˒��͈͓��A�n�ʂɐ����ĂȂ��A�����ʂɂ���
	m_bulletManager.Update(120.0f, arg_player.m_sphere,
		distance <= 50.0f &&
		!arg_player.GetIsUnderGround() &&
		!isDiffrentWallFlag
	);

	//�G�ƒe�̔���
	if (m_bulletManager.IsHit())
	{
		arg_player.Damage();
	}
	//�G�ƃv���C���[�̔���
	if (Collision::Instance()->CheckCircleAndCircle(arg_player.m_sphere, m_hitBox) && !arg_player.GetIsUnderGround())
	{
		arg_player.Damage();
	}
	//���̓����蔻��
	if (arg_player.CheckHitGrassSphere(m_transform.GetPosWorld(), m_transform.GetUpWorld(), m_transform.GetScale().Length()) != Player::CHECK_HIT_GRASS_STATUS::NOHIT && !m_startDeadMotionFlag)
	{
		m_startDeadMotionFlag = true;
	}

	switch (m_barrelPattern)
	{
	case ENEMY_BARREL_PATTERN_FIXED:
		//�����Œ�
		break;
	case ENEMY_BARREL_PATTERN_ROCKON:
		//�n�ʂɋ��Ȃ����Ƀv���C���[�̕���������------
		if (!arg_player.GetIsUnderGround() && distance <= 50.0f)
		{
			if (!m_noticeFlag)
			{
				m_reaction->Init(FIND, m_transform.GetUp());
				m_noticeFlag = true;
			}

			//�G�̕�������������
			dir.Normalize();
			KuroEngine::Vec3<float>frontVec(m_transform.GetFront());
			KuroEngine::Vec3<float>axis = frontVec.Cross(dir);
			float rptaVel = acosf(frontVec.Dot(dir));

			DirectX::XMVECTOR dirVec = { axis.x,axis.y,axis.z,1.0f };
			m_rotation = DirectX::XMQuaternionRotationAxis(dirVec, rptaVel);

			m_bulletDir = dir;
		}
		//�n�ʂɋ��Ȃ����Ƀv���C���[�̕���������------
		//������Ȃ����͕ʕ���������------
		else
		{
			m_noticeFlag = false;
			m_larpRotation = {};
		}
		m_larpRotation = DirectX::XMQuaternionSlerp(m_transform.GetRotate(), m_rotation, 0.1f);

		break;
	case ENEMY_BARREL_PATTERN_INVALID:
		break;
	default:
		break;
	}

	m_pos += vel;
	m_transform.SetPos(m_pos);

	m_reaction->Update(m_pos);
	//m_transform.SetRotate(m_larpRotation);

}

void Battery::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	if (m_deadFlag)
	{
		return;
	}

	IndividualDrawParameter edgeColor = IndividualDrawParameter::GetDefault();
	edgeColor.m_edgeColor = KuroEngine::Color(0.0f, 0.0f, 0.0f, 1.0f);

	//BasicDraw::Instance()->Draw_Player(
	//	arg_cam,
	//	arg_ligMgr,
	//	m_model,
	//	m_transform,
	//	edgeColor);

	m_bulletManager.Draw(arg_cam);

	m_reaction->Draw(arg_cam);
	//DebugDraw(arg_cam);
}
#pragma endregion