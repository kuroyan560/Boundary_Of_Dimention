#include "EnemyStatus.h"

EnemyKnockBack::EnemyKnockBack()
{
}

void EnemyKnockBack::Init(const KuroEngine::Vec3<float> &pos, const KuroEngine::Vec3<float> &dir, int time)
{
	m_maxSpeed = 1.0f;
	m_vel = dir * m_maxSpeed;
	m_timer.Reset(time);

	m_timer.UpdateTimer(static_cast<float>(time / 2));
}

KuroEngine::Vec3<float> EnemyKnockBack::Update()
{
	m_timer.UpdateTimer();
	KuroEngine::Vec3<float> vel = m_vel * KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Exp, m_timer.GetTimeRate(), 0.0f, m_maxSpeed);
	return vel;
}

EnemyHeadAttack::EnemyHeadAttack()
{
}

void EnemyHeadAttack::Init(const KuroEngine::Transform &transform, const KuroEngine::Vec3<float> &dir)
{
	m_transform = transform;
	m_dir = dir;
	m_speed = { 1.2f,1.0f,1.2f };
	m_angle = 0.0f;
	m_rotationVec = { 1.0f,0.0f,0.0f };

	m_timer.Reset(50);
}

HeadAttackData EnemyHeadAttack::Update()
{
	m_timer.UpdateTimer();

	//Xé≤Ç…éhÇµÇƒÇÆÇÈÇÆÇÈâÒì]Ç≥ÇπÇÈèàóù
	m_angle += 5.0f;

	m_dir.y -= 0.01f;
	HeadAttackData headAttackData;
	headAttackData.m_dir = m_dir * m_speed;
	headAttackData.m_rotation = DirectX::XMQuaternionRotationAxis(m_rotationVec, KuroEngine::Angle::ConvertToRadian(m_angle));
	return headAttackData;
}
