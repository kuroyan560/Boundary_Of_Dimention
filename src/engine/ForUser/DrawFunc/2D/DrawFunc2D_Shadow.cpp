#include "DrawFunc2D_Shadow.h"
#include"KuroEngineDevice.h"
#include"Render/RenderObject/LightManager.h"

std::shared_ptr<KuroEngine::ConstantBuffer>KuroEngine::DrawFunc2D_Shadow::s_eyePosBuff;
std::shared_ptr<KuroEngine::TextureBuffer>KuroEngine::DrawFunc2D_Shadow::s_defaultTex;
std::shared_ptr<KuroEngine::TextureBuffer>KuroEngine::DrawFunc2D_Shadow::s_defaultNormalMap;
std::shared_ptr<KuroEngine::TextureBuffer>KuroEngine::DrawFunc2D_Shadow::s_defaultEmissiveMap;

//DrawExtendGraph
int KuroEngine::DrawFunc2D_Shadow::s_drawExtendGraphCount = 0;

//DrawRotaGraph
int KuroEngine::DrawFunc2D_Shadow::s_drawRotaGraphCount = 0;

static std::vector<KuroEngine::RootParam>ROOT_PARAMETER =
{
	{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "平行投影行列定数バッファ"},
	{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "視点座標情報"},
	{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "アクティブ中のライト数バッファ"},
	{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "ディレクションライト情報 (構造化バッファ)"},
	{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "ポイントライト情報 (構造化バッファ)"},
	{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "スポットライト情報 (構造化バッファ)"},
	{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "天球ライト情報 (構造化バッファ)"},
	{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "テクスチャリソース"},
	{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "法線マップ"},
	{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "放射マップ"},
};

void KuroEngine::DrawFunc2D_Shadow::StaticInit()
{
	if (s_eyePosBuff)return;

	//視点座標
	Vec3<float>INIT_EYE_POS = { KuroEngine::WinApp::Instance()->GetExpandWinCenter().x,KuroEngine::WinApp::Instance()->GetExpandWinCenter().y,-5.0f };
	s_eyePosBuff = D3D12App::Instance()->GenerateConstantBuffer(sizeof(Vec3<float>), 1, &INIT_EYE_POS, "DrawFunc_Shadow - EYE_POS");

	//白テクスチャ
	s_defaultTex = D3D12App::Instance()->GenerateTextureBuffer(Color(1.0f, 1.0f, 1.0f, 1.0f));

	// (0,0,-1) ノーマルマップ
	s_defaultNormalMap = D3D12App::Instance()->GenerateTextureBuffer(Color(0.5f, 0.5f, 1.0f, 1.0f));

	//黒テクスチャ
	s_defaultEmissiveMap = D3D12App::Instance()->GenerateTextureBuffer(Color(0.0f, 0.0f, 0.0f, 1.0f));
}

void KuroEngine::DrawFunc2D_Shadow::SetEyePos(Vec3<float> EyePos)
{
	s_eyePosBuff->Mapping(&EyePos);
}

void KuroEngine::DrawFunc2D_Shadow::DrawExtendGraph2D(LightManager& LigManager, const Vec2<float>& LeftUpPos, const Vec2<float>& RightBottomPos, const std::shared_ptr<TextureBuffer>& Tex, const std::shared_ptr<TextureBuffer>& NormalMap, const std::shared_ptr<TextureBuffer>& EmissiveMap, const float& SpriteDepth, const Vec2<bool>& Miror, const float& Diffuse, const float& Specular, const float& Lim)
{
	//DrawExtendGraph専用頂点
	class ExtendGraphVertex
	{
	public:
		Vec2<float>leftUpPos;
		Vec2<float>rightBottomPos;
		Vec2<int> miror;
		float depth;
		float diffuse;
		float specular;
		float lim;
		ExtendGraphVertex(const Vec2<float>& LeftUpPos, const Vec2<float>& RightBottomPos, const Vec2<bool>& Miror,
			const float& Depth, const float& Diffuse, const float& Specular, const float& Lim)
			:leftUpPos(LeftUpPos), rightBottomPos(RightBottomPos), miror({ Miror.x ? 1 : 0 ,Miror.y ? 1 : 0 }),
			depth(Depth), diffuse(Diffuse), specular(Specular), lim(Lim) {}
	};

	static std::shared_ptr<GraphicsPipeline>EXTEND_GRAPH_PIPELINE;
	static std::vector<std::shared_ptr<VertexBuffer>>EXTEND_GRAPH_VERTEX_BUFF;

	//パイプライン未生成
	if (!EXTEND_GRAPH_PIPELINE)
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		PIPELINE_OPTION.m_depthTest = false;

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/engine/DrawExtendGraph_Shadow.hlsl", "VSmain", "vs_6_4");
		SHADERS.m_gs = D3D12App::Instance()->CompileShader("resource/engine/DrawExtendGraph_Shadow.hlsl", "GSmain", "gs_6_4");
		SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/engine/DrawExtendGraph_Shadow.hlsl", "PSmain", "ps_6_4");

		//インプットレイアウト
		static std::vector<InputLayoutParam>INPUT_LAYOUT =
		{
			InputLayoutParam("POSITION_L_U",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("POSITION_R_B",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("MIROR",DXGI_FORMAT_R32G32_SINT),
			InputLayoutParam("DEPTH",DXGI_FORMAT_R32_FLOAT),
			InputLayoutParam("DIFFUSE",DXGI_FORMAT_R32_FLOAT),
			InputLayoutParam("SPECULAR",DXGI_FORMAT_R32_FLOAT),
			InputLayoutParam("LIM",DXGI_FORMAT_R32_FLOAT)
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO =
		{
			//バックバッファのフォーマット、アルファブレンド
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(),AlphaBlendMode_Trans),
			//エミッシブ
			RenderTargetInfo(DXGI_FORMAT_R32G32B32A32_FLOAT,AlphaBlendMode_Trans)
		};

		//パイプライン生成
		EXTEND_GRAPH_PIPELINE = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, INPUT_LAYOUT, ROOT_PARAMETER, RENDER_TARGET_INFO, { WrappedSampler(false, true) });
	}

	StaticInit();

	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(EXTEND_GRAPH_PIPELINE);

	if (EXTEND_GRAPH_VERTEX_BUFF.size() < (s_drawExtendGraphCount + 1))
	{
		EXTEND_GRAPH_VERTEX_BUFF.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(ExtendGraphVertex), 1, nullptr, ("DrawExtendGraph_Shadow -" + std::to_string(s_drawExtendGraphCount)).c_str()));
	}

	ExtendGraphVertex vertex(LeftUpPos, RightBottomPos, Miror, SpriteDepth, Diffuse, Specular, Lim);
	EXTEND_GRAPH_VERTEX_BUFF[s_drawExtendGraphCount]->Mapping(&vertex);

	auto normalMap = NormalMap ? NormalMap : s_defaultNormalMap;
	auto emissiveMap = EmissiveMap ? EmissiveMap : s_defaultEmissiveMap;

	std::vector<RegisterDescriptorData>descDatas =
	{
		{KuroEngine::KuroEngineDevice::Instance()->GetParallelMatProjBuff(),CBV},
		{s_eyePosBuff,CBV},	//視点座標
		{LigManager.GetLigNumInfo(),CBV},	//アクティブ中のライト数
		{LigManager.GetLigInfo(Light::DIRECTION),SRV},	//ディレクションライト
		{LigManager.GetLigInfo(Light::POINT),SRV},			//ポイントライト
		{LigManager.GetLigInfo(Light::SPOT),SRV},			//スポットライト
		{LigManager.GetLigInfo(Light::HEMISPHERE),SRV},	//天球ライト
		{Tex,SRV},
		{normalMap,SRV},
		{emissiveMap,SRV}
	};

	KuroEngine::KuroEngineDevice::Instance()->Graphics().ObjectRender(EXTEND_GRAPH_VERTEX_BUFF[s_drawExtendGraphCount],
		descDatas, 0, true);

	s_drawExtendGraphCount++;
}

