#pragma once
#include"KuroEngine.h"
#include"ForUser/Object/Model.h"
#include"../Graphics/BasicDraw.h"
#include"ForUser/Timer.h"

//�]�V�����̃G�t�F�N�g
class HeadAttackEffect
{
public:
	HeadAttackEffect();

	void Init(const KuroEngine::Vec3<float> &pos);

	void Update();
	void Draw(KuroEngine::Camera &camera);


	class Particle
	{
	public:
		Particle();
	};
};

//�]�V�U����̒n�ʒ��n�G�t�F�N�g
class AfterHeadEffectEffect
{
public:
	AfterHeadEffectEffect();

	void Init(const KuroEngine::Vec3<float> &pos);

	void Update();
	void Draw(KuroEngine::Camera &camera);

	class Particle
	{
	public:
		Particle() :m_initFlag(false)
		{};
		void Init(const KuroEngine::Vec3<float> &pos, const KuroEngine::Vec3<float> &vel, std::shared_ptr<KuroEngine::TextureBuffer>tex)
		{
			m_pos = pos;
			m_vel = vel;
			m_speed = KuroEngine::GetRand(0.1f, 0.5f);
			m_timer.Reset(30);
			m_tex = tex;
			m_initFlag = true;

			m_size = KuroEngine::GetRand(KuroEngine::Vec2<float>(3.0f, 3.3f), KuroEngine::Vec2<float>(15.0f, 15.0f));
		}

		void Update()
		{
			if (!m_initFlag)
			{
				return;
			}

			m_pos += m_vel * m_speed;

			if (m_timer.UpdateTimer())
			{
				m_initFlag = false;
			}
		}

		void Draw(KuroEngine::Camera &camera)
		{
			if (!m_initFlag)
			{
				return;
			}
			KuroEngine::Transform transform;
			transform.SetPos(m_pos);
			transform.SetScale({ m_size.x, m_size.y,m_size.y });
			BasicDraw::Instance()->DrawBillBoard(camera, transform, m_tex, KuroEngine::Color(1.0f, 1.0f, 1.0f, m_timer.GetInverseTimeRate()));
		}

	private:
		KuroEngine::Vec3<float>m_pos, m_vel;
		KuroEngine::Vec2<float>m_size;
		float m_speed;
		KuroEngine::Timer m_timer;
		std::shared_ptr<KuroEngine::TextureBuffer>m_tex;

		bool m_initFlag;
	};


	//1F�ڂɂł���
	//�w�i��---------------
	bool isFlashFlag;
	KuroEngine::Vec3<float>m_lightPos;
	KuroEngine::Vec2<float>m_lightSize;
	std::shared_ptr<KuroEngine::TextureBuffer> m_flashTex;
	//����---------------------------------------
	KuroEngine::Vec3<float>m_godRayPos;
	float m_godRayAngle;
	std::shared_ptr<KuroEngine::TextureBuffer> m_godRayTex;


	//���̌�
	//��------------
	std::array<Particle, 25>m_cloudParticleArray;
	//��---------------------------------------
	std::array<Particle, 25>m_lightTexArray;
	//���̂��蔲���~
	KuroEngine::Vec3<float>m_lightCirclePos;
	KuroEngine::Vec2<float>m_lightCircleSize;
	std::shared_ptr<KuroEngine::TextureBuffer> m_lightCircleTex;



	std::shared_ptr<KuroEngine::TextureBuffer>m_cloudTex;
	std::shared_ptr<KuroEngine::TextureBuffer>m_orbTex;

};