#include "Enemy.h"
#include"../../Player/Player.h"
#include"FrameWork/UsersInput.h"

void MiniBug::Update(Player &arg_player)
{

	if (m_decisionFlag != m_prevDecisionFlag)
	{
		//�v�l
		//�G������(�v���C���[�����E�ɓ�����)

		//�G������(�v���C���[���������Ă��邩�����Ă��鎞�ɔ�������)
		if (false)
		{
			m_nowStatus = MiniBug::NOTICE;
		}
		//�A��(�G����������A��������)
		else if (false)
		{
			m_nowStatus = MiniBug::RETURN;
		}
		//����(�����N���Ă��Ȃ�or���[�g�ɋA�҂�����)
		else if (false)
		{
			m_nowStatus = MiniBug::SERACH;
		}
		m_prevDecisionFlag = m_decisionFlag;
	}

	//�G������(�v���C���[�����E�ɓ�����)
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_1))
	{
		m_nowStatus = MiniBug::ATTACK;
	}
	//�G������(�v���C���[���������Ă��邩�����Ă��鎞�ɔ�������)
	else if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_2))
	{
		m_nowStatus = MiniBug::NOTICE;
	}
	//�A��(�G����������A��������)
	else if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_3))
	{
		m_nowStatus = MiniBug::RETURN;
	}
	//����(�����N���Ă��Ȃ�or���[�g�ɋA�҂�����)
	else if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_4))
	{
		m_nowStatus = MiniBug::SERACH;
	}

	bool findFlag = m_sightArea.IsFind(arg_player.GetTransform().GetPos(), 180.0f);
	if (findFlag)
	{
		m_nowStatus = MiniBug::ATTACK;
	}

	//������
	if (m_nowStatus != m_prevStatus)
	{
		int index = 0;
		float min = 10000000000.0f;
		float prevMin = 0.0f;
		switch (m_nowStatus)
		{
			//�ł��߂�����n�_���烋�[�v����
		case MiniBug::SERACH:
			m_patrol.Init(m_limitIndex, false);
			m_pos = m_patrol.GetLimitPos(m_limitIndex);

			break;
		case MiniBug::ATTACK:
			m_attackIntervalTimer.Reset(120);
			m_readyToGoToPlayerTimer.Reset(120);
			m_sightArea.Init(&m_transform);
			track.Init(0.1f);

			m_jumpMotion.Init(m_pos, m_pos + KuroEngine::Vec3<float>(0.0f, 5.0f, 0.0f), 0.5f);

			break;
		case MiniBug::NOTICE:

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

	bool debug = false;
	float distance = 0.0f;

	//�s��
	KuroEngine::Vec3<float>vel = { 0.0f,0.0f,0.0f };
	switch (m_nowStatus)
	{
	case MiniBug::SERACH:
		vel = m_patrol.Update(m_pos);
		m_dir = vel;
		break;
	case MiniBug::ATTACK:

		//���������̃��A�N�V��������
		//if (!m_readyToGoToPlayerTimer.UpdateTimer()) ���ԂŐ؂�ւ���
		if (!m_jumpMotion.IsDone())	//���[�V�����Ő؂�ւ���
		{
			//����
			//��������̃A�N�V����������
			vel = m_jumpMotion.GetVel(m_pos);
			m_dir = KuroEngine::Vec3<float>(arg_player.GetTransform().GetPos() - m_pos).GetNormal();
			m_dir.y = 0.0f;
			break;
		}

		distance = arg_player.GetTransform().GetPos().Distance(m_pos);


		//�v���C���[�ƈ�苗���܂ŋ߂Â�����U���\�����������
		if (distance <= 5.0f && m_attackCoolTimer.UpdateTimer() && !m_attackFlag)
		{
			m_attackFlag = true;
			m_attackMotion.Init(m_pos, m_pos + KuroEngine::Vec3<float>(0.0f, 2.0f, 0.0f), 0.5f);
		}
		if (m_attackFlag)
		{
			vel = m_attackMotion.GetVel(m_pos);
		}

		//�U���\�����삪�I����čU�����s�����B
		//if (m_attackFlag && m_attackIntervalTimer.UpdateTimer())
		if (m_attackFlag && m_attackMotion.IsDone())
		{
			//�v���C���[�ƓG�̓����蔻��̏����������ɏ���
			m_attackIntervalTimer.Reset(120);
			m_attackFlag = false;

			m_attackCoolTimer.Reset(60);
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
		//�������璍������
		//����a
		break;
	case MiniBug::RETURN:
		//���Ԓ�
		m_thinkTimer.Reset(120);
		vel = m_trackPlayer.Update();
		m_dir = m_trackPlayer.Update().GetNormal();
		if (m_trackPlayer.IsArrive(arg_player.GetTransform().GetPos()))
		{
			m_nowStatus = MiniBug::SERACH;
		}
		break;
	default:
		break;
	}

	//���ʏ���



	//���̓����蔻��
	arg_player.CheckHitGrassSphere(m_transform.GetPosWorld(), m_transform.GetUpWorld(), m_transform.GetScale().Length());

	//���W�ړ�
	m_pos += vel;
	m_prevPos = m_pos;

	KuroEngine::Vec3<float>frontVec(0.0f, 0.0f, 1.0f);

	KuroEngine::Vec3<float>axis = frontVec.Cross(m_dir);
	float rptaVel = acosf(frontVec.Dot(m_dir));

	if (axis.x == 0.0f && axis.y == 0.0f && axis.z == 0.0f)
	{
		m_larpRotation = DirectX::XMQuaternionIdentity();
	}
	else
	{
		DirectX::XMVECTOR dirVec = { axis.x,axis.y,axis.z,1.0f };
		m_larpRotation = DirectX::XMQuaternionRotationAxis(dirVec, rptaVel);
	}




	m_larpPos = KuroEngine::Math::Lerp(m_larpPos, m_pos, 0.1f);
	KuroEngine::Quaternion rotation = Lerp(m_transform.GetRotate(), m_larpRotation, 0.1f);

	m_transform.SetPos(m_larpPos);
	m_transform.SetRotate(rotation);
}

void MiniBug::DebugDraw(KuroEngine::Camera &camera)
{
#ifdef _DEBUG

	switch (m_nowStatus)
	{
	case MiniBug::SERACH:
		m_patrol.DebugDraw();
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

void DossunRing::Update(Player &arg_player)
{
	if (m_sightArea.IsFind(arg_player.m_sphere))
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
	case DossunRing::ALWAYS:
		m_findPlayerFlag = true;
		break;
	default:
		break;
	}

	if (!m_findPlayerFlag)
	{
		return;
	}
	//�ȍ~�v���C���[���������ꂽ����---------------------------------------

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
		m_hitBoxRadius = m_attackTimer.GetTimeRate() * m_hitBoxRadiusMax;
		//�L����؂�����C���^�[�o���ɖ߂�
		if (m_attackTimer.UpdateTimer())
		{
			m_hitBoxRadius = 0.0f;
			m_attackInterval.Reset(m_maxAttackIntervalTime);
			m_attackFlag = false;
		}

		//�v���C���[�ƓG�̓����蔻��̏����������ɏ���
		if (false)
		{

		}
	}
}

void DossunRing::DebugDraw(KuroEngine::Camera &camera)
{
#ifdef _DEBUG

	m_sightArea.DebugDraw(camera);

#endif // _DEBUG

}