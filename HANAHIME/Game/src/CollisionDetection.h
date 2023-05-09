#pragma once
#include"Common/Transform.h"

struct Sphere
{
	const KuroEngine::Vec3<float> *m_centerPos;
	const float *m_radius;
};

class Collision : public KuroEngine::DesignPattern::Singleton<Collision>
{
public:
	bool CheckCircleAndCircle(const Sphere &sphereA, const Sphere &sphereB)
	{
		float distance = sphereA.m_centerPos->Distance(*sphereB.m_centerPos);
		float sumRadist = *sphereA.m_radius + *sphereB.m_radius;
		return (distance <= sumRadist);
	}
};
