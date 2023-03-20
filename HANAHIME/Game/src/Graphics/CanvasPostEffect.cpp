#include "CanvasPostEffect.h"
#include"DirectX12/D3D12App.h"

CanvasPostEffect::CanvasPostEffect()
{
	using namespace KuroEngine;
	m_gradationTex = D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/gradation.png");
	m_materialTex = D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/screenMaterial.png");
}

#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
void CanvasPostEffect::Execute()
{
	using namespace KuroEngine;
	DrawFunc2D::DrawGraph({ 0,0 }, m_gradationTex, 0.5f, 1.0f, AlphaBlendMode_Add);
	DrawFunc2D::DrawGraph({ 0,0 }, m_materialTex, 0.17f, 1.0f);
}
