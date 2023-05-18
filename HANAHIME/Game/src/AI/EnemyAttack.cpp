#include"EnemyAttack.h"

BulletManager::BulletManager()
{
}

void BulletManager::Init(const KuroEngine::Vec3<float> *pos, float scale, const KuroEngine::Vec3<float> *vel, float bulletSpan)
{
	m_pos = pos;
	m_vel = vel;
	m_scale = scale;
	m_timer.Reset(bulletSpan);
}

void BulletManager::Finalize()
{
	for (auto &obj : m_bulltArray)
	{
		obj.Finalize();
	}
}

void BulletManager::Update(float bulletSpan, const Sphere &hitBox, bool shotFlag)
{
	m_hitFlag = false;
	for (auto &obj : m_bulltArray)
	{
		if (!obj.IsAlive() && m_timer.IsTimeUp() && shotFlag)
		{
			obj.Init(*m_pos, *m_vel, m_scale);
			m_timer.Reset(bulletSpan);
		}
		if (obj.Hit(hitBox))
		{
			m_hitFlag = true;
		}
		obj.Update();
	}
	m_timer.UpdateTimer();
}

void BulletManager::Draw(KuroEngine::Camera &camera)
{
	for (auto &obj : m_bulltArray)
	{
		obj.Draw(camera);
	}
}