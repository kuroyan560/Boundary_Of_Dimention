#include"GlitterEmitter.h"
#include"ForUser/DrawFunc/BillBoard/DrawFuncBillBoard.h"

GlitterEmitter::GlitterEmitter()
{
}

void GlitterEmitter::Init(const KuroEngine::Vec3<float> &pos)
{
	const float scale = 10.0f;
	leftUpPos = { 0.0f,0.0f,0.0f };
	rightDownPos = { 0.0f, 0.0f, 0.0f };
	leftUpPos -= KuroEngine::Vec3<float>(scale, scale, scale);
	rightDownPos += KuroEngine::Vec3<float>(scale, scale, scale);

	//正面になるようにパーティクルを配置
	for (auto &obj : m_particleArray)
	{
		if (!obj.IsAlive())
		{
			KuroEngine::Vec3<float>rand(KuroEngine::GetRand(leftUpPos, rightDownPos));
			obj.Init(pos, pos + rand);
		}
	}
}

void GlitterEmitter::Finalize()
{
	for (auto & obj : m_particleArray)
	{
		obj.Finalize();
	}
}

void GlitterEmitter::Update()
{
	for (auto &obj : m_particleArray)
	{
		if (obj.IsAlive())
		{
			obj.Update();
		}
	}
}

void GlitterEmitter::Draw(KuroEngine::Camera &camera)
{
	for (auto &obj : m_particleArray)
	{
		if (obj.IsAlive())
		{
			obj.Draw(camera);
		}
	}
}




GlitterEmitter::Particle::Particle()
{
	m_initFlag = false;
	m_buffer = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/GreenOrb.png");
}

void GlitterEmitter::Particle::Init(const KuroEngine::Vec3<float> &pos, const KuroEngine::Vec3<float> &vel)
{
	m_pos = pos;
	m_vel = vel;
	m_initFlag = true;
	m_timer.Reset(KuroEngine::GetRand(60, 120));
}

void GlitterEmitter::Particle::Update()
{
	m_pos = KuroEngine::Math::Lerp(m_pos, m_vel, 0.1f);
	m_timer.UpdateTimer();

	if (m_timer.IsTimeUp())
	{
		m_initFlag = false;
	}
}

void GlitterEmitter::Particle::Draw(KuroEngine::Camera &camera)
{
	KuroEngine::DrawFuncBillBoard::Graph(camera, m_pos, { 1.0f,1.0f }, m_buffer, m_timer.GetInverseTimeRate());
}