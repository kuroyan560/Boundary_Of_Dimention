#pragma once
#include"DirectX12/D3D12App.h"
#include"DirectX12/D3D12Data.h"
#include"../Graphics/BasicDraw.h"
#include"ForUser/Object/Object.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"Render/RenderObject/Camera.h"

class SignSpotFireFly
{
public:
	SignSpotFireFly(std::shared_ptr<KuroEngine::RWStructuredBuffer>particle_buffer);

	void Update();
	void Draw(KuroEngine::Camera &camera);

	void Finish();
	void GoThisPos(const KuroEngine::Vec3<float>pos);

	float GetAlphaRate()
	{
		return data.alpha;
	}

private:
	struct ParticleData
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 vel;
	};
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_fireFlyArrayBuffer;
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_particleBuffer;

	struct CommonData
	{
		DirectX::XMMATRIX scaleRotaMat;
		DirectX::XMFLOAT3 emittPos;
		float alpha;
	};
	CommonData data;
	std::shared_ptr<KuroEngine::ConstantBuffer>m_commonBuffer;

	std::shared_ptr<KuroEngine::ComputePipeline> m_cInitPipeline, m_cUpdatePipeline;

	bool m_finishFlag;

	//CPU挙動デバック
	std::shared_ptr<KuroEngine::ModelObject> m_particleMove;
	KuroEngine::Vec3<float>m_particlePos;
	KuroEngine::Vec3<float>m_vel;

public:
	KuroEngine::Vec3<float>m_prevPos;
};

