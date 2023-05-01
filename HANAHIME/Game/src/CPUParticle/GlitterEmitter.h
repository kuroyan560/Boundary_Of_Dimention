#pragma once
#include"KuroEngine.h"
#include"DirectX12/D3D12App.h"
#include"Render/RenderObject/Camera.h"
#include"ForUser/Timer.h"
#include<array>

class GlitterEmitter
{
public:
	GlitterEmitter();

	void Init(const KuroEngine::Vec3<float> &pos);
	void Finalize();
	void Update();
	void Draw(KuroEngine::Camera &camera);

private:
	class Particle
	{
	public:
		Particle();
		void Init(const KuroEngine::Vec3<float> &pos, const KuroEngine::Vec3<float> &vel);
		void Finalize()
		{
			m_initFlag = false;
		}
		void Update();
		void Draw(KuroEngine::Camera &camera);

		bool IsAlive()
		{
			return m_initFlag;
		}
	private:
		bool m_initFlag;
		KuroEngine::Vec3<float> m_pos, m_vel;
		std::shared_ptr<KuroEngine::TextureBuffer> m_buffer;

		KuroEngine::Timer m_timer;
	};

	KuroEngine::Vec3<float>leftUpPos;
	KuroEngine::Vec3<float>rightDownPos;

	std::array<Particle, 200>m_particleArray;

};
