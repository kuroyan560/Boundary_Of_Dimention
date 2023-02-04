#pragma once
#include"KuroEngine.h"
#include"Common/Transform2D.h"
#include<array>
#include<memory>

namespace KuroEngine
{
	class Sprite;

	//ペルソナで出てくるような四角
	class RandBox2D
	{
		std::shared_ptr<Sprite>m_sprite;
		float m_t;

	public:
		int m_interval = 20;
		Vec2<float> m_maxVal = { 20.0f,20.0f };

		Transform2D& Transform();

		RandBox2D();
		void Init();
		void Update();
		void Draw();

		void ImguiDebug();

		void SetSize(Vec2<float>arg_size);
		void SetAnchorPoint(Vec2<float>arg_anchor);
		void SetColor(Color arg_color);
	};
}