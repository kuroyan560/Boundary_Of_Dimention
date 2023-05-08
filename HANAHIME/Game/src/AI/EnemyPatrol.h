#pragma once
#include"KuroEngine.h"
#include"ForUser/Timer.h"
#include"../TimeScaleMgr.h"

//A地点からB地点に向かう
class HeadNextPoint
{
public:
	HeadNextPoint();
	void Init(const KuroEngine::Vec3<float> &aPos, const KuroEngine::Vec3<float> &bPos, float speed);
	KuroEngine::Vec3<float> Update();

	bool IsArrive(const KuroEngine::Vec3<float> &pos);

private:
	KuroEngine::Vec3<float>m_vel;
	KuroEngine::Vec3<float>m_endPos;
	float m_speed;

	KuroEngine::Timer m_arriveTimer;
};

class TrackEndPoint
{
public:
	TrackEndPoint();
	void Init(float speed);
	KuroEngine::Vec3<float> Update(const KuroEngine::Vec3<float> &aPos, const KuroEngine::Vec3<float> &bPos);

private:
	float m_speed;

};


//指定した地点を元に巡回する
class PatrolBasedOnControlPoint
{
public:
	PatrolBasedOnControlPoint(std::vector<KuroEngine::Vec3<float>>posArray, int initLimitIndex, bool loopFlag);

	void Init(int initLimitIndex);
	KuroEngine::Vec3<float> Update(const KuroEngine::Vec3<float> &pos);

	KuroEngine::Vec3<float>GetLimitPos(int index)
	{
		return m_limitPosArray[index].m_startPos;
	};

	void DebugDraw();

private:

	struct EnemyMoveData
	{
		KuroEngine::Vec3<float>m_startPos;
		KuroEngine::Vec3<float>m_endPos;
		HeadNextPoint m_moveToPoint;
		float m_speed;

		void Init()
		{
			m_moveToPoint.Init(m_startPos, m_endPos, m_speed);
		};
	};
	std::vector<EnemyMoveData>m_limitPosArray;

	KuroEngine::Timer m_moveTimer;
	int m_limitIndex;
	bool m_loopFlag;
	bool m_inverseFlag;

	float m_speed;

};
