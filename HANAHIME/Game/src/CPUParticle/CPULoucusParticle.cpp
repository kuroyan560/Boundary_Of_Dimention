#include "CPULoucusParticle.h"

void CPULoucusEmitter::Init(std::vector<KuroEngine::Vec3<float>> posArrat)
{
	//スプライン曲線を基準に2次元の円を作りランダムで配置

	limitMaxNum = static_cast<int>(posArrat.size());
	int startIndex = 0;
	float rateMax = static_cast<float>(m_particle.size() / limitMaxNum);
	float rateMin = 0.0f;

	const int MAX_TIME = 10;

	for (int i = 0; i < m_particle.size(); ++i)
	{
		float rate = (i - rateMin) / rateMax;
		if (std::isnan(rate))
		{
			rate = 0.0f;
		}

		KuroEngine::Vec3<float>splinePos = {};
		bool dontIncrementNunFlag = false;
		//スプライン曲線の配置
		if (limitMaxNum - 2 <= startIndex)
		{
			startIndex = KuroEngine::GetRand(0, limitMaxNum - 3);
			dontIncrementNunFlag = true;
		}
		splinePos = SplinePosition(posArrat, startIndex, rate, false);

		//円を作って配置
		float radian = KuroEngine::Angle::ConvertToRadian(KuroEngine::GetRand(0.0f, 360.0f));
		float radius = KuroEngine::GetRand(0.1f, 2.0f);
		KuroEngine::Vec3<float>circlePos(cosf(radian) * radius, sinf(radian) * radius, sinf(radian) * radius);


		int time = startIndex * MAX_TIME + static_cast<int>(rate * MAX_TIME);
		m_particle[i].Init(splinePos+ circlePos, KuroEngine::GetRand(0.1f, 1.0f), time);

		//インデックス追加
		if (1.0f <= rate && !dontIncrementNunFlag)
		{
			++startIndex;
			rateMin += rateMax;
		}
	}

	m_finishFlag = false;
}

void CPULoucusEmitter::Update()
{
	//スプライン曲線に沿って光らせる

	//球を作ってランダムで配置&時間経過で光らせる

	int countDeadNum = 0;
	for (auto &obj : m_particle)
	{
		if (obj.IsDead())
		{
			++countDeadNum;
		}

		obj.Update();
	}

	if (300 <= countDeadNum)
	{
		m_finishFlag = true;
	}

}

void CPULoucusEmitter::Draw(KuroEngine::Camera &camera)
{
	for (auto &obj : m_particle)
	{
		obj.Draw(camera);
	}
}

CPULoucusEmitter::CPUParticle::CPUParticle():m_initFlag(false)
{
	m_tex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/GreenOrb.png");
}

void CPULoucusEmitter::CPUParticle::Init(const KuroEngine::Vec3<float> &pos, float scale, int time)
{
	m_pos = pos;
	m_vel = KuroEngine::Vec3<float>(KuroEngine::GetRand(0.001f, 0.01f), KuroEngine::GetRand(0.001f, 0.01f), KuroEngine::GetRand(0.001f, 0.01f));
	m_size = { scale,scale };
	m_appearTimer.Reset(time);
	m_disappearTimer.Reset(60);
	m_initFlag = false;
}

void CPULoucusEmitter::CPUParticle::Update()
{

	if (m_appearTimer.UpdateTimer())
	{
		m_disappearTimer.UpdateTimer();
		m_initFlag = true;
	}
	if (m_disappearTimer.IsTimeUp())
	{
		m_initFlag = false;
		return;
	}

	//少しづつ拡散させる
	m_pos += m_vel;

}

void CPULoucusEmitter::CPUParticle::Draw(KuroEngine::Camera &camera)
{
	//時間経過で消滅
	if (m_initFlag)
	{
		KuroEngine::DrawFuncBillBoard::Graph(camera, m_pos, m_size, m_tex, m_disappearTimer.GetInverseTimeRate());
	}
}
