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
		float distance = sphereA.m_centerPos->Distance(*(sphereB.m_centerPos));
		float sumRadist = *(sphereA.m_radius) + *(sphereB.m_radius);
		return (distance <= sumRadist);
	}

	bool CheckPointAndEdgeOfCircle(
		const Sphere &attack_hitBox,
		const KuroEngine::Vec3<float> &playerPos,
		const KuroEngine::Vec3<float> &upVec,
		const KuroEngine::Vec3<float> &vec_enemy_player
	)
	{
		float distance = attack_hitBox.m_centerPos->Distance(playerPos);
		float dot = upVec.Dot(vec_enemy_player);

		const float limit = 1.0f;
		bool inFlag = *(attack_hitBox.m_radius) - 1.0f <= distance;
		bool outFlag = distance <= *(attack_hitBox.m_radius) + 1.0f;

		bool isInCirlceFlag = inFlag && outFlag;
		bool isTouchEdgeFlag = abs(dot) <= 0.5f;

		return isInCirlceFlag && isTouchEdgeFlag;
	}


	KuroEngine::Vec3<float>PushBackEnemy(const Sphere &attack_hitBoxA, const Sphere &attack_hitBoxB)
	{
		//�������Ă���m�F
		if (!CheckCircleAndCircle(attack_hitBoxA, attack_hitBoxB))
		{
			return KuroEngine::Vec3<float>();
		}

		KuroEngine::Vec3<float>aPos(*(attack_hitBoxA.m_centerPos)), bPos(*(attack_hitBoxB.m_centerPos));
		float aRadius(*(attack_hitBoxA.m_radius)), bRadius(*(attack_hitBoxB.m_radius));

		//���������߂�
		KuroEngine::Vec3<float>distance(bPos - aPos);
		float length = (aRadius + bRadius) - distance.Length();

		if (distance.Length() <= 0.0f)
		{
			return KuroEngine::Vec3<float>();
		}
		if (length <= 0.0f)
		{
			return KuroEngine::Vec3<float>();
		}


		KuroEngine::Vec3<float> dir(distance.GetNormal());

		//�߂荞�ݗʂ����߂�
		KuroEngine::Vec3<float>pushBackOffset(dir * length);
		return pushBackOffset;
	}

};

//�����Ɠ_�̔���
class CollisionDetectionRayAndPoint
{
public:
	//�����Ɠ_�̊O��
	static float CaluCross(const KuroEngine::Vec3<float> &a, const KuroEngine::Vec3<float> &b, const KuroEngine::Vec3<float> &c)
	{
		return (b.x - a.x) * ((c.y * -1) - (a.y * -1)) - (c.x - a.x) * ((b.y * -1) - (a.y * -1));
	}

};



//���E����Ƃ������G
class SightSearch
{
public:
	SightSearch();
	void Init(KuroEngine::Transform *transform);
	//���E���ɖڕW�̕��̂͌���������
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
		//�E��
		{
			m_rayArray[index].m_startPos = basePos + KuroEngine::Vec3<float>(nearScale, nearHeight, 0.0f);
			m_rayArray[index].m_length = length;
			KuroEngine::Vec3<float>farPos(m_rayArray[index].m_startPos + KuroEngine::Vec3<float>(farScale, farHeight, length));
			KuroEngine::Vec3<float>distance(farPos - m_rayArray[index].m_startPos);
			distance.Normalize();
			m_rayArray[index].m_dir = distance;
		}
		++index;
		//����
		{
			m_rayArray[index].m_startPos = basePos + KuroEngine::Vec3<float>(-nearScale, nearHeight, 0.0f);
			m_rayArray[index].m_length = length;
			KuroEngine::Vec3<float>farPos(m_rayArray[index].m_startPos + KuroEngine::Vec3<float>(-farScale, farHeight, length));
			KuroEngine::Vec3<float>distance(farPos - m_rayArray[index].m_startPos);
			distance.Normalize();
			m_rayArray[index].m_dir = distance;
		}
		++index;
		//�E��
		{
			m_rayArray[index].m_startPos = basePos + KuroEngine::Vec3<float>(nearScale, -nearHeight, 0.0f);
			m_rayArray[index].m_length = length;
			KuroEngine::Vec3<float>farPos(m_rayArray[index].m_startPos + KuroEngine::Vec3<float>(farScale, -farHeight, length));
			KuroEngine::Vec3<float>distance(farPos - m_rayArray[index].m_startPos);
			distance.Normalize();
			m_rayArray[index].m_dir = distance;
		}
		++index;
		//����
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

//���Ӕ͈͂���Ƃ������G
class CircleSearch
{
public:
	CircleSearch();
	void Init(const Sphere &sphere);
	//���E���ɖڕW�̕��̂͌���������
	bool IsFind(const Sphere &sphere);
	void DebugDraw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr);

private:
	Sphere m_hitBox;

	std::shared_ptr<KuroEngine::Model>m_hitBoxModel;

};