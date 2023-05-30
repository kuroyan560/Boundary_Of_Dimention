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

	//ジャンプ
	m_vel = m_dir * m_speed;
	m_vel += m_transform.GetUp() * 2.0f;	//上ベクトルに合わせて移動量を見る
	m_gravity = {};
	m_basePos = m_transform.GetPos();

	m_initTransform = m_transform;
	m_isHitGroundFlag = false;
}

HeadAttackData EnemyHeadAttack::Update()
{
	m_timer.UpdateTimer();

	//X軸に刺してぐるぐる回転させる処理
	m_angle += 5.0f;
	m_dir.y -= 0.01f;

	//ヒットエフェクト


	//重量ありのジャンプ
	m_gravity += m_transform.GetUp() * 0.1f;
	m_vel += -m_gravity;

	float getBasePos = 0.0f;
	if (1.0f <= abs(m_initTransform.GetUp().x))
	{
		getBasePos = m_initTransform.GetUp().x;
	}
	if (1.0f <= abs(m_initTransform.GetUp().y))
	{
		getBasePos = m_initTransform.GetUp().y;
	}
	if (1.0f <= abs(m_initTransform.GetUp().z))
	{
		getBasePos = m_initTransform.GetUp().z;
	}
	//地面に着地したらエフェクト
	if (m_basePos.y <= getBasePos)
	{
		m_isHitGroundFlag = true;
	}


	HeadAttackData headAttackData;
	headAttackData.m_dir = m_vel;
	headAttackData.m_rotation = DirectX::XMQuaternionRotationAxis(m_rotationVec, KuroEngine::Angle::ConvertToRadian(m_angle));
	return headAttackData;
}
