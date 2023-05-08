#include "DrawFuncBillBoard.h"
#include"KuroEngineDevice.h"
#include"Render/RenderObject/Camera.h"

//DrawBox
int KuroEngine::DrawFuncBillBoard::s_drawBoxCount = 0;
//DrawGraph
int KuroEngine::DrawFuncBillBoard::s_drawGraphCount = 0;

std::map<DXGI_FORMAT, std::array<std::shared_ptr<KuroEngine::GraphicsPipeline>, KuroEngine::AlphaBlendModeNum>>KuroEngine::DrawFuncBillBoard::s_pipeline;
std::vector<std::shared_ptr<KuroEngine::VertexBuffer>>KuroEngine::DrawFuncBillBoard::s_graphVertBuff;

void KuroEngine::DrawFuncBillBoard::GeneratePipeline(DXGI_FORMAT Format,const AlphaBlendMode& BlendMode)
{
	//パイプライン生成済
	if (s_pipeline.find(Format) != s_pipeline.end() && s_pipeline[Format][BlendMode])return;

	//パイプライン設定
	static PipelineInitializeOption s_pipelineOption(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	s_pipelineOption.m_calling = D3D12_CULL_MODE_NONE;
	s_pipelineOption.m_depthWriteMask = false;

	//シェーダー情報
	static Shaders s_shaders;
	s_shaders.m_vs = D3D12App::Instance()->CompileShader("resource/engine/DrawBillBoard.hlsl", "VSmain", "vs_6_4");
	s_shaders.m_gs = D3D12App::Instance()->CompileShader("resource/engine/DrawBillBoard.hlsl", "GSmain", "gs_6_4");
	s_shaders.m_ps = D3D12App::Instance()->CompileShader("resource/engine/DrawBillBoard.hlsl", "PSmain", "ps_6_4");

	//インプットレイアウト
	static std::vector<InputLayoutParam>s_inputLayOut =
	{
		InputLayoutParam("POS",DXGI_FORMAT_R32G32B32_FLOAT),
		InputLayoutParam("SIZE",DXGI_FORMAT_R32G32_FLOAT),
		InputLayoutParam("COLOR",DXGI_FORMAT_R32G32B32A32_FLOAT),
	};

	//ルートパラメータ
	static std::vector<RootParam>s_rootParams =
	{
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"カメラ情報バッファ"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"テクスチャ"),
	};

	//レンダーターゲット描画先情報
	std::vector<RenderTargetInfo>s_renderTargetInfo = { RenderTargetInfo(Format, BlendMode) };
	//パイプライン生成
	s_pipeline[Format][BlendMode] = D3D12App::Instance()->GenerateGraphicsPipeline(s_pipelineOption, s_shaders, s_inputLayOut, s_rootParams, s_renderTargetInfo, { WrappedSampler(false, false) });
}

void KuroEngine::DrawFuncBillBoard::PrepareGraphVertBuff(int arg_num)
{
	for (int i = 0; i < arg_num; ++i)
	{
		s_graphVertBuff.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(Vertex), 1, nullptr, ("DrawGraphBillBoard -" + std::to_string(s_drawGraphCount)).c_str()));
	}
}

void KuroEngine::DrawFuncBillBoard::Box(Camera& Cam, const Vec3<float>& Pos, const Vec2<float>& Size, const Color& BoxColor, const AlphaBlendMode& BlendMode, const int& arg_layer)
{
	static std::vector<std::shared_ptr<VertexBuffer>>s_boxVertBuff;
	static std::shared_ptr<TextureBuffer>s_defaultTex = D3D12App::Instance()->GenerateTextureBuffer(Color());

	const auto targetFormat = KuroEngine::KuroEngineDevice::Instance()->Graphics().GetRecentRenderTargetFormat(0);
	GeneratePipeline(targetFormat, BlendMode);

	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(s_pipeline[targetFormat][BlendMode]);

	if (s_boxVertBuff.size() < (s_drawBoxCount + 1))
	{
		s_boxVertBuff.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(Vertex), 1, nullptr, ("DrawBoxBillBoard -" + std::to_string(s_drawBoxCount)).c_str()));
	}

	Vertex vertex(Pos, Size, BoxColor);
	s_boxVertBuff[s_drawBoxCount]->Mapping(&vertex);

	KuroEngine::KuroEngineDevice::Instance()->Graphics().ObjectRender(
		s_boxVertBuff[s_drawBoxCount],
		{
			{Cam.GetBuff(),CBV},
			{s_defaultTex,SRV}
		},
		arg_layer,
		true);

	s_drawBoxCount++;
}

void KuroEngine::DrawFuncBillBoard::Graph(Camera& Cam, const Vec3<float>& Pos, const Vec2<float>& Size, std::shared_ptr<TextureBuffer> Tex, const AlphaBlendMode& BlendMode, const int& arg_layer)
{
	const auto targetFormat = KuroEngine::KuroEngineDevice::Instance()->Graphics().GetRecentRenderTargetFormat(0);
	GeneratePipeline(targetFormat, BlendMode);

	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(s_pipeline[targetFormat][BlendMode]);

	if (s_graphVertBuff.size() < (s_drawGraphCount + 1))
	{
		s_graphVertBuff.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(Vertex), 1, nullptr, ("DrawGraphBillBoard -" + std::to_string(s_drawGraphCount)).c_str()));
	}

	Vertex vertex(Pos, Size, Color());
	s_graphVertBuff[s_drawGraphCount]->Mapping(&vertex);

	KuroEngine::KuroEngineDevice::Instance()->Graphics().ObjectRender(
		s_graphVertBuff[s_drawGraphCount],
		{
			{Cam.GetBuff(),CBV},
			{Tex,SRV}
		},
		arg_layer,
		true);

	s_drawGraphCount++;
}

void KuroEngine::DrawFuncBillBoard::Graph(Camera &Cam, const Vec3<float> &Pos, const Vec2<float> &Size, std::shared_ptr<TextureBuffer> Tex, float alpha, const AlphaBlendMode &BlendMode)
{
	const auto targetFormat = KuroEngine::KuroEngineDevice::Instance()->Graphics().GetRecentRenderTargetFormat(0);
	GeneratePipeline(targetFormat, BlendMode);

	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(s_pipeline[targetFormat][BlendMode]);

	if (s_graphVertBuff.size() < (s_drawGraphCount + 1))
	{
		s_graphVertBuff.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(Vertex), 1, nullptr, ("DrawGraphBillBoard -" + std::to_string(s_drawGraphCount)).c_str()));
	}

	Vertex vertex(Pos, Size, Color(1.0f, 1.0f, 1.0f, alpha));
	s_graphVertBuff[s_drawGraphCount]->Mapping(&vertex);

	KuroEngine::KuroEngineDevice::Instance()->Graphics().ObjectRender(
		s_graphVertBuff[s_drawGraphCount],
		{
			{Cam.GetBuff(),CBV},
			{Tex,SRV}
		},
		static_cast<int>(Pos.z),
		true);

	s_drawGraphCount++;
}
