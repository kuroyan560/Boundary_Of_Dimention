#include "DrawFunc2D.h"
#include"KuroEngineDevice.h"
#include<map>

//DrawLine
int KuroEngine::DrawFunc2D::s_DrawLineCount = 0;

//DrawBox
int KuroEngine::DrawFunc2D::s_DrawBoxCount = 0;

//DrawCircle
int KuroEngine::DrawFunc2D::s_DrawCircleCount = 0;

//DrawExtendGraph
int KuroEngine::DrawFunc2D::s_DrawExtendGraphCount = 0;

//DrawRotaGraph
int KuroEngine::DrawFunc2D::s_DrawRotaGraphCount = 0;

//DrawRadialWipeGraph
int KuroEngine::DrawFunc2D::s_DrawRadialWipeGraphCount = 0;

void KuroEngine::DrawFunc2D::DrawLine2D(const Vec2<float>& FromPos, const Vec2<float>& ToPos, const Color& LineColor, const AlphaBlendMode& BlendMode)
{
	static std::shared_ptr<GraphicsPipeline>LINE_PIPELINE[AlphaBlendModeNum];
	static std::vector<std::shared_ptr<VertexBuffer>>LINE_VERTEX_BUFF;

	//DrawLine専用頂点
	class LineVertex
	{
	public:
		Vec2<float>pos;
		Color color;
		LineVertex(const Vec2<float>& Pos, const Color& Color) :pos(Pos), color(Color) {}
	};

	//パイプライン未生成
	if(!LINE_PIPELINE[BlendMode])
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);
		PIPELINE_OPTION.m_depthTest = false;

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/engine/DrawLine2D.hlsl", "VSmain", "vs_6_4");
		SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/engine/DrawLine2D.hlsl", "PSmain", "ps_6_4");

		//インプットレイアウト
		static std::vector<InputLayoutParam>INPUT_LAYOUT =
		{
			InputLayoutParam("POSITION",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("COLOR",DXGI_FORMAT_R32G32B32A32_FLOAT)
		};

		//ルートパラメータ
		static std::vector<RootParam>ROOT_PARAMETER =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"平行投影行列定数バッファ")
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), BlendMode) };
		//パイプライン生成
		LINE_PIPELINE[BlendMode] = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, INPUT_LAYOUT, ROOT_PARAMETER, RENDER_TARGET_INFO, { WrappedSampler(false, false) });
	}

	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(LINE_PIPELINE[BlendMode]);

	if (LINE_VERTEX_BUFF.size() < (s_DrawLineCount + 1))
	{
		LINE_VERTEX_BUFF.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(LineVertex), 2, nullptr, ("DrawLine2D -" + std::to_string(s_DrawLineCount)).c_str()));
	}

	LineVertex vertex[2] =
	{
		{FromPos,LineColor },
		{ToPos,LineColor}
	};
	LINE_VERTEX_BUFF[s_DrawLineCount]->Mapping(&vertex[0]);

	KuroEngine::KuroEngineDevice::Instance()->Graphics().ObjectRender(LINE_VERTEX_BUFF[s_DrawLineCount], { {KuroEngine::KuroEngineDevice::Instance()->GetParallelMatProjBuff(), CBV} }, 0, true);

	s_DrawLineCount++;
}

void KuroEngine::DrawFunc2D::DrawLine2DGraph(const Vec2<float>& FromPos, const Vec2<float>& ToPos, const std::shared_ptr<TextureBuffer>& Tex, const int& Thickness, const float& Alpha, const AlphaBlendMode& BlendMode, const Vec2<bool>& Mirror)
{
	float distance = FromPos.Distance(ToPos);
	Vec2<float> vec = (ToPos - FromPos).GetNormal();

	auto graphSize = Tex->GetGraphSize().Float();
	Vec2<float>expRate = { distance / graphSize.x,Thickness / graphSize.y };
	Vec2<float>centerPos = FromPos + vec * distance / 2;

	DrawRotaGraph2D(centerPos, expRate, KuroEngine::Math::GetAngle(vec), Tex, Alpha, { 0.5f,0.5f }, BlendMode, Mirror);
}


