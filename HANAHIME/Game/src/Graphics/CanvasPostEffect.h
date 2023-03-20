#pragma once
#include<memory>
namespace KuroEngine
{
	class TextureBuffer;
}

class CanvasPostEffect
{
	std::shared_ptr<KuroEngine::TextureBuffer>m_gradationTex;
	std::shared_ptr<KuroEngine::TextureBuffer>m_materialTex;
public:
	CanvasPostEffect();
	void Execute();
};