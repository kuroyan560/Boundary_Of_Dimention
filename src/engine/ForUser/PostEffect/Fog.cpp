#include "Fog.h"
#include"KuroEngineDevice.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"


void KuroEngine::Fog::OnImguiItems()
{
	if (CustomParamDirty())
	{

		m_config.m_distMin = std::max(0.0f, m_config.m_distMin);
		m_config.m_distMax = std::max(m_config.m_distMin, m_config.m_distMax);

		m_configBuffer->Mapping(&m_config);
		m_colorConfigBuffer->Mapping(&m_colorConfig);
	}
}

KuroEngine::Fog::Fog(Vec2<int>arg_size, DXGI_FORMAT arg_mainFormat) : Debugger("Fog")
{
	//カスタムパラメータ設定
	AddCustomParameter("intensity", { "Config","intensity" }, PARAM_TYPE::FLOAT, &m_config.m_intensity, "Config", true, 0.0f, 1.0f);
	AddCustomParameter("opacityMax", { "Config","opacityMax" }, PARAM_TYPE::FLOAT, &m_config.m_opacityMax, "Config", true, 0.0f, 1.0f);
	AddCustomParameter("distMax", { "Config","distMax" }, PARAM_TYPE::FLOAT, &m_config.m_distMax, "Config");
	AddCustomParameter("distMin", { "Config","distMin" }, PARAM_TYPE::FLOAT, &m_config.m_distMin, "Config");

	AddCustomParameter("useTex", { "Color","useTex" }, PARAM_TYPE::INT, &m_colorConfig.m_useTex, "Color", true, 0, 1);
	AddCustomParameter("fogColor_near", { "Color","fogColor","near"}, PARAM_TYPE::COLOR, &m_colorConfig.m_fogColorNear, "Color");
	AddCustomParameter("fogColor_far", { "Color","fogColor","far"}, PARAM_TYPE::COLOR, &m_colorConfig.m_fogColorFar, "Color");

	LoadParameterLog();

	//コンピュートパイプライン生成
	{
		std::vector<RootParam>rootParam =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"フォグの設定"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"フォグの色設定"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"フォグの色のテクスチャ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"メインレンダーターゲット"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"深度マップ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"マスク（フォグをかけない）"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"結果の描画先"),
		};
		auto cs = D3D12App::Instance()->CompileShader("resource/engine/Fog.hlsl", "CSmain", "cs_6_4");
		m_cPipeline = D3D12App::Instance()->GenerateComputePipeline(cs, rootParam, { WrappedSampler(true,true) });
	}

	//フォグ設定
	m_configBuffer = D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(Config), 1, &m_config, "Fog - Config - ConstantBuffer");

	//色に関する設定
	m_colorConfigBuffer = D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(ColorParameter), 1, &m_colorConfig, "Fog - ColorConfig - ConstantBuffer");

	//テクスチャ（デフォルトで 1x1 の白テクスチャをセットしとく）
	m_gradationTex = D3D12App::Instance()->GenerateTextureBuffer(Color(1.0f, 1.0f, 1.0f, 1.0f));

	//結果の描画先テクスチャ
	m_resultTex = D3D12App::Instance()->GenerateTextureBuffer(arg_size, arg_mainFormat, "Fog - ResultTex - TextureBuffer");
}

void KuroEngine::Fog::Register(const std::shared_ptr<TextureBuffer>& arg_main, const std::shared_ptr<TextureBuffer>& arg_depthMap, std::shared_ptr<TextureBuffer>arg_maskTex, bool arg_isGameScene)
{
	m_colorConfig.m_isGameScene = arg_isGameScene;
	m_colorConfigBuffer->Mapping(&m_colorConfig);

	std::vector<RegisterDescriptorData>descData =
	{
		{m_configBuffer,CBV},
		{m_colorConfigBuffer,CBV},
		{m_gradationTex,SRV},
		{arg_main,SRV},
		{arg_depthMap,SRV},
		{arg_maskTex,SRV},
		{m_resultTex,UAV},
	};

	Vec3<int>threadNum =
	{
		static_cast<int>(m_resultTex->GetGraphSize().x / THREAD_PER_NUM) + 1,
		static_cast<int>(m_resultTex->GetGraphSize().y / THREAD_PER_NUM) + 1,
		1
	};

	KuroEngineDevice::Instance()->Graphics().SetComputePipeline(m_cPipeline);
	KuroEngineDevice::Instance()->Graphics().Dispatch(threadNum, descData);
}

void KuroEngine::Fog::DrawResult(const AlphaBlendMode& arg_alphaBlend)
{
	KuroEngine::DrawFunc2D::DrawExtendGraph2D({ 0,0 }, KuroEngine::WinApp::Instance()->GetExpandWinSize(), m_resultTex, 1.0f, arg_alphaBlend);
}

void KuroEngine::Fog::AttachGradationTex(std::shared_ptr<TextureBuffer> arg_tex)
{
	m_gradationTex = arg_tex;
	m_colorConfig.m_useTex = 1;
	m_colorConfigBuffer->Mapping(&m_colorConfig);
}