void KuroEngine::DrawFunc2D::DrawBox2D(const Vec2<float>& LeftUpPos, const Vec2<float>& RightBottomPos, const Color& BoxColor, const bool& FillFlg, const AlphaBlendMode& BlendMode)
{
	if (FillFlg)
	{
		static std::shared_ptr<GraphicsPipeline>BOX_PIPELINE[AlphaBlendModeNum];
		static std::vector<std::shared_ptr<VertexBuffer>>BOX_VERTEX_BUFF;

		//DrawBox専用頂点
		class BoxVertex
		{
		public:
			Vec2<float>leftUpPos;
			Vec2<float>rightBottomPos;
			Color color;
			BoxVertex(const Vec2<float>& LeftUpPos, const Vec2<float>& RightBottomPos, const Color& Color)
				:leftUpPos(LeftUpPos), rightBottomPos(RightBottomPos), color(Color) {}
		};

		//パイプライン未精製
		if (!BOX_PIPELINE[BlendMode])
		{
			//パイプライン設定
			static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
			PIPELINE_OPTION.m_depthTest = false;
			PIPELINE_OPTION.m_calling = D3D12_CULL_MODE_NONE;
			//シェーダー情報
			static Shaders SHADERS;
			SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/engine/DrawBox2D.hlsl", "VSmain", "vs_6_4");
			SHADERS.m_gs = D3D12App::Instance()->CompileShader("resource/engine/DrawBox2D.hlsl", "GSmain", "gs_6_4");
			SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/engine/DrawBox2D.hlsl", "PSmain", "ps_6_4");

			//インプットレイアウト
			static std::vector<InputLayoutParam>INPUT_LAYOUT =
			{
				InputLayoutParam("POSITION_L_U",DXGI_FORMAT_R32G32_FLOAT),
				InputLayoutParam("POSITION_R_B",DXGI_FORMAT_R32G32_FLOAT),
				InputLayoutParam("COLOR",DXGI_FORMAT_R32G32B32A32_FLOAT)
			};

			//ルートパラメータ
			static std::vector<RootParam>ROOT_PARAMETER =
			{
				RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"平行投影行列定数バッファ")
			};

			//レンダーターゲット描画先情報
			std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), BlendMode) };
			//パイプライン生成
			BOX_PIPELINE[BlendMode] = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, INPUT_LAYOUT, ROOT_PARAMETER, RENDER_TARGET_INFO, { WrappedSampler(false, false) });
		}

		KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(BOX_PIPELINE[BlendMode]);

		if (BOX_VERTEX_BUFF.size() < (s_DrawBoxCount + 1))
		{
			BOX_VERTEX_BUFF.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(BoxVertex), 1, nullptr, ("DrawBox2D -" + std::to_string(s_DrawBoxCount)).c_str()));
		}

		BoxVertex vertex(LeftUpPos, RightBottomPos, BoxColor);
		BOX_VERTEX_BUFF[s_DrawBoxCount]->Mapping(&vertex);

		KuroEngine::KuroEngineDevice::Instance()->Graphics().ObjectRender(BOX_VERTEX_BUFF[s_DrawBoxCount],
			{ {KuroEngine::KuroEngineDevice::Instance()->GetParallelMatProjBuff(),CBV} }, 0, true);

		s_DrawBoxCount++;
	}
	//外枠だけのときはDrawLineで済ます
	else
	{
		const float width = RightBottomPos.x - LeftUpPos.x;
		Vec2<float>pos[4] =
		{
			LeftUpPos,
			{LeftUpPos.x + width,LeftUpPos.y},
			RightBottomPos,
			{RightBottomPos.x - width,RightBottomPos.y}
		};
		for (int i = 0; i < 4; ++i)
		{
			int next = 4 <= (i + 1) ? 0 : i + 1;
			DrawLine2D(pos[i], pos[next], BoxColor, BlendMode);
		}
	}
}

void KuroEngine::DrawFunc2D::DrawCircle2D(const Vec2<float>& Center, const float& Radius, const Color& CircleColor, const bool& FillFlg, const int& LineThickness, const AlphaBlendMode& BlendMode)
{
	static std::shared_ptr<GraphicsPipeline>CIRCLE_PIPELINE[AlphaBlendModeNum];
	static std::vector<std::shared_ptr<VertexBuffer>>CIRCLE_VERTEX_BUFF;

	//DrawCircle専用頂点
	class CircleVertex
	{
	public:
		Vec2<float>center;
		float radius;
		Color color;
		unsigned int fillFlg;
		int thickness;

		CircleVertex(const Vec2<float>& Center, const float& Radius, const Color& Color, const bool& FillFlg, const int& Thickness)
			:center(Center), radius(Radius), color(Color), fillFlg(FillFlg ? 0 : 1), thickness(Thickness) {}
	};

	//パイプライン未生成
	if (!CIRCLE_PIPELINE[BlendMode])
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		PIPELINE_OPTION.m_depthTest = false;

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/engine/DrawCircle2D.hlsl", "VSmain", "vs_6_4");
		SHADERS.m_gs = D3D12App::Instance()->CompileShader("resource/engine/DrawCircle2D.hlsl", "GSmain", "gs_6_4");
		SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/engine/DrawCircle2D.hlsl", "PSmain", "ps_6_4");

		//インプットレイアウト
		static std::vector<InputLayoutParam>INPUT_LAYOUT =
		{
			InputLayoutParam("CENTER",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("RADIUS",DXGI_FORMAT_R32_FLOAT),
			InputLayoutParam("COLOR",DXGI_FORMAT_R32G32B32A32_FLOAT),
			InputLayoutParam("FILL",DXGI_FORMAT_R32_UINT),
			InputLayoutParam("THICKNESS",DXGI_FORMAT_R32_SINT),
		};

		//ルートパラメータ
		static std::vector<RootParam>ROOT_PARAMETER =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"平行投影行列定数バッファ")
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), BlendMode) };
		//パイプライン生成
		CIRCLE_PIPELINE[BlendMode] = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, INPUT_LAYOUT, ROOT_PARAMETER, RENDER_TARGET_INFO, { WrappedSampler(false, false) });
	}

	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(CIRCLE_PIPELINE[BlendMode]);

	if (CIRCLE_VERTEX_BUFF.size() < (s_DrawCircleCount + 1))
	{
		CIRCLE_VERTEX_BUFF.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(CircleVertex), 1, nullptr, ("DrawCircle2D -" + std::to_string(s_DrawCircleCount)).c_str()));
	}

	CircleVertex vertex(Center, Radius, CircleColor, FillFlg, LineThickness);

	CIRCLE_VERTEX_BUFF[s_DrawCircleCount]->Mapping(&vertex);

	KuroEngine::KuroEngineDevice::Instance()->Graphics().ObjectRender(CIRCLE_VERTEX_BUFF[s_DrawCircleCount], { {KuroEngine::KuroEngineDevice::Instance()->GetParallelMatProjBuff(),CBV} }, 0, true);

	s_DrawCircleCount++;
}

void KuroEngine::DrawFunc2D::DrawGraph(const Vec2<float>& LeftUpPos, const std::shared_ptr<TextureBuffer>& Tex, const float& Alpha, const float& Scale, const AlphaBlendMode& BlendMode, const Vec2<bool>& Miror)
{
	DrawExtendGraph2D(LeftUpPos, LeftUpPos + Tex->GetGraphSize().Float() * Scale, Tex, Alpha, BlendMode, Miror);
}

