#include"EnemyPatrol.h"

PatrolBasedOnControlPoint::PatrolBasedOnControlPoint(std::vector<KuroEngine::Vec3<float>> posArray, int initLimitIndex, bool loopFlag) :
	m_moveTimer(60), m_limitIndex(initLimitIndex), m_loopFlag(loopFlag)
{
	m_speed = 0.1f;
	const int halfArraySize = static_cast<int>(posArray.size() - 1);

	m_inverseFlag = false;

	if (halfArraySize == 0)
	{
		int limitMaxNum = 36;
		int angle = 360 / limitMaxNum;
		for (int i = 0; i < limitMaxNum; ++i)
		{
			m_limitPosArray.emplace_back();

			float startRadian = KuroEngine::Angle::ConvertToRadian(static_cast<float>(angle * i));
			float endRadian = KuroEngine::Angle::ConvertToRadian(static_cast<float>(angle * (i + 1)));
			float radius = 15.0f;

			KuroEngine::Vec3<float>startPos(posArray[0].x + cosf(startRadian) * radius, posArray[0].y, posArray[0].z + sinf(startRadian) * radius);
			KuroEngine::Vec3<float>endPos(posArray[0].x + cosf(endRadian) * radius, posArray[0].y, posArray[0].z + sinf(endRadian) * radius);

			m_posLimitArray.emplace_back(startPos);

			m_limitPosArray[i].m_startPos = startPos;
			m_limitPosArray[i].m_endPos = endPos;
			m_limitPosArray[i].m_speed = m_speed;
			m_limitPosArray[i].m_moveToPoint.Init(startPos, endPos, m_speed);
		}
		m_loopFlag = true;
		return;
	}

	//�ŏ�����Ō�܂ł̃��[�g
	for (int i = 0; i < halfArraySize; ++i)
	{
		m_limitPosArray.emplace_back();
		m_limitPosArray[i].m_startPos = posArray[i];
		m_limitPosArray[i].m_endPos = posArray[i + 1];
		m_limitPosArray[i].m_speed = m_speed;
		m_limitPosArray[i].m_moveToPoint.Init(posArray[i], posArray[i + 1], m_speed);
	}
	if (!m_loopFlag)
	{
		//��̂Ƃ͋t���[�g
		for (int i = halfArraySize; 0 < i; --i)
		{
			m_limitPosArray.emplace_back();
			m_limitPosArray.back().m_startPos = posArray[i];
			m_limitPosArray.back().m_endPos = posArray[i - 1];
			m_limitPosArray.back().m_speed = m_speed;
			m_limitPosArray.back().m_moveToPoint.Init(posArray[i], posArray[i - 1], m_speed);
		}
	}
	else
	{
		m_limitPosArray.emplace_back();
		m_limitPosArray.back().m_startPos = m_limitPosArray[m_limitPosArray.size() - 2].m_endPos;
		m_limitPosArray.back().m_endPos = m_limitPosArray[0].m_startPos;
		m_limitPosArray.back().m_speed = m_speed;
		m_limitPosArray.back().m_moveToPoint.Init(m_limitPosArray[m_limitPosArray.size() - 2].m_endPos, m_limitPosArray[0].m_startPos, m_speed);
	}

}

void PatrolBasedOnControlPoint::Init(int initLimitIndex)
{
	m_limitIndex = initLimitIndex;
	m_moveTimer.Reset(60);
	m_inverseFlag = false;

	for (auto &obj : m_limitPosArray)
	{
		obj.Init();
	}

}

KuroEngine::Vec3<float> PatrolBasedOnControlPoint::Update(const KuroEngine::Vec3<float> &pos)
{
	KuroEngine::Vec3<float>vel;

	//�ŏ�����Ō�̐���_�̐i��
	if (!m_inverseFlag)
	{
		vel = m_limitPosArray[m_limitIndex].m_moveToPoint.Update();
	}
	//�Ōォ��ŏ��̐���_�̐i��
	else
	{
		vel = m_limitPosArray[m_limitIndex].m_moveToPoint.Update() * -1.0f;
	}

	bool timerUpFlag = m_limitPosArray[m_limitIndex].m_moveToPoint.IsArrive(pos);
	if (timerUpFlag)
	{
		m_moveTimer.Reset(60);
	}

	//���[�v���Ȃ���Ԃ̃C���f�b�N�X��������
	if (timerUpFlag && !m_loopFlag)
	{
		if (!m_inverseFlag)
		{
			++m_limitIndex;
		}
		else
		{
			--m_limitIndex;
		}
	}
	//���[�v�����Ԃ̃C���f�b�N�X����
	else if (timerUpFlag)
	{
		++m_limitIndex;
	}

	//���[�v���Ȃ��ꍇ�A����_�Ő������ꂽ���[�g����������
	if (!m_loopFlag)
	{
		if (m_limitIndex < 0)
		{
			m_inverseFlag = false;
			m_limitIndex = 0;
		}
		else if (m_limitPosArray.size() <= m_limitIndex)
		{
			m_inverseFlag = true;
			m_limitIndex = static_cast<int>(m_limitPosArray.size() - 1);
		}
	}
	//���[�v����ꍇ�́A����_�̍Ō�܂œ��B������ŏ��̒n�_�ɖ߂�
	else
	{
		if (m_limitPosArray.size() <= m_limitIndex)
		{
			m_limitIndex = 0;
		}
	}

	if (timerUpFlag)
	{
		KuroEngine::Vec3<float>startPos(m_limitPosArray[m_limitIndex].m_startPos);
		KuroEngine::Vec3<float>endPos(m_limitPosArray[m_limitIndex].m_endPos);

		m_limitPosArray[m_limitIndex].m_moveToPoint.Init(startPos, endPos, m_speed);
	}

	return vel;

}

void PatrolBasedOnControlPoint::DebugDraw()
{
}

HeadNextPoint::HeadNextPoint()
{
}

void HeadNextPoint::Init(const KuroEngine::Vec3<float> &aPos, const KuroEngine::Vec3<float> &bPos, float speed)
{
	m_speed = speed;
	m_endPos = bPos;
	KuroEngine::Vec3<float>distance(m_endPos - aPos);
	m_vel = distance.GetNormal() * m_speed;
	//�������莞��(����/�X�s�[�h = ���t���[�����ŒH�蒅���邩)
	m_arriveTimer.Reset(distance.Length() / m_speed);
}

KuroEngine::Vec3<float> HeadNextPoint::Update()
{
	m_arriveTimer.UpdateTimer();
	return m_vel;
}

bool HeadNextPoint::IsArrive(const KuroEngine::Vec3<float> &pos)
{
	if (m_arriveTimer.IsTimeUp())
	{
		return true;
	}
	return false;
}


TrackEndPoint::TrackEndPoint()
{
}

void TrackEndPoint::Init(float speed)
{
	m_speed = speed;
}

KuroEngine::Vec3<float> TrackEndPoint::Update(const KuroEngine::Vec3<float> &aPos, const KuroEngine::Vec3<float> &bPos)
{
	KuroEngine::Vec3<float>distance(bPos - aPos);
	distance.Normalize();
	return distance * m_speed;
}