void KuroEngine::DrawFunc2D_Shadow::DrawRotaGraph2D(LightManager& LigManager, const Vec2<float>& Center, const Vec2<float>& ExtRate, const float& Radian, const std::shared_ptr<TextureBuffer>& Tex, const std::shared_ptr<TextureBuffer>& NormalMap, const std::shared_ptr<TextureBuffer>& EmissiveMap, const float& SpriteDepth, const Vec2<float>& RotaCenterUV, const Vec2<bool>& Miror, const float& Diffuse, const float& Specular, const float& Lim)
{
	//DrawRotaGraph専用頂点
	class RotaGraphVertex
	{
	public:
		Vec2<float>center;
		Vec2<float> extRate;
		float radian;
		Vec2<float>rotaCenterUV;
		Vec2<int> miror;
		float depth;
		float diffuse;
		float specular;
		float lim;
		RotaGraphVertex(const Vec2<float>& Center, const Vec2<float>& ExtRate, const float& Radian,
			const Vec2<float>& RotaCenterUV, const Vec2<bool>& Miror,
			const float& Depth, const float& Diffuse, const float& Specular, const float& Lim)
			:center(Center), extRate(ExtRate), radian(Radian), rotaCenterUV(RotaCenterUV), miror({ Miror.x ? 1 : 0,Miror.y ? 1 : 0 }),
			depth(Depth), diffuse(Diffuse), specular(Specular), lim(Lim) {}
	};

	static std::shared_ptr<GraphicsPipeline>ROTA_GRAPH_PIPELINE;
	static std::vector<std::shared_ptr<VertexBuffer>>ROTA_GRAPH_VERTEX_BUFF;

	//パイプライン未生成
	if (!ROTA_GRAPH_PIPELINE)
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		PIPELINE_OPTION.m_depthTest = false;

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/engine/DrawRotaGraph_Shadow.hlsl", "VSmain", "vs_6_4");
		SHADERS.m_gs = D3D12App::Instance()->CompileShader("resource/engine/DrawRotaGraph_Shadow.hlsl", "GSmain", "gs_6_4");
		SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/engine/DrawRotaGraph_Shadow.hlsl", "PSmain", "ps_6_4");

		//インプットレイアウト
		static std::vector<InputLayoutParam>INPUT_LAYOUT =
		{
			InputLayoutParam("CENTER",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("EXT_RATE",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("RADIAN",DXGI_FORMAT_R32_FLOAT),
			InputLayoutParam("ROTA_CENTER_UV",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("MIROR",DXGI_FORMAT_R32G32_SINT),
			InputLayoutParam("DEPTH",DXGI_FORMAT_R32_FLOAT),
			InputLayoutParam("DIFFUSE",DXGI_FORMAT_R32_FLOAT),
			InputLayoutParam("SPECULAR",DXGI_FORMAT_R32_FLOAT),
			InputLayoutParam("LIM",DXGI_FORMAT_R32_FLOAT)
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO =
		{
			//バックバッファのフォーマット、アルファブレンド
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(),AlphaBlendMode_Trans),
			//エミッシブ
			RenderTargetInfo(DXGI_FORMAT_R32G32B32A32_FLOAT,AlphaBlendMode_Trans)
		};

		//パイプライン生成
		ROTA_GRAPH_PIPELINE = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, INPUT_LAYOUT, ROOT_PARAMETER, RENDER_TARGET_INFO, { WrappedSampler(false,true) });
	}

	StaticInit();

	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(ROTA_GRAPH_PIPELINE);

	if (ROTA_GRAPH_VERTEX_BUFF.size() < (s_drawRotaGraphCount + 1))
	{
		ROTA_GRAPH_VERTEX_BUFF.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(RotaGraphVertex), 1, nullptr, ("DrawRotaGraph_Shadow -" + std::to_string(s_drawRotaGraphCount)).c_str()));
	}

	RotaGraphVertex vertex(Center, ExtRate, Radian, RotaCenterUV, Miror, SpriteDepth, Diffuse, Specular, Lim);
	ROTA_GRAPH_VERTEX_BUFF[s_drawRotaGraphCount]->Mapping(&vertex);

	auto normalMap = NormalMap ? NormalMap : s_defaultNormalMap;
	auto emissiveMap = EmissiveMap ? EmissiveMap : s_defaultEmissiveMap;

	KuroEngine::KuroEngineDevice::Instance()->Graphics().ObjectRender(ROTA_GRAPH_VERTEX_BUFF[s_drawRotaGraphCount],
		{
			{KuroEngine::KuroEngineDevice::Instance()->GetParallelMatProjBuff(),CBV},
			{s_eyePosBuff,CBV},	//視点座標
			{LigManager.GetLigNumInfo(),CBV},	//アクティブ中のライト数
			{LigManager.GetLigInfo(Light::DIRECTION),SRV},	//ディレクションライト
			{LigManager.GetLigInfo(Light::POINT),SRV},	//ポイントライト
			{LigManager.GetLigInfo(Light::SPOT),SRV},	//スポットライト
			{LigManager.GetLigInfo(Light::HEMISPHERE),SRV},	//天球ライト
			{Tex,SRV},
			{normalMap,SRV},
			{emissiveMap,SRV}
		},
		0, true);

	s_drawRotaGraphCount++;
}