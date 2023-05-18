#pragma once
#include"KuroEngine.h"
#include"Common/Transform.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/Timer.h"
#include"../Graphics/BasicDraw.h"
#include<array>


class DashEffect
{
public:
	DashEffect();
	void Finalize();
	void Update(const KuroEngine::Vec3<float> &pos, bool activeFlag);
	void Draw(KuroEngine::Camera &camera);

private:


	class Particle
	{
	public:
		Particle() :m_isAliveFlag(false)
		{};

		void Init(const KuroEngine::Vec3<float> &pos, std::shared_ptr<KuroEngine::TextureBuffer>tex)
		{
			m_pos = pos;
			m_vel = KuroEngine::GetRand({ -1.0f,-1.0f,-1.0f }, { 1.0f,1.0f,1.0f });
			m_scale = KuroEngine::GetRand({ 1.0f,1.0f }, { 3.0f,3.0f });
			m_speed = 0.01f;
			m_isAliveFlag = true;
			m_appearTimer.Reset(15);
			m_disappearTimer.Reset(60);

			m_tex = tex;
		};
		void Finalize()
		{
			m_isAliveFlag = false;
		}
		void Update()
		{
			if (!m_isAliveFlag)
			{
				return;
			}
			m_pos += m_vel * m_speed;
			if (m_appearTimer.UpdateTimer())
			{
				m_isAliveFlag = !m_disappearTimer.UpdateTimer();
			}

			m_scale *= m_disappearTimer.GetInverseTimeRate();

		}
		void Draw(KuroEngine::Camera &camera)
		{
			if (!m_isAliveFlag)
			{
				return;
			}
			KuroEngine::Transform transform;
			transform.SetPos(m_pos);
			transform.SetScale({ m_scale.x,m_scale.y ,1.0f });
			KuroEngine::Color color(0.8f, 0.0f, 0.0f, 1.0f);
			BasicDraw::Instance()->DrawBillBoard(camera, transform, m_tex, color);
		}

		bool IsAlive()
		{
			return m_isAliveFlag;
		}

	private:
		KuroEngine::Vec3<float>m_pos, m_vel;
		KuroEngine::Vec2<float>m_scale;
		float m_speed;
		bool m_isAliveFlag;
		KuroEngine::Timer m_appearTimer, m_disappearTimer;
		std::shared_ptr<KuroEngine::TextureBuffer>m_tex;
	};

	static const int RESET_TIMER = 2;
	KuroEngine::Timer m_timer;
	std::shared_ptr<KuroEngine::TextureBuffer>m_tex;
	std::array<Particle, 100>m_particleArray;

};