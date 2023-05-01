#pragma once
#include"KuroEngine.h"
#include"ForUser/DrawFunc/BillBoard/DrawFuncBillBoard.h"
#include"ForUser/Timer.h"
#include"DirectX12/D3D12App.h"
#include<array>

class CPULoucusEmitter
{
public:
	void Init(std::vector<KuroEngine::Vec3<float>>posArray);
	void Update();
	void Draw(KuroEngine::Camera &camera);

	bool IsFinish()
	{
		return m_finishFlag;
	}
private:
	KuroEngine::Vec3<float>m_pos;
	int limitMaxNum;
	bool m_finishFlag;


	class CPUParticle
	{
	public:
		CPUParticle();
		void Init(const KuroEngine::Vec3<float> &pos, float scale, int time);
		void Update();
		void Draw(KuroEngine::Camera &camera);

		bool IsDead()
		{
			return m_disappearTimer.IsTimeUp();
		}

	private:
		KuroEngine::Vec3<float>m_pos;
		KuroEngine::Vec3<float>m_vel;
		KuroEngine::Vec2<float>m_size;
		KuroEngine::Timer m_appearTimer;
		KuroEngine::Timer m_disappearTimer;
		std::shared_ptr<KuroEngine::TextureBuffer>m_tex;

		bool m_initFlag;
	};
	std::array<CPUParticle, 500>m_particle;


	KuroEngine::Vec3<float> SplinePosition(const std::vector<KuroEngine::Vec3<float>> &points, size_t startIndex, float t, bool Loop)
	{
		if (startIndex < 1)
		{
			return points[1];
		}
		DirectX::XMVECTOR p0 = points[startIndex - 1];
		DirectX::XMVECTOR p1 = points[startIndex];
		DirectX::XMVECTOR p2;
		DirectX::XMVECTOR p3;

		size_t subIndex = 3;
		if (Loop == true)
		{
			if (startIndex > points.size() - subIndex)
			{
				p2 = points[1];
				p3 = points[2];
			}
			else
			{
				p2 = points[startIndex + 1];
				p3 = points[startIndex + 2];
			}
		}
		else
		{
			int size = static_cast<int>(points.size());
			if (startIndex > size - 3)return points[size - 3];
			p2 = points[startIndex + 1];
			p3 = points[startIndex + 2];
		}
		using namespace DirectX;
		DirectX::XMVECTOR anser2 =
			0.5 * ((2 * p1 + (-p0 + p2) * t) +
				(2 * p0 - 5 * p1 + 4 * p2 - p3) * (t * t) +
				(-p0 + 3 * p1 - 3 * p2 + p3) * (t * t * t));


		KuroEngine::Vec3<float>result = { anser2.m128_f32[0],anser2.m128_f32[1],anser2.m128_f32[2] };
		return result;
	};

	KuroEngine::Vec3<float> GetUpVec(KuroEngine::Vec3<float>pos, KuroEngine::Vec3<float>nextPos)
	{
		KuroEngine::Vec3<float>eyePos = nextPos;
		KuroEngine::Vec3<float>eyeDir = pos - eyePos;
		eyeDir.Normalize();

		//②上ベクトルを固定し、右ベクトルを求める。
		KuroEngine::Vec3<float> rightVec = eyeDir.Cross({ 0,-1,0 });
		rightVec.Normalize();

		//③視点ベクトルと右ベクトルから正しい上ベクトルを求める。
		KuroEngine::Vec3<float> upVec = eyeDir.Cross(rightVec);
		upVec.Normalize();

		return upVec;
	}

};
