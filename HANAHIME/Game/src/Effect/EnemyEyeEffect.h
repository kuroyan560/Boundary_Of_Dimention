#pragma once
#include"KuroEngine.h"
#include"Common/Transform.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/Timer.h"
#include"../Graphics/BasicDraw.h"
#include<array>

class EnemyEyeEffect
{
public:
	EnemyEyeEffect(KuroEngine::Transform *transform)
	{
		m_transform.SetParent(transform);
		m_tex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/Cloud.png");
	}

	void Update(const KuroEngine::Vec3<float> &pos)
	{
		m_transform.SetPos(pos);
	}
	void Draw(KuroEngine::Camera &camera)
	{
		BasicDraw::Instance()->DrawBillBoard(camera, m_transform, m_tex);
	}

private:
	KuroEngine::Transform m_transform;
	std::shared_ptr<KuroEngine::TextureBuffer>m_tex;
};