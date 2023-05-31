#pragma once
#include"KuroEngine.h"
#include"Render/RenderObject/Camera.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"FrameWork/Importer.h"
#include"Common/Singleton.h"
#include"../Effect/HitEffect.h"

class EnemyKnockBack
{
public:
	EnemyKnockBack();

	void Init(const KuroEngine::Vec3<float> &pos, const KuroEngine::Vec3<float> &dir, int time);
	KuroEngine::Vec3<float> Update();
	bool IsDone()
	{
		return m_timer.IsTimeUp();
	}

private:
	KuroEngine::Vec3<float>m_pos, m_vel;
	float m_speed;
	float m_maxSpeed;
	KuroEngine::Timer m_timer;
};

struct HeadAttackData
{
	KuroEngine::Vec3<float>m_dir;
	KuroEngine::Quaternion m_rotation;
};

class EnemyHeadAttack
{
public:
	EnemyHeadAttack();

	void Init(const KuroEngine::Transform &transform, const KuroEngine::Vec3<float> &dir);
	void Finalize();
	HeadAttackData Update(const KuroEngine::Vec3<float> &pos);

	void ParticleUpdate();
	void ParticleDraw(KuroEngine::Camera &camera);

	bool IsHitGround()
	{
		return m_isHitGroundFlag;
	}

private:
	KuroEngine::Transform m_transform;
	KuroEngine::Transform m_initTransform;
	KuroEngine::Vec3<float>m_dir;
	KuroEngine::Vec3<float>m_rotationVec;

	float m_angle;
	KuroEngine::Vec3<float> m_speed;

	//èdó çûÇ›ÇÃÉWÉÉÉìÉv
	KuroEngine::Vec3<float> m_vel;
	KuroEngine::Vec3<float> m_basePos;
	KuroEngine::Vec3<float> m_gravity;
	bool m_isHitGroundFlag, m_prevIsHitGroundFlag;


	KuroEngine::Vec3<float> SplinePosition(const std::vector<KuroEngine::Vec3<float>> &points, size_t startIndex, float t, bool Loop);
	XMVECTOR ConvertVec3toXMVECTOR(const KuroEngine::Vec3<float> &vec)
	{
		return { vec.x, vec.y , vec.z , 0.0f };
	}

	KuroEngine::Timer m_blowTimer;

	KuroEngine::Vec3<float>m_nowPos, m_prevPos;
	std::vector<KuroEngine::Vec3<float>>m_limitPosArray;
	KuroEngine::Timer m_splineTimer;
	size_t m_splineIndex;
	bool m_initFlag;
	float m_timer;

	AfterHeadEffectEffect m_hitGroundEffecct;

};