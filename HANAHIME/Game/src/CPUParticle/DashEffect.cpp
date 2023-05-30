#include"DashEffect.h"

DashEffect::DashEffect()
{
	m_timer.Reset(RESET_TIMER);
	m_tex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/Particle/Cloud.png");
}

void DashEffect::Finalize()
{
	for (auto &obj : m_particleArray)
	{
		obj.Finalize();
	}
}

void DashEffect::Update(const KuroEngine::Vec3<float> &pos, bool activeFlag)
{
	for (auto &obj : m_particleArray)
	{
		if (m_timer.IsTimeUp() && !obj.IsAlive() && activeFlag)
		{
			obj.Init(pos, m_tex);
			m_timer.Reset(RESET_TIMER);
		}
		obj.Update();
	}
	m_timer.UpdateTimer();
}

void DashEffect::Draw(KuroEngine::Camera &camera)
{
	for (auto &obj : m_particleArray)
	{
		obj.Draw(camera);
	}
}