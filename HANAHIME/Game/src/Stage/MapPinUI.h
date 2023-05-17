#pragma once
#include<memory>
#include"Common/Transform2D.h"
#include"Common/Transform.h"
#include"Common/Vec.h"
#include<vector>
#include<array>

namespace KuroEngine
{
	class TextureBuffer;
	class Camera;
}

//目的地を示すマップピン
class MapPinUI
{
	struct Content
	{
		std::shared_ptr<KuroEngine::TextureBuffer>m_tex;
		KuroEngine::Transform2D m_transform;
		float m_alpha = 1.0f;
		Content(std::string arg_texPath, KuroEngine::Transform2D* arg_parent);
	};

	std::shared_ptr<Content>m_smallSquare;
	std::shared_ptr<Content>m_middleSquare;
	std::shared_ptr<Content>m_largeSquare;

	enum PIN_MODE { PIN_MODE_IN_SCREEN, PIN_MODE_OUT_SCREEN, PIN_MODE_NUM };
	std::array<std::vector<std::weak_ptr<Content>>, PIN_MODE_NUM>m_mapPinUI;

	float m_pinSize = 92.0f;
	float m_arrowClampOffset = 65.0f;
	KuroEngine::Transform2D m_canvasTransform;

public:
	MapPinUI();
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::Vec3<float>arg_destinationPos);
};

