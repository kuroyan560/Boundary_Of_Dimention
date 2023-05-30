#pragma once
#include"KuroEngine.h"
#include"Common/Transform.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/Timer.h"
#include"../Graphics/BasicDraw.h"
#include<array>
#include"EnemyPatrol.h"
#include"EnemySearch.h"
#include"../Player/PlayerCollision.h"

class BulletManager
{
public:
	BulletManager();

	void Init(const KuroEngine::Vec3<float> *pos, float scale, const KuroEngine::Vec3<float> *vel, float bulletSpan);
	void Finalize();
	void Update(float bulletSpan, const Sphere &hitBox, bool shotFlag);
	void Draw(KuroEngine::Camera &camera);

	bool IsHit()
	{
		return m_hitFlag;
	}
private:

	const KuroEngine::Vec3<float> *m_pos, *m_vel;
	float m_scale;
	bool m_hitFlag;

	class Bullet
	{
	public:
		Bullet() :m_isAliveFlag(false), m_tex(KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/Particle/Cloud.png"))
		{
			m_sphere.m_centerPos = &m_pos;
			m_sphere.m_radius = &m_radius;
		};
		void Init(const KuroEngine::Vec3<float> &pos, const KuroEngine::Vec3<float> &vel, float radius)
		{
			m_pos = pos;
			m_vel = vel;
			m_isAliveFlag = true;
			m_deadFlag = false;
			m_radius = radius;
			m_baseRadius = m_radius;
			m_timer.Reset(60 * 3);
			m_deadTiemr.Reset(10);
		}
		void Finalize()
		{
			m_isAliveFlag = false;
		}
		void Update()
		{
			//—LŒø‚©‚Ç‚¤‚©
			if (!m_isAliveFlag)
			{
				return;
			}
			m_timer.UpdateTimer();
			//Ž€–S‰‰o—P—\ŽžŠÔ
			if (m_deadFlag || m_timer.IsTimeUp())
			{
				m_deadTiemr.UpdateTimer();
			}
			//Ž€–S
			if (m_deadTiemr.IsTimeUp())
			{
				m_isAliveFlag = false;
			}

			m_radius = m_baseRadius * m_deadTiemr.GetInverseTimeRate();
			m_pos += m_vel;
		}
		void Draw(KuroEngine::Camera &camera)
		{
			if (!m_isAliveFlag)
			{
				return;
			}
			m_transform.SetPos(m_pos);
			m_transform.SetScale(m_radius);
			BasicDraw::Instance()->DrawBillBoard(camera, m_transform, m_tex);
		}
		bool IsAlive()
		{
			return m_isAliveFlag;
		}

		bool Hit(const Sphere &arg_hitbox)
		{
			if (Collision::Instance()->CheckCircleAndCircle(m_sphere, arg_hitbox))
			{
				m_deadTiemr.Reset(0);
				m_timer.Reset(0);
				m_deadFlag = true;
				return true;
			}
			return false;
		}

	private:
		Sphere m_sphere;
		KuroEngine::Vec3<float> m_pos, m_vel;
		float m_radius;
		float m_baseRadius;
		bool m_isAliveFlag;
		bool m_deadFlag;

		KuroEngine::Transform m_transform;
		KuroEngine::Timer m_timer, m_deadTiemr;
		std::shared_ptr<KuroEngine::TextureBuffer>m_tex;
	};

	std::array<Bullet, 10>m_bulltArray;
	KuroEngine::Timer m_timer;

};

class MeshHitBox
{
public:


	void Update()
	{
		//collision.CheckHit();



	}

	PlayerCollision collision;

};