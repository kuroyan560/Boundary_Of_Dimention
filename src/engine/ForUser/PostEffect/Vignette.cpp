#include "Vignette.h"
#include"KuroEngineDevice.h"
#include"Render/RenderObject/SpriteMesh.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"

void KuroEngine::Vignette::OnImguiItems()
{
	if (CustomParamDirty())
	{
		m_vignetteInfo.m_intensity = std::max(m_vignetteInfo.m_intensity, 0.0f);
		m_vignetteInfo.m_smoothness = std::max(m_vignetteInfo.m_smoothness, 0.0f);
		m_vignetteInfoBuff->Mapping(&m_vignetteInfo);
	}
}

KuroEngine::Vignette::Vignette() : Debugger("Vignette")
{
	AddCustomParameter("intensity", { "intensity" }, PARAM_TYPE::FLOAT, &m_vignetteInfo.m_intensity, "Info");
	AddCustomParameter("smoothness", { "smoothness" }, PARAM_TYPE::FLOAT, &m_vignetteInfo.m_smoothness, "Info");
	AddCustomParameter("center", { "center" }, PARAM_TYPE::FLOAT_VEC2, &m_vignetteInfo.m_center, "Info", true, 0.0f, 1.0f);
	AddCustomParameter("color", { "color" }, PARAM_TYPE::COLOR, &m_vignetteInfo.m_color, "Info");
	LoadParameterLog();

	m_resultTex = D3D12App::Instance()->GenerateRenderTarget(D3D12App::Instance()->GetBackBuffFormat(),
		Color(0, 0, 0, 0), D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize(), L"Vignette_Result");
	m_vignetteInfoBuff = D3D12App::Instance()->GenerateConstantBuffer(sizeof(Info), 1, &m_vignetteInfo, "Vignette - Info");
	m_spriteMesh = std::make_shared<SpriteMesh>("Vignette - Mesh");
	m_spriteMesh->SetSize(KuroEngine::WinApp::Instance()->GetExpandWinSize());
}

void KuroEngine::Vignette::Register(const std::shared_ptr<TextureBuffer>& arg_srcTex)
{
	static std::shared_ptr<GraphicsPipeline>PIPELINE;
	//パイプライン未生成
	if (!PIPELINE)
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		PIPELINE_OPTION.m_depthTest = false;
		PIPELINE_OPTION.m_depthWriteMask = false;

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/engine/Vignette.hlsl", "VSmain", "vs_6_4");
		SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/engine/Vignette.hlsl", "PSmain", "ps_6_4");

		//インプットレイアウト
		static std::vector<InputLayoutParam>INPUT_LAYOUT =
		{
			InputLayoutParam("POSITION",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("TEXCOORD",DXGI_FORMAT_R32G32_FLOAT)
		};

		//ルートパラメータ
		static std::vector<RootParam>ROOT_PARAMETER =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"平行投影行列定数バッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"ソース画像バッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"Vignette情報")
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(m_resultTex->GetDesc().Format, AlphaBlendMode_None) };
		//パイプライン生成
		PIPELINE = D3D12App::Instance()->GenerateGraphicsPipeline(
			PIPELINE_OPTION,
			SHADERS,
			INPUT_LAYOUT,
			ROOT_PARAMETER,
			RENDER_TARGET_INFO,
			{ WrappedSampler(false, false) });
	}

	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetRenderTargets({ m_resultTex });
	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(PIPELINE);
	m_spriteMesh->Render(
		{ { KuroEngine::KuroEngineDevice::Instance()->GetParallelMatProjBuff(),CBV },
		{ arg_srcTex,SRV },
		{ m_vignetteInfoBuff,CBV },
		});
}

void KuroEngine::Vignette::DrawResult(const AlphaBlendMode& arg_alphaBlend)
{
	KuroEngine::DrawFunc2D::DrawExtendGraph2D({ 0,0 }, KuroEngine::WinApp::Instance()->GetExpandWinSize(), m_resultTex, 1.0f, arg_alphaBlend);
}

void KuroEngine::Vignette::SetInfo(const Info& arg_info)
{
	m_vignetteInfo = arg_info;
	m_vignetteInfoBuff->Mapping(&m_vignetteInfo);
}