void KuroEngine::DrawFunc2D::DrawExtendGraph2D(const Vec2<float>& LeftUpPos, const Vec2<float>& RightBottomPos, const std::shared_ptr<TextureBuffer>& Tex, const float& Alpha, const AlphaBlendMode& BlendMode, const Vec2<bool>& Miror)
{
	//最新のレンダーターゲットのフォーマット取得
	const auto targetFormat = KuroEngine::KuroEngineDevice::Instance()->Graphics().GetRecentRenderTargetFormat(0);

	static std::map<DXGI_FORMAT, std::array<std::shared_ptr<GraphicsPipeline>, AlphaBlendModeNum>>EXTEND_GRAPH_PIPELINE;
	static std::vector<std::shared_ptr<VertexBuffer>>EXTEND_GRAPH_VERTEX_BUFF;

	//DrawExtendGraph専用頂点
	class ExtendGraphVertex
	{
	public:
		Vec2<float>leftUpPos;
		Vec2<float>rightBottomPos;
		Vec2<int> miror;
		float alpha;
		ExtendGraphVertex(const Vec2<float>& LeftUpPos, const Vec2<float>& RightBottomPos, const Vec2<bool>& Miror, const float& Alpha)
			:leftUpPos(LeftUpPos), rightBottomPos(RightBottomPos), miror({ Miror.x ? 1 : 0 ,Miror.y ? 1 : 0 }), alpha(Alpha) {}
	};

	//パイプライン未生成
	if (!EXTEND_GRAPH_PIPELINE[targetFormat][BlendMode])
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		PIPELINE_OPTION.m_depthTest = false;

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/engine/DrawExtendGraph.hlsl", "VSmain", "vs_6_4");
		SHADERS.m_gs = D3D12App::Instance()->CompileShader("resource/engine/DrawExtendGraph.hlsl", "GSmain", "gs_6_4");
		SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/engine/DrawExtendGraph.hlsl", "PSmain", "ps_6_4");

		//インプットレイアウト
		static std::vector<InputLayoutParam>INPUT_LAYOUT =
		{
			InputLayoutParam("POSITION_L_U",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("POSITION_R_B",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("MIROR",DXGI_FORMAT_R32G32_SINT),
			InputLayoutParam("ALPHA",DXGI_FORMAT_R32_FLOAT),
		};

		//ルートパラメータ
		static std::vector<RootParam>ROOT_PARAMETER =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"平行投影行列定数バッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"テクスチャリソース")
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(targetFormat, BlendMode) };
		//パイプライン生成
		EXTEND_GRAPH_PIPELINE[targetFormat][BlendMode] = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, INPUT_LAYOUT, ROOT_PARAMETER, RENDER_TARGET_INFO, { WrappedSampler(false, true) });
	}

	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(EXTEND_GRAPH_PIPELINE[targetFormat][BlendMode]);

	if (EXTEND_GRAPH_VERTEX_BUFF.size() < (s_DrawExtendGraphCount + 1))
	{
		EXTEND_GRAPH_VERTEX_BUFF.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(ExtendGraphVertex), 1, nullptr, ("DrawExtendGraph -" + std::to_string(s_DrawExtendGraphCount)).c_str()));
	}

	ExtendGraphVertex vertex(LeftUpPos, RightBottomPos, Miror, Alpha);
	EXTEND_GRAPH_VERTEX_BUFF[s_DrawExtendGraphCount]->Mapping(&vertex);

	KuroEngine::KuroEngineDevice::Instance()->Graphics().ObjectRender(EXTEND_GRAPH_VERTEX_BUFF[s_DrawExtendGraphCount],
		{
			{KuroEngine::KuroEngineDevice::Instance()->GetParallelMatProjBuff(),CBV},
			{Tex,SRV},
		}, 0, true);

	s_DrawExtendGraphCount++;
}

void KuroEngine::DrawFunc2D::DrawRotaGraph2D(const Vec2<float>& Center, const Vec2<float>& ExtRate, const float& Radian,
	const std::shared_ptr<TextureBuffer>& Tex, const float& Alpha, const Vec2<float>& RotaCenterUV, const AlphaBlendMode& BlendMode, const Vec2<bool>& Miror)
{
	//最新のレンダーターゲットのフォーマット取得
	const auto targetFormat = KuroEngine::KuroEngineDevice::Instance()->Graphics().GetRecentRenderTargetFormat(0);

	static std::map<DXGI_FORMAT, std::array<std::shared_ptr<GraphicsPipeline>, AlphaBlendModeNum>>ROTA_GRAPH_PIPELINE;
	static std::vector<std::shared_ptr<VertexBuffer>>ROTA_GRAPH_VERTEX_BUFF;

	//DrawRotaGraph専用頂点
	class RotaGraphVertex
	{
	public:
		Vec2<float>center;
		Vec2<float> extRate;
		float radian;
		float alpha;
		Vec2<float>rotaCenterUV;
		Vec2<int> miror;
		RotaGraphVertex(const Vec2<float>& Center, const Vec2<float>& ExtRate, const float& Radian, const float& Alpha,
			const Vec2<float>& RotaCenterUV, const Vec2<bool>& Miror)
			:center(Center), extRate(ExtRate), radian(Radian), alpha(Alpha), rotaCenterUV(RotaCenterUV), miror({ Miror.x ? 1 : 0,Miror.y ? 1 : 0 }) {}
	};

	//パイプライン未生成
	if (!ROTA_GRAPH_PIPELINE[targetFormat][BlendMode])
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		PIPELINE_OPTION.m_depthTest = false;

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/engine/DrawRotaGraph.hlsl", "VSmain", "vs_6_4");
		SHADERS.m_gs = D3D12App::Instance()->CompileShader("resource/engine/DrawRotaGraph.hlsl", "GSmain", "gs_6_4");
		SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/engine/DrawRotaGraph.hlsl", "PSmain", "ps_6_4");

		//インプットレイアウト
		static std::vector<InputLayoutParam>INPUT_LAYOUT =
		{
			InputLayoutParam("CENTER",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("EXT_RATE",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("RADIAN",DXGI_FORMAT_R32_FLOAT),
			InputLayoutParam("ALPHA",DXGI_FORMAT_R32_FLOAT),
			InputLayoutParam("ROTA_CENTER_UV",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("MIROR",DXGI_FORMAT_R32G32_SINT)
		};

		//ルートパラメータ
		static std::vector<RootParam>ROOT_PARAMETER =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"平行投影行列定数バッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"テクスチャリソース")
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(targetFormat, BlendMode) };
		//パイプライン生成
		ROTA_GRAPH_PIPELINE[targetFormat][BlendMode] = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, INPUT_LAYOUT, ROOT_PARAMETER, RENDER_TARGET_INFO, { WrappedSampler(true, false) });
	}

	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(ROTA_GRAPH_PIPELINE[targetFormat][BlendMode]);

	if (ROTA_GRAPH_VERTEX_BUFF.size() < (s_DrawRotaGraphCount + 1))
	{
		ROTA_GRAPH_VERTEX_BUFF.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(RotaGraphVertex), 1, nullptr, ("DrawRotaGraph -" + std::to_string(s_DrawRotaGraphCount)).c_str()));
	}

	RotaGraphVertex vertex(Center, ExtRate, Radian, Alpha, RotaCenterUV, Miror);
	ROTA_GRAPH_VERTEX_BUFF[s_DrawRotaGraphCount]->Mapping(&vertex);

	KuroEngine::KuroEngineDevice::Instance()->Graphics().ObjectRender(ROTA_GRAPH_VERTEX_BUFF[s_DrawRotaGraphCount],
		{
			{KuroEngine::KuroEngineDevice::Instance()->GetParallelMatProjBuff(),CBV},
			{Tex,SRV }
		}, 0, true);

	s_DrawRotaGraphCount++;
}

