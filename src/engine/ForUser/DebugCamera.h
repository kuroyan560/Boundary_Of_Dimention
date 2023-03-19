#pragma once
#include<memory>
#include"Render/RenderObject/Camera.h"
#include"KuroEngine.h"

namespace KuroEngine
{
	class DebugCamera
	{
		std::shared_ptr<Camera>m_cam;

		//スケーリング
		Vec2<float>m_scale;
		
		//注視点のトランスフォーム（座標情報のみ格納）
		Transform m_gazePointTransform;

		//注視点をどの座標角度から見ているかの情報のトランスフォーム
		Transform m_posAngleTransform;

	public:
		DebugCamera();
		void Init(const Vec3<float>& InitPos);
		void Move();

		operator std::shared_ptr<Camera>& ()
		{
			return m_cam;
		}

		operator Camera& ()
		{
			return *m_cam;
		}
	};
}