#pragma once
#include"KuroEngine.h"
#include"ForUser/Timer.h"
#include"../TimeScaleMgr.h"

//A�n�_����B�n�_�Ɍ�����
class HeadNextPoint
{
public:
	HeadNextPoint();
	void Init();
	KuroEngine::Vec3<float> Update(const KuroEngine::Vec3<float> &aPos, const KuroEngine::Vec3<float> &bPos, float timer);

	bool IsArrive();

private:
};


//�w�肵���n�_�����ɏ��񂷂�
class PatrolBasedOnControlPoint
{
public:
	PatrolBasedOnControlPoint(std::vector<KuroEngine::Vec3<float>>posArray, int initLimitIndex);

	void Init(int initLimitIndex, bool loopFlag);
	KuroEngine::Vec3<float> Update();
	void DebugDraw();

private:

	struct EnemyMoveData
	{
		KuroEngine::Vec3<float>m_startPos;
		KuroEngine::Vec3<float>m_endPos;
		HeadNextPoint m_moveToPoint;
		float timer;
	};
	std::vector<EnemyMoveData>m_limitPosArray;
	
	KuroEngine::Timer m_moveTimer;
	int m_limitIndex;
	bool m_loopFlag;
	bool m_inverseFlag;

};