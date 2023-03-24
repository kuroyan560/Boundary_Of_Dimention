#include "WaterPaintBlend.h"
#include"DirectX12/D3D12App.h"

WaterPaintBlend::WaterPaintBlend()
{
	using namespace KuroEngine;

	auto targetSize = D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize();

	//���ʂ̕`��惌���_�[�^�[�Q�b�g����
	m_resultTex = D3D12App::Instance()->GenerateRenderTarget(
		D3D12App::Instance()->GetBackBuffFormat(),
		Color(0, 0, 0, 0), 
		targetSize, 
		L"WaterPaintBlend_Result");

	//�m�C�Y����
	m_noiseTex = PerlinNoise::GenerateTex(
		"WaterPaintBlend_NoiseTex",
		targetSize,
		m_noiseInitializer);
}
