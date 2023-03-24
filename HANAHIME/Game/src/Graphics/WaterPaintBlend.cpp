#include "WaterPaintBlend.h"
#include"DirectX12/D3D12App.h"

WaterPaintBlend::WaterPaintBlend()
{
	using namespace KuroEngine;

	auto targetSize = D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize();

	//結果の描画先レンダーターゲット生成
	m_resultTex = D3D12App::Instance()->GenerateRenderTarget(
		D3D12App::Instance()->GetBackBuffFormat(),
		Color(0, 0, 0, 0), 
		targetSize, 
		L"WaterPaintBlend_Result");

	//ノイズ生成
	m_noiseTex = PerlinNoise::GenerateTex(
		"WaterPaintBlend_NoiseTex",
		targetSize,
		m_noiseInitializer);
}
