#include "WaterPaintBlend.h"
#include"DirectX12/D3D12App.h"
#include"KuroEngineDevice.h"
#include"ForUser/DrawFunc/BillBoard/DrawFuncBillBoard.h"
#include"Render/RenderObject/Camera.h"

std::shared_ptr<KuroEngine::ComputePipeline>WaterPaintBlend::s_pipeline;

void WaterPaintBlend::GeneratePipeline()
{
	using namespace KuroEngine;

	//ルートパラメータ
	std::vector<RootParam>rootParam =
	{
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"元画像"),
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

	m_isDirty = CustomParamDirty();
}

WaterPaintBlend::WaterPaintBlend() : Debugger("WaterPaintBlend")
{
	using namespace KuroEngine;

	//パイプライン未生成なら生成
	if (!s_pipeline)GeneratePipeline();

	AddCustomParameter("split", { "Noise","split" }, PARAM_TYPE::INT_VEC2, &m_noiseInitializer.m_split, "Noise");
	AddCustomParameter("constrast", { "Noise","constrast" }, PARAM_TYPE::INT, &m_noiseInitializer.m_contrast, "Noise");
	AddCustomParameter("octave", { "Noise","octave" }, PARAM_TYPE::INT, &m_noiseInitializer.m_octave, "Noise");
	AddCustomParameter("frequency", { "Noise","frequency" }, PARAM_TYPE::FLOAT, &m_noiseInitializer.m_frequency, "Noise");
	AddCustomParameter("persistance", { "Noise","persistance" }, PARAM_TYPE::FLOAT, &m_noiseInitializer.m_persistance, "Noise");
	LoadParameterLog();

	//インクテクスチャ読み込み
	D3D12App::Instance()->GenerateTextureBuffer(
		m_inkTexArray.data(),
		"resource/user/tex/ink.png",
		INK_TEX_NUM,
		Vec2<int>(INK_TEX_NUM, 1));

	auto targetSize = D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize();

	//結果の描画先テクスチャ生成
	m_resultTex = D3D12App::Instance()->GenerateTextureBuffer(
		targetSize,
		D3D12App::Instance()->GetBackBuffFormat(),
		"WaterPaintBlend - ResultTex");

	//ノイズテクスチャ生成
	m_noiseTex = PerlinNoise::GenerateTex(
		"WaterPaintBlend_NoiseTex",
		targetSize,
		m_noiseInitializer);

	//マスクレイヤー生成
	m_maskLayer = D3D12App::Instance()->GenerateRenderTarget(
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		Color(0, 0, 0, 0),
		targetSize, L"MaskLayer");
}

void WaterPaintBlend::Init()
{
	m_maskInkArray.clear();
}

void WaterPaintBlend::DropMaskInk(KuroEngine::Vec3<float> arg_pos)
{
	m_maskInkArray.emplace_back();
	m_maskInkArray.back().m_pos = arg_pos;
	m_maskInkArray.back().m_texIdx = KuroEngine::GetRand(INK_TEX_NUM - 1);
}

void WaterPaintBlend::Register(std::shared_ptr<KuroEngine::TextureBuffer> arg_baseTex, KuroEngine::Camera& arg_cam, std::weak_ptr<KuroEngine::DepthStencil>arg_depthStencil)
{
	using namespace KuroEngine;

	//マスクレイヤークリア
	KuroEngineDevice::Instance()->Graphics().ClearRenderTarget(m_maskLayer);

	//マスクレイヤーにインク描画
	KuroEngineDevice::Instance()->Graphics().SetRenderTargets({ m_maskLayer }, arg_depthStencil);
	const float size = 7.0;
	for (auto& ink : m_maskInkArray)
	{
		DrawFuncBillBoard::Graph(
			arg_cam,
			ink.m_pos,
			{ size,size },
			m_inkTexArray[ink.m_texIdx],
			AlphaBlendMode_Trans);
	}

	if (m_isDirty)
	{
		auto targetSize = D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize();
		m_noiseTex.reset();
		m_noiseTex = PerlinNoise::GenerateTex(
			"WaterPaintBlend_NoiseTex",
			targetSize,
			m_noiseInitializer);

		m_isDirty = false;
	}

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
			{m_maskLayer,SRV},
			{m_noiseTex,SRV},
			{m_resultTex,UAV}
		});
}
