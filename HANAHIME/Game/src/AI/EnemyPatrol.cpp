#include"EnemyPatrol.h"

PatrolBasedOnControlPoint::PatrolBasedOnControlPoint(std::vector<KuroEngine::Vec3<float>> posArray, int initLimitIndex) :
	m_moveTimer(60),m_limitIndex(initLimitIndex)
{
	for (int i = 0; i < posArray.size() / 2; ++i)
	{
		m_limitPosArray.emplace_back();
		m_limitPosArray[i].m_startPos = posArray[i];
		m_limitPosArray[i].m_endPos = posArray[i + 1];
		m_limitPosArray[i].timer = 0.0f;
	}
	m_inverseFlag = false;
	m_loopFlag = false;
}

void PatrolBasedOnControlPoint::Init(int initLimitIndex, bool loopFlag)
{
	m_limitIndex = initLimitIndex;
	m_loopFlag = loopFlag;
	m_moveTimer.Reset(60);
	m_inverseFlag = false;
}

KuroEngine::Vec3<float> PatrolBasedOnControlPoint::Update()
{
	KuroEngine::Vec3<float>startPos(m_limitPosArray[m_limitIndex].m_startPos);
	KuroEngine::Vec3<float>endPos(m_limitPosArray[m_limitIndex].m_endPos);

	KuroEngine::Vec3<float>pos;

	//最初から最後の制御点の進む
	if (!m_inverseFlag)
	{
		pos = m_limitPosArray[m_limitIndex].m_moveToPoint.Update(startPos, endPos, m_moveTimer.GetTimeRate());
	}
	//最後から最初の制御点の進む
	else
	{
		pos = m_limitPosArray[m_limitIndex].m_moveToPoint.Update(endPos, startPos, m_moveTimer.GetTimeRate());
	}

	bool timerUpFlag = m_moveTimer.UpdateTimer(TimeScaleMgr::s_inGame.GetTimeScale());
	if (timerUpFlag)
	{
		m_moveTimer.Reset(60);
	}

	//ループしない状態のインデックス増加減少
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
	//ループする状態のインデックス増加
	else if (timerUpFlag)
	{
		++m_limitIndex;
	}

	//ループしない場合、制御点で生成されたルートを往復する
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
	//ループする場合は、制御点の最後まで到達したら最初の地点に戻る
	else
	{
		if (m_limitPosArray.size() <= m_limitIndex)
		{
			m_limitIndex = 0;
		}
	}

	return pos;

}

void PatrolBasedOnControlPoint::DebugDraw()
{
}

HeadNextPoint::HeadNextPoint()
{
}

void HeadNextPoint::Init()
{
}

KuroEngine::Vec3<float> HeadNextPoint::Update(const KuroEngine::Vec3<float> &aPos, const KuroEngine::Vec3<float> &bPos, float timer)
{
	KuroEngine::Vec3<float>distance(bPos - aPos);
	return aPos + distance * timer;
}

bool HeadNextPoint::IsArrive()
{
	return false;
}
