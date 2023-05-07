#pragma once
#include"KuroEngine.h"
#include"Render/RenderObject/Camera.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"FrameWork/Importer.h"
#include"Common/Singleton.h"

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

//ê¸ï™Ç∆ì_ÇÃîªíË
class CollisionDetectionRayAndPoint
{
public:
	//ê¸ï™Ç∆ì_ÇÃäOêœ
	static float CaluCross(const KuroEngine::Vec3<float> &a, const KuroEngine::Vec3<float> &b, const KuroEngine::Vec3<float> &c)
	{
		return (b.x - a.x) * ((c.y * -1) - (a.y * -1)) - (c.x - a.x) * ((b.y * -1) - (a.y * -1));
	}

};



//éãäEÇäÓèÄÇ∆ÇµÇΩçıìG
class SightSearch
{
public:
	SightSearch();
	void Init(KuroEngine::Transform *transform);
	//éãäEì‡Ç…ñ⁄ïWÇÃï®ëÃÇÕå©Ç¬Ç©Ç¡ÇΩÇ©
	bool IsFind(const KuroEngine::Vec3<float> &transform, float viewAngle);
	void DebugDraw(KuroEngine::Camera &camera);

private:
	struct Ray
	{
		KuroEngine::Vec3<float>m_startPos;
		KuroEngine::Vec3<float>m_dir;
		float m_length;
		KuroEngine::Vec3<float> GetEndPos()
		{
			return m_startPos + m_dir * m_length;
		}
	};

	struct Sight
	{
		Ray ray;
		bool hitFlag;
		Sight() :hitFlag(false)
		{};
	};

	std::vector<Ray>m_rayArray;
	std::vector<Sight>m_sightRay;

	KuroEngine::Transform *m_transformPtr;

	CollisionDetectionRayAndPoint collisionDetection;


	void InitSight(const KuroEngine::Vec3<float> &basePos, float length, float nearScale, float nearHeight, float farScale, float farHeight)
	{
		int index = 0;
		//âEè„
		{
			m_rayArray[index].m_startPos = basePos + KuroEngine::Vec3<float>(nearScale, nearHeight, 0.0f);
			m_rayArray[index].m_length = length;
			KuroEngine::Vec3<float>farPos(m_rayArray[index].m_startPos + KuroEngine::Vec3<float>(farScale, farHeight, length));
			KuroEngine::Vec3<float>distance(farPos - m_rayArray[index].m_startPos);
			distance.Normalize();
			m_rayArray[index].m_dir = distance;
		}
		++index;
		//ç∂è„
		{
			m_rayArray[index].m_startPos = basePos + KuroEngine::Vec3<float>(-nearScale, nearHeight, 0.0f);
			m_rayArray[index].m_length = length;
			KuroEngine::Vec3<float>farPos(m_rayArray[index].m_startPos + KuroEngine::Vec3<float>(-farScale, farHeight, length));
			KuroEngine::Vec3<float>distance(farPos - m_rayArray[index].m_startPos);
			distance.Normalize();
			m_rayArray[index].m_dir = distance;
		}
		++index;
		//âEâ∫
		{
			m_rayArray[index].m_startPos = basePos + KuroEngine::Vec3<float>(nearScale, -nearHeight, 0.0f);
			m_rayArray[index].m_length = length;
			KuroEngine::Vec3<float>farPos(m_rayArray[index].m_startPos + KuroEngine::Vec3<float>(farScale, -farHeight, length));
			KuroEngine::Vec3<float>distance(farPos - m_rayArray[index].m_startPos);
			distance.Normalize();
			m_rayArray[index].m_dir = distance;
		}
		++index;
		//ç∂â∫
		{
			m_rayArray[index].m_startPos = basePos + KuroEngine::Vec3<float>(-nearScale, -nearHeight, 0.0f);
			m_rayArray[index].m_length = length;
			KuroEngine::Vec3<float>farPos(m_rayArray[index].m_startPos + KuroEngine::Vec3<float>(-farScale, -farHeight, length));
			KuroEngine::Vec3<float>distance(farPos - m_rayArray[index].m_startPos);
			distance.Normalize();
			m_rayArray[index].m_dir = distance;
		}

		for (int i = 0; i < m_sightRay.size(); ++i)
		{
			m_sightRay[i].ray = m_rayArray[i];
		}
	}
};

//é¸ï”îÕàÕÇäÓèÄÇ∆ÇµÇΩçıìG
class CircleSearch
{
public:
	CircleSearch();
	void Init(const Sphere &sphere);
	//éãäEì‡Ç…ñ⁄ïWÇÃï®ëÃÇÕå©Ç¬Ç©Ç¡ÇΩÇ©
	bool IsFind(const Sphere &sphere);
	void DebugDraw(KuroEngine::Camera &camera);

private:
	Sphere m_hitBox;

	std::shared_ptr<KuroEngine::Model>m_hitBoxModel;

};