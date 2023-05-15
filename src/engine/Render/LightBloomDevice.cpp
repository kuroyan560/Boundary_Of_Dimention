#include "LightBloomDevice.h"
#include"KuroEngineDevice.h"
#include"ForUser/PostEffect/GaussianBlur.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"

std::shared_ptr<KuroEngine::ComputePipeline>KuroEngine::LightBloomDevice::s_filterPipeline;
std::shared_ptr<KuroEngine::ComputePipeline>KuroEngine::LightBloomDevice::s_combinePipeline;

void KuroEngine::LightBloomDevice::GeneratePipeline()
{
	{
		//シェーダー
		auto cs = D3D12App::Instance()->CompileShader("resource/engine/LightBloom.hlsl", "Filter", "cs_6_4");
		//ルートパラメータ
		std::vector<RootParam>rootParams =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"ライトブルーム設定"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"エミッシブマップ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"加工後エミッシブマップ"),
		};
		//パイプライン生成
		s_filterPipeline = D3D12App::Instance()->GenerateComputePipeline(cs, rootParams, { WrappedSampler(false,true) });
	}
	{
		//シェーダー
		auto cs = D3D12App::Instance()->CompileShader("resource/engine/LightBloom.hlsl", "Combine", "cs_6_4");
		//ルートパラメータ
		std::vector<RootParam>rootParams;
		for (int i = 0; i < BLUR_COUNT; ++i)
			rootParams.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "ボケ画像");
		rootParams.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, "加工後エミッシブマップ");
		rootParams.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "ライトブルーム設定"),

			//パイプライン生成
			s_combinePipeline = D3D12App::Instance()->GenerateComputePipeline(cs, rootParams, { WrappedSampler(false,true) });
	}
}

KuroEngine::LightBloomDevice::LightBloomDevice()
{
	//パイプライン生成
	if (!s_filterPipeline)GeneratePipeline();

	//ウィンドウサイズ取得
	const auto winSize = KuroEngine::WinApp::Instance()->GetExpandWinSize().Int();

	//エミッシブマップ
	m_processedEmissiveMap = D3D12App::Instance()->GenerateTextureBuffer(winSize, DXGI_FORMAT_R32G32B32A32_FLOAT, "LightBloom - EmissiveMap - Processed");

	//定数バッファ生成
	m_config.m_imgSize = winSize;
	m_constBuff = D3D12App::Instance()->GenerateConstantBuffer(sizeof(LightBloomConfig), 1, &m_config, "LgihtBloom - Config - ConstantBuffer");

	//ガウシアンブラー
	for (int i = 0; i < BLUR_COUNT; ++i)
		m_gaussianBlurs[i] = std::make_shared<GaussianBlur>(winSize, DXGI_FORMAT_R32G32B32A32_FLOAT);
}

void KuroEngine::LightBloomDevice::Draw(std::weak_ptr<RenderTarget>EmissiveMap, std::weak_ptr<RenderTarget>Target)
{
	auto emissiveMap = EmissiveMap.lock();

	//エミッシブマップをしきい値などに応じて加工
	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetComputePipeline(s_filterPipeline);
	Vec3<int>threadNum = { emissiveMap->GetGraphSize().x / THREAD_DIV + 1,emissiveMap->GetGraphSize().y / THREAD_DIV + 1,1 };

	KuroEngine::KuroEngineDevice::Instance()->Graphics().Dispatch(threadNum,
		{
			{m_constBuff,CBV},
			{emissiveMap,SRV,D3D12_RESOURCE_STATE_COMMON},
			{m_processedEmissiveMap,UAV}
		});

	//エミッシブマップにブラーをかける
	m_gaussianBlurs[0]->Register(m_processedEmissiveMap);
	for (int i = 1; i < BLUR_COUNT; ++i)m_gaussianBlurs[i]->Register(m_gaussianBlurs[i - 1]->GetResultTex());

	//ブラー合成
	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetComputePipeline(s_combinePipeline);
	std::vector<RegisterDescriptorData>descDatas;
	for (int i = 0; i < BLUR_COUNT; ++i)
		descDatas.emplace_back(m_gaussianBlurs[i]->GetResultTex(), SRV, D3D12_RESOURCE_STATE_COMMON);
	descDatas.emplace_back(m_processedEmissiveMap, UAV);
	KuroEngine::KuroEngineDevice::Instance()->Graphics().Dispatch(threadNum, descDatas);

	//結果を描画
	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetRenderTargets({ Target.lock() });
	KuroEngine::DrawFunc2D::DrawGraph({ 0,0 }, m_processedEmissiveMap, 1.0f, 1.0f, AlphaBlendMode_Add);
}

void KuroEngine::LightBloomDevice::SetOutputColorRate(const Vec3<float>& RGBrate)
{
	m_config.m_outputColorRate = RGBrate;
	m_constBuff->Mapping(&m_config);
}

void KuroEngine::LightBloomDevice::SetBrightThreshold(const float& Threshold)
{
	m_config.m_brightThreshold = Threshold;
	m_constBuff->Mapping(&m_config);
}

void KuroEngine::LightBloomDevice::SetBlurPower(const float& BlurPower)
{
	for (int i = 0; i < BLUR_COUNT; ++i)
		m_gaussianBlurs[i]->SetBlurPower(BlurPower);
}
