#include "HitEffect.h"

HeadAttackEffect::HeadAttackEffect()
{
}

void HeadAttackEffect::Init(const KuroEngine::Vec3<float> &pos)
{
}

void HeadAttackEffect::Update()
{
}

void HeadAttackEffect::Draw(KuroEngine::Camera &camera)
{
}

AfterHeadEffectEffect::AfterHeadEffectEffect()
{
	m_cloudTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/Particle/Cloud.png");
	m_orbTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/GreenOrb.png");
}

void AfterHeadEffectEffect::Init(const KuroEngine::Vec3<float> &pos)
{
	isFlashFlag = false;

	for (int i = 0; i < m_cloudParticleArray.size(); ++i)
	{
		KuroEngine::Vec3<float> range(5.0f, 5.0f, 0.0f);
		KuroEngine::Vec3<float> pos(KuroEngine::GetRand(pos - range, pos + range));
		float angle = KuroEngine::GetRand(0.0f, 360.0f);

		float radian = KuroEngine::Angle::ConvertToRadian(angle);
		KuroEngine::Vec3<float> vel(
			sinf(radian) * cosf(radian),
			sinf(radian) * sinf(radian),
			cosf(radian)
		);

		m_cloudParticleArray[i].Init(pos, vel, m_cloudTex);
	}

	for (int i = 0; i < m_lightTexArray.size(); ++i)
	{
		KuroEngine::Vec3<float> range(5.0f, 5.0f, 0.0f);
		KuroEngine::Vec3<float> pos(KuroEngine::GetRand(pos - range, pos + range));
		float angle = KuroEngine::GetRand(0.0f, 360.0f);

		float radian = KuroEngine::Angle::ConvertToRadian(angle);
		KuroEngine::Vec3<float> vel(
			sinf(radian) * cosf(radian),
			sinf(radian) * sinf(radian),
			cosf(radian)
		);
		m_lightTexArray[i].Init(pos, vel, m_orbTex);
	}
}

void AfterHeadEffectEffect::Update()
{
	for (int i = 0; i < m_cloudParticleArray.size(); ++i)
	{
		m_cloudParticleArray[i].Update();
	}
	for (int i = 0; i < m_lightTexArray.size(); ++i)
	{
		m_lightTexArray[i].Update();
	}
}

void AfterHeadEffectEffect::Draw(KuroEngine::Camera &camera)
{
	if (!isFlashFlag)
	{
		KuroEngine::Transform transform;
		transform.SetPos(m_lightPos);
		transform.SetScale({ m_lightSize.x, m_lightSize.y,m_lightSize.y });
		//BasicDraw::Instance()->DrawBillBoard(camera, transform, m_flashTex);

		transform.SetPos(m_godRayPos);
		transform.SetScale({ 1.0f,1.0f,1.0f });
		//BasicDraw::Instance()->DrawBillBoard(camera, transform, m_godRayTex);
		isFlashFlag = true;
	}

	for (int i = 0; i < m_cloudParticleArray.size(); ++i)
	{
		m_cloudParticleArray[i].Draw(camera);
	}
	for (int i = 0; i < m_lightTexArray.size(); ++i)
	{
		m_lightTexArray[i].Draw(camera);
	}
}