void KuroEngine::DrawFunc2D::DrawRadialWipeGraph2D(
	const Vec2<float>& arg_center,
	const Vec2<float>& arg_extRate, 
	const float& arg_startRadian, 
	const float& arg_endRadian,
	const Vec2<float>& arg_anchor,
	const std::shared_ptr<TextureBuffer>& arg_tex,
	const AlphaBlendMode& BlendMode)
{
	static std::shared_ptr<GraphicsPipeline>RADIAL_WIPE_GRAPH_PIPELINE[AlphaBlendModeNum];
	static std::vector<std::shared_ptr<VertexBuffer>>RADIAL_WIPE_GRAPH_VERTEX_BUFF;

	//DrawRadialWipeGraph専用頂点
	class RadialWipeGraphVertex
	{
	public:
		Vec2<float>m_center;
		Vec2<float>m_extRate;
		float m_sRadian;
		float m_eRadian;
		Vec2<float>m_anchor;
		RadialWipeGraphVertex(const Vec2<float>& arg_center, const Vec2<float>& arg_extRate, const float& arg_sRadian, const float& arg_eRadian, const Vec2<float>& arg_anchor)
			:m_center(arg_center), m_extRate(arg_extRate), m_sRadian(arg_sRadian), m_eRadian(arg_eRadian), m_anchor(arg_anchor) {}
	};

	//パイプライン未生成
	if (!RADIAL_WIPE_GRAPH_PIPELINE[BlendMode])
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		PIPELINE_OPTION.m_depthTest = false;

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/engine/DrawRadialWipeGraph.hlsl", "VSmain", "vs_6_4");
		SHADERS.m_gs = D3D12App::Instance()->CompileShader("resource/engine/DrawRadialWipeGraph.hlsl", "GSmain", "gs_6_4");
		SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/engine/DrawRadialWipeGraph.hlsl", "PSmain", "ps_6_4");

		//インプットレイアウト
		static std::vector<InputLayoutParam>INPUT_LAYOUT =
		{
			InputLayoutParam("CENTER",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("EXT_RATE",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("START_RADIAN",DXGI_FORMAT_R32_FLOAT),
			InputLayoutParam("END_RADIAN",DXGI_FORMAT_R32_FLOAT),
			InputLayoutParam("ANCHOR",DXGI_FORMAT_R32G32_FLOAT),
		};

		//ルートパラメータ
		static std::vector<RootParam>ROOT_PARAMETER =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"平行投影行列定数バッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"テクスチャリソース")
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), BlendMode) };
		//パイプライン生成
		RADIAL_WIPE_GRAPH_PIPELINE[BlendMode] = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, INPUT_LAYOUT, ROOT_PARAMETER, RENDER_TARGET_INFO, { WrappedSampler(true, false) });
	}

	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(RADIAL_WIPE_GRAPH_PIPELINE[BlendMode]);

	if (RADIAL_WIPE_GRAPH_VERTEX_BUFF.size() < (s_DrawRadialWipeGraphCount + 1))
	{
		RADIAL_WIPE_GRAPH_VERTEX_BUFF.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(RadialWipeGraphVertex), 1, nullptr, ("DrawRadialWipeGraph -" + std::to_string(s_DrawRadialWipeGraphCount)).c_str()));
	}

	RadialWipeGraphVertex vertex(arg_center, arg_extRate, arg_startRadian, arg_endRadian, arg_anchor);
	RADIAL_WIPE_GRAPH_VERTEX_BUFF[s_DrawRadialWipeGraphCount]->Mapping(&vertex);

	KuroEngine::KuroEngineDevice::Instance()->Graphics().ObjectRender(RADIAL_WIPE_GRAPH_VERTEX_BUFF[s_DrawRadialWipeGraphCount],
		{
			{KuroEngine::KuroEngineDevice::Instance()->GetParallelMatProjBuff(),CBV},
			{arg_tex,SRV }
		}, 0, 
		BlendMode == AlphaBlendMode_Trans);

	s_DrawRadialWipeGraphCount++;
}

void KuroEngine::DrawFunc2D::DrawNumber2D(const int& arg_num, const Vec2<float>& arg_pos, const std::shared_ptr<TextureBuffer>* arg_numTexArray, const Vec2<float>& arg_expRate,
	const float& arg_letterSpace, const HORIZONTAL_ALIGN& arg_horizontalAlign, const VERTICAL_ALIGN& arg_verticalAlign)
{
	const auto graphSize = arg_numTexArray[0]->GetGraphSize().Float() * arg_expRate;
	const auto numStr = std::to_string(arg_num);
	const auto letterSpace = graphSize.x + arg_letterSpace;

	float startX = arg_pos.x;	//左揃え
	if (arg_horizontalAlign == HORIZONTAL_ALIGN::CENTER)	//中揃え
	{
		startX -= numStr.size() / 2.0f * graphSize.x;
		startX -= floor(numStr.size() / 2.0f) * (arg_letterSpace / 2.0f);
	}
	else if (arg_horizontalAlign == HORIZONTAL_ALIGN::RIGHT)	//右揃え
	{
		startX += numStr.size() * letterSpace;
	}

	float y = arg_pos.y;	//上揃え
	if (arg_verticalAlign == VERTICAL_ALIGN::CENTER)	//中揃え
	{
		y -= graphSize.y / 2.0f;
	}
	else if (arg_verticalAlign == VERTICAL_ALIGN::BOTTOM)	//下揃え
	{
		y -= graphSize.y;
	}

	for (int numIdx = 0; numIdx < numStr.size(); ++numIdx)
	{
		float x = startX + numIdx * letterSpace;
		Vec2<float>pos = { x,y };
		DrawExtendGraph2D(pos, pos + graphSize, arg_numTexArray[numStr[numIdx] - '0']);
	}
}