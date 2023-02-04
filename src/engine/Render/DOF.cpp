#include "DOF.h"
#include"KuroEngineDevice.h"
#include"ForUser/PostEffect/GaussianBlur.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"

std::shared_ptr<KuroEngine::ComputePipeline>KuroEngine::DOF::s_pipeline;

void KuroEngine::DOF::GeneratePipeline()
{
	std::vector<RootParam>rootParam =
	{
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"DOF設定"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"ブラーをかけたテクスチャ"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"深度マップ"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"描き込み先バッファ")
	};

	auto cs = D3D12App::Instance()->CompileShader("resource/engine/DOF.hlsl", "CSmain", "cs_6_4");
	s_pipeline = D3D12App::Instance()->GenerateComputePipeline(cs, rootParam, { WrappedSampler(false, true) });
}

KuroEngine::DOF::DOF()
{
	//パイプライン未生成なら生成
	if (!s_pipeline)GeneratePipeline();

	//ウィンドウサイズの取得
	const auto winSize = KuroEngine::WinApp::Instance()->GetExpandWinSize();

	//バックバッファの取得
	auto backBuffFormat = D3D12App::Instance()->GetBackBuffFormat();

	//DOF設定
	m_configBuff = D3D12App::Instance()->GenerateConstantBuffer(sizeof(DOFConfig), 1, &m_config, "DOF - Config - ConstantBuffer");

	//ガウシアンブラー生成
	m_gaussianBlur = std::make_shared<GaussianBlur>(winSize.Int(), backBuffFormat);

	//深度マップをもとに生成した透過ボケ画像の格納先
	m_processedTex = D3D12App::Instance()->GenerateTextureBuffer(winSize.Int(), backBuffFormat, "DOF - ProcessedTex");
}

void KuroEngine::DOF::SetPintConfig(float FrontPint, float BackPint, float PintLength)
{
	m_config.m_nearPint = FrontPint;
	m_config.m_farPint = BackPint;
	m_config.m_pintLength = PintLength;
	m_configBuff->Mapping(&m_config);
}

void KuroEngine::DOF::Draw(std::weak_ptr<RenderTarget> Src, std::weak_ptr<RenderTarget> DepthMap)
{
	m_gaussianBlur->Register(Src.lock());

	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetComputePipeline(s_pipeline);

	static const int DIV = 32;
	Vec3<int>threadNum = 
	{ 
		static_cast<int>(ceil(m_processedTex->GetGraphSize().x / DIV)),
		static_cast<int>(ceil(m_processedTex->GetGraphSize().y / DIV)),
		1 
	};

	KuroEngine::KuroEngineDevice::Instance()->Graphics().Dispatch(threadNum,
		{
			{m_configBuff,CBV},
			{m_gaussianBlur->GetResultTex(),SRV},
			{DepthMap.lock(),SRV},
			{m_processedTex,UAV}
		});

	KuroEngine::DrawFunc2D::DrawGraph({ 0,0 }, m_processedTex);
}
