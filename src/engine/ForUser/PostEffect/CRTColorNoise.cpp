#include "CRTColorNoise.h"
#include"KuroEngineDevice.h"
#include"Render/RenderObject/SpriteMesh.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"

void KuroEngine::CRTColorNoise::OnImguiItems()
{
	ImGui::ColorPicker4("OverlayColor", (float*)&m_crtInfo.m_overlayColor);
	ImGui::DragFloat("OverlayThresholdBright", &m_crtInfo.m_overlayThreshold, 0.02f, 0.0f, 1.0f);
	ImGui::DragFloat("ColorSaturationRate", &m_crtInfo.m_saturationRate, 0.2f);
}

KuroEngine::CRTColorNoise::CRTColorNoise() : Debugger("CRTColorNoise")
{
	m_resultTex = D3D12App::Instance()->GenerateRenderTarget(D3D12App::Instance()->GetBackBuffFormat(),
		Color(0, 0, 0, 0), D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize(), L"CRT_Result");
	m_crtInfoBuff = D3D12App::Instance()->GenerateConstantBuffer(sizeof(Info), 1, &m_crtInfo, "CRTColorNoise - Info");
	m_spriteMesh = std::make_shared<SpriteMesh>("CRTColorNoise - Mesh");
	m_spriteMesh->SetSize(KuroEngine::WinApp::Instance()->GetExpandWinSize());
}

void KuroEngine::CRTColorNoise::Register(const std::shared_ptr<TextureBuffer>& arg_srcTex)
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
		SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/engine/CRTColorNoise.hlsl", "VSmain", "vs_6_4");
		SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/engine/CRTColorNoise.hlsl", "PSmain", "ps_6_4");

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
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"CRT情報")
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(m_resultTex->GetDesc().Format, AlphaBlendMode_None)};
		//パイプライン生成
		PIPELINE = D3D12App::Instance()->GenerateGraphicsPipeline(
			PIPELINE_OPTION,
			SHADERS,
			INPUT_LAYOUT,
			ROOT_PARAMETER,
			RENDER_TARGET_INFO,
			{ WrappedSampler(false, false) });
	}

	m_crtInfo.m_screenSize = m_resultTex->GetGraphSize().Float();
	m_crtInfoBuff->Mapping(&m_crtInfo);

	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetRenderTargets({ m_resultTex });
	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(PIPELINE);
	m_spriteMesh->Render(
		{ { KuroEngine::KuroEngineDevice::Instance()->GetParallelMatProjBuff(),CBV },
		{ arg_srcTex,SRV },
		{ m_crtInfoBuff,CBV } });
}

void KuroEngine::CRTColorNoise::DrawResult(const AlphaBlendMode& arg_alphaBlend)
{
	KuroEngine::DrawFunc2D::DrawExtendGraph2D({ 0,0 }, KuroEngine::WinApp::Instance()->GetExpandWinSize(), m_resultTex, 1.0f, arg_alphaBlend);
}
