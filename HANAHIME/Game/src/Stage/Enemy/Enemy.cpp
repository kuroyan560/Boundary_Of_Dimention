#include "Enemy.h"
#include"../../Player/Player.h"
#include"FrameWork/UsersInput.h"

void MiniBug::Update(Player &arg_player)
{
	if (m_decisionFlag != m_prevDecisionFlag)
	{
		//�v�l
		//�G������(�v���C���[�����E�ɓ�����)
		if (m_sightArea.IsFind(arg_player.GetTransform().GetPos(), &arg_player.GetTransform()))
		{
			m_nowStatus = MiniBug::ATTACK;
		}
		//�G������(�v���C���[���������Ă��邩�����Ă��鎞�ɔ�������)
		else if (false)
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
			m_patrol.Init(0, false);
			break;
		case MiniBug::ATTACK:
			m_trackTimer.Reset(260);
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
			m_trackTimer.Reset(120);
			break;
		default:
			break;
		}
		m_thinkTimer.Reset(120);
		m_prevStatus = m_nowStatus;
	}


	//�s��
	KuroEngine::Vec3<float>vel;
	switch (m_nowStatus)
	{
	case MiniBug::SERACH:
		vel = m_patrol.Update();
		break;
	case MiniBug::ATTACK:

		//���������̃��A�N�V��������
		if (false)
		{
			break;
		}

		//�v���C���[�ƈ��ȏ㋗�������ꂽ�ꍇ
		if (20.0f <= arg_player.GetTransform().GetPos().Distance(vel))
		{
			//�b���~�܂�A�����Ȃ���Ύv�l��؂�ւ���B
			if (m_thinkTimer.UpdateTimer())
			{
				m_decisionFlag = true;
			}
			m_aPointPos = m_pos;
		}
		//�v���C���[��ǔ���
		else
		{
			m_thinkTimer.Reset(120);
			vel = m_trackPlayer.Update(m_aPointPos, arg_player.GetTransform().GetPos(), m_trackTimer.GetTimeRate());
		}

		//�v���C���[�Ɠ��������ꍇ
		if (m_trackTimer.UpdateTimer())
		{

		}
		break;
	case MiniBug::NOTICE:
		break;
	case MiniBug::RETURN:
		//���Ԓ�
		m_thinkTimer.Reset(120);
		vel = m_trackPlayer.Update(m_aPointPos, m_bPointPos, m_trackTimer.GetTimeRate());
		if (m_trackTimer.UpdateTimer())
		{
			m_decisionFlag = true;
		}
		break;
	default:
		break;
	}

	//���ʏ���


	//���W�ړ�
	m_pos = vel;
	m_transform.SetPos(m_pos);
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
		m_sightArea.DebugDraw(camera);
		break;
	case MiniBug::NOTICE:
		break;
	case MiniBug::RETURN:
		break;
	default:
		break;
	}


#endif // _DEBUG

}

void DossunRing::Update(Player &arg_player)
{
	if (m_sightArea.IsFind(arg_player.m_sphere))
	{
		m_findPlayerFlag = true;
	}
	//�U�����I����ăv���C���[�������Ȃ��Ȃ�����U���I��
	else if(!m_attackFlag)
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
	}
}

void DossunRing::DebugDraw(KuroEngine::Camera &camera)
{
#ifdef _DEBUG

	m_sightArea.DebugDraw(camera);

#endif // _DEBUG

}