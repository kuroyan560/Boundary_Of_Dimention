#include "WaterPaintBlend.h"
#include"DirectX12/D3D12App.h"
#include"KuroEngineDevice.h"

std::shared_ptr<KuroEngine::ComputePipeline>WaterPaintBlend::s_pipeline;

void WaterPaintBlend::GeneratePipeline()
{
	using namespace KuroEngine;

	//ルートパラメータ
	std::vector<RootParam>rootParam =
	{
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"元画像"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"ブレンドする画像"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"マスク画像"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"ノイズテクスチャ"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"描き込み先バッファ"),
	};

	//シェーダーコンパイル
	auto cs = D3D12App::Instance()->CompileShader("resource/user/shaders/WaterPaintBlend.hlsl", "CSmain", "cs_6_4");

	//パイプライン生成
	s_pipeline = D3D12App::Instance()->GenerateComputePipeline(cs, rootParam, { WrappedSampler(true,true) });
}

void WaterPaintBlend::OnImguiItems()
{
	using namespace KuroEngine;

	if (CustomParamDirty())
	{
		auto targetSize = D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize();

		m_noiseTex.reset();
		m_noiseTex = PerlinNoise::GenerateTex(
			"WaterPaintBlend_NoiseTex",
			targetSize,
			m_noiseInitializer);
	}
}

WaterPaintBlend::WaterPaintBlend() : Debugger("WaterPaintBlend")
{
	using namespace KuroEngine;

	//パイプライン未生成なら生成
	if (!s_pipeline)GeneratePipeline();

	auto targetSize = D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize();

	//結果の描画先テクスチャ生成
	m_resultTex = D3D12App::Instance()->GenerateTextureBuffer(
		targetSize,
		D3D12App::Instance()->GetBackBuffFormat(),
		"WaterPaintBlend - ResultTex");

	//ノイズ生成
	m_noiseTex = PerlinNoise::GenerateTex(
		"WaterPaintBlend_NoiseTex",
		targetSize,
		m_noiseInitializer);

	AddCustomParameter("split", { "Noise","split" }, PARAM_TYPE::INT_VEC2, &m_noiseInitializer.m_split, "Noise");
	AddCustomParameter("constrast", { "Noise","constrast" }, PARAM_TYPE::INT, &m_noiseInitializer.m_contrast, "Noise");
	AddCustomParameter("octave", { "Noise","octave" }, PARAM_TYPE::INT, &m_noiseInitializer.m_octave, "Noise");
	AddCustomParameter("frequency", { "Noise","frequency" }, PARAM_TYPE::FLOAT, &m_noiseInitializer.m_frequency, "Noise");
	AddCustomParameter("persistance", { "Noise","persistance" }, PARAM_TYPE::FLOAT, &m_noiseInitializer.m_persistance, "Noise");
}

void WaterPaintBlend::Register(std::shared_ptr<KuroEngine::TextureBuffer> arg_baseTex, std::shared_ptr<KuroEngine::TextureBuffer> arg_blendTex, std::shared_ptr<KuroEngine::TextureBuffer> arg_maskTex)
{
	using namespace KuroEngine;

	KuroEngineDevice::Instance()->Graphics().SetComputePipeline(s_pipeline);

	Vec3<int>threadNum =
	{
		static_cast<int>(ceil(m_resultTex->GetGraphSize().x / THREAD_PER_NUM) + 1),
		static_cast<int>(ceil(m_resultTex->GetGraphSize().y / THREAD_PER_NUM) + 1),
		1
	};

	KuroEngineDevice::Instance()->Graphics().Dispatch(
		threadNum,
		{
			{arg_baseTex,SRV},
			{arg_blendTex,SRV},
			{arg_maskTex,SRV},
			{m_noiseTex,SRV},
			{m_resultTex,UAV}
		});
}
