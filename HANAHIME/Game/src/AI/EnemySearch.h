#pragma once
#include"KuroEngine.h"
#include"Render/RenderObject/Camera.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"FrameWork/Importer.h"

//ü•ª‚Æ“_‚Ì”»’è
class CollisionDetectionRayAndPoint
{
public:
	//ü•ª‚Æ“_‚ÌŠOÏ
	static float CaluCross(const KuroEngine::Vec3<float> &a, const KuroEngine::Vec3<float> &b, const KuroEngine::Vec3<float> &c)
	{
		return (b.x - a.x) * ((c.y * -1) - (a.y * -1)) - (c.x - a.x) * ((b.y * -1) - (a.y * -1));
	}

};

struct Sphere
{
	const KuroEngine::Vec3<float>*m_centerPos;
	const float *m_radius;
};


//‹ŠE‚ğŠî€‚Æ‚µ‚½õ“G
class SightSearch
{
public:
	SightSearch();
	void Init(
	float nearScale,
	float farScale,
	float length,
	float nearHeight,
	float farHeight
	);
	//‹ŠE“à‚É–Ú•W‚Ì•¨‘Ì‚ÍŒ©‚Â‚©‚Á‚½‚©
	bool IsFind(const KuroEngine::Vec3<float> &pos, KuroEngine::Transform *rotation);
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

	CollisionDetectionRayAndPoint collisionDetection;


	void InitSight(const KuroEngine::Vec3<float> &basePos, float length, float nearScale, float nearHeight, float farScale, float farHeight)
	{
		int index = 0;
		//‰Eã
		{
			m_rayArray[index].m_startPos = basePos + KuroEngine::Vec3<float>(nearScale, nearHeight, 0.0f);
			m_rayArray[index].m_length = length;
			KuroEngine::Vec3<float>farPos(m_rayArray[index].m_startPos + KuroEngine::Vec3<float>(farScale, farHeight, length));
			KuroEngine::Vec3<float>distance(farPos - m_rayArray[index].m_startPos);
			distance.Normalize();
			m_rayArray[index].m_dir = distance;
		}
		++index;
		//¶ã
		{
			m_rayArray[index].m_startPos = basePos + KuroEngine::Vec3<float>(-nearScale, nearHeight, 0.0f);
			m_rayArray[index].m_length = length;
			KuroEngine::Vec3<float>farPos(m_rayArray[index].m_startPos + KuroEngine::Vec3<float>(-farScale, farHeight, length));
			KuroEngine::Vec3<float>distance(farPos - m_rayArray[index].m_startPos);
			distance.Normalize();
			m_rayArray[index].m_dir = distance;
		}
		++index;
		//‰E‰º
		{
			m_rayArray[index].m_startPos = basePos + KuroEngine::Vec3<float>(nearScale, -nearHeight, 0.0f);
			m_rayArray[index].m_length = length;
			KuroEngine::Vec3<float>farPos(m_rayArray[index].m_startPos + KuroEngine::Vec3<float>(farScale, -farHeight, length));
			KuroEngine::Vec3<float>distance(farPos - m_rayArray[index].m_startPos);
			distance.Normalize();
			m_rayArray[index].m_dir = distance;
		}
		++index;
		//¶‰º
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

//ü•Ó”ÍˆÍ‚ğŠî€‚Æ‚µ‚½õ“G
class CircleSearch
{
public:
	CircleSearch();
	void Init(const Sphere &sphere);
	//‹ŠE“à‚É–Ú•W‚Ì•¨‘Ì‚ÍŒ©‚Â‚©‚Á‚½‚©
	bool IsFind(const Sphere &sphere);
	void DebugDraw(KuroEngine::Camera &camera);

private:
	Sphere m_hitBox;

	std::shared_ptr<KuroEngine::Model>m_hitBoxModel;
	
};