#include "WaterPaintBlend.h"
#include"DirectX12/D3D12App.h"
#include"KuroEngineDevice.h"
#include"ForUser/DrawFunc/BillBoard/DrawFuncBillBoard.h"
#include"Render/RenderObject/Camera.h"

std::shared_ptr<KuroEngine::VertexBuffer>WaterPaintBlend::s_maskInkPolygon;
std::shared_ptr<KuroEngine::ComputePipeline>WaterPaintBlend::s_initInkPipeline;
std::shared_ptr<KuroEngine::ComputePipeline>WaterPaintBlend::s_appearInkPipeline;
std::shared_ptr<KuroEngine::ComputePipeline>WaterPaintBlend::s_updateInkPipeline;
std::shared_ptr<KuroEngine::GraphicsPipeline>WaterPaintBlend::s_drawInkPipeline;
std::shared_ptr<KuroEngine::ComputePipeline>WaterPaintBlend::s_waterPaintPipeline;

void WaterPaintBlend::GeneratePipeline()
{
	using namespace KuroEngine;

	//マスクインク描画用の頂点バッファ
	{
		struct Vertex
		{
			Vec3<float>m_pos;
			Vec2<float> m_uv;
			Vertex(Vec3<float>arg_pos, Vec2<float>arg_uv) :m_pos(arg_pos), m_uv(arg_uv) {}
		};

		std::array<Vertex, 4>vertices =
		{
			Vertex({-0.5f,-0.5f,0.0f},{0.0f,1.0f}),	//左下
			Vertex({-0.5f,+0.5f,0.0f},{0.0f,0.0f}),	//左上
			Vertex({+0.5f,-0.5f,0.0f},{1.0f,1.0f}),	//右下
			Vertex({+0.5f,+0.5f,0.0f},{1.0f,0.0f}),	//右上
		};
		s_maskInkPolygon = D3D12App::Instance()->GenerateVertexBuffer(
			sizeof(Vertex),
			4,
			vertices.data(),
			"WaterPaintBlend - MaskInkPolygon - VertexBuffer");
	}

	//マスクインクに関するコンピュートパイプライン
	{
		//ルートパラメータ
		std::vector<RootParam>rootParam =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"生成したインクのバッファー(RWStructuredBuffer)"),
		};

		//初期化用パイプライン
		auto cs_init = D3D12App::Instance()->CompileShader("resource/user/shaders/MaskInk.hlsl", "Init", "cs_6_4");
		s_initInkPipeline = D3D12App::Instance()->GenerateComputePipeline(cs_init, rootParam, { WrappedSampler(true,true) });

		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "生成する予定のスタックしたインクバッファー(StructuredBuffer)");
		rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "更新時に使用する定数バッファ");

		//生成用パイプライン
		auto cs_appear = D3D12App::Instance()->CompileShader("resource/user/shaders/MaskInk.hlsl", "Appear", "cs_6_4");
		s_appearInkPipeline = D3D12App::Instance()->GenerateComputePipeline(cs_appear, rootParam, { WrappedSampler(true,true) });

		//更新用パイプライン
		auto cs_update = D3D12App::Instance()->CompileShader("resource/user/shaders/MaskInk.hlsl", "Update", "cs_6_4");
		s_updateInkPipeline = D3D12App::Instance()->GenerateComputePipeline(cs_update, rootParam, { WrappedSampler(true,true) });
	}
	//マスクインク描画用グラフィックスパイプライン
	{
		std::vector<RootParam>rootParam =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"生成したインクのバッファー(RWStructuredBuffer)"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"カメラバッファ"),
		};
		for (int texIdx = 0; texIdx < INK_TEX_NUM; ++texIdx)
		{
			rootParam.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "インク画像");
		}

		//描画用パイプライン
		PipelineInitializeOption option(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		option.m_depthWriteMask = false;

		Shaders shaders;
		shaders.m_vs = D3D12App::Instance()->CompileShader("resource/user/shaders/MaskInk_Draw.hlsl", "VSmain", "vs_6_4");
		shaders.m_ps = D3D12App::Instance()->CompileShader("resource/user/shaders/MaskInk_Draw.hlsl", "PSmain", "ps_6_4");

		std::vector<InputLayoutParam>inputLayout = { InputLayoutParam("POSITION",DXGI_FORMAT_R16G16B16A16_FLOAT) };

		s_drawInkPipeline = D3D12App::Instance()->GenerateGraphicsPipeline(
			option,
			shaders,
			{
				InputLayoutParam("POSITION",DXGI_FORMAT_R32G32B32_FLOAT),
				InputLayoutParam("TEXCOORD",DXGI_FORMAT_R32G32_FLOAT),
			},
			rootParam,
			{ RenderTargetInfo(DXGI_FORMAT_R16G16B16A16_FLOAT, AlphaBlendMode_Trans) },
			{ WrappedSampler(true,true) });
	}

	//水彩画風にするパイプライン
	{
		//ルートパラメータ
		std::vector<RootParam>rootParam =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"元画像"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"マスク画像"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"描き込み先バッファ"),
		};

		//シェーダーコンパイル
		auto cs = D3D12App::Instance()->CompileShader("resource/user/shaders/WaterPaintBlend.hlsl", "CSmain", "cs_6_4");

		//パイプライン生成
		s_waterPaintPipeline = D3D12App::Instance()->GenerateComputePipeline(cs, rootParam, { WrappedSampler(true,true) });
	}

}

void WaterPaintBlend::OnImguiItems()
{
	if (CustomParamDirty())
	{
		m_constBuffer->Mapping(&m_constData);
	}
	ImGui::Text("AliveMaskInkNum : %d / %d", *m_aliveInkCounterBuffer->GetResource()->GetBuffOnCpu<unsigned int>(), m_aliveInkMax);
}

WaterPaintBlend::WaterPaintBlend() : Debugger("WaterPaintBlend")
{
	using namespace KuroEngine;

	//パイプライン未生成なら生成
	if (!s_waterPaintPipeline)GeneratePipeline();

	AddCustomParameter("initScale", { "ConstData","initScale" }, PARAM_TYPE::FLOAT, &m_constData.m_initScale, "ConstData");
	AddCustomParameter("posOffsetMax", { "ConstData","posOffsetMax" }, PARAM_TYPE::FLOAT, &m_constData.m_posOffsetMax, "ConstData");
	AddCustomParameter("updateSpan", { "MaskInk","updateSpan" }, PARAM_TYPE::INT, &m_updateSpan, "MaskInk");
	LoadParameterLog();

	//インクテクスチャ読み込み
	D3D12App::Instance()->GenerateTextureBuffer(
		m_inkTexArray.data(),
		"resource/user/tex/ink.png",
		INK_TEX_NUM,
		Vec2<int>(INK_TEX_NUM, 1));

	//定数バッファ生成
	m_constBuffer = D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(ConstData),
		1,
		&m_constData,
		"WaterPaintBlend - ConstantBuffer");

	//生成予定のインクをスタックしておくバッファ
	m_stackInkBuffer = D3D12App::Instance()->GenerateStructuredBuffer(
		sizeof(Vec3<float>),
		GENERATE_MAX_ONCE,
		nullptr,
		"WaterPaintBlend - StackInk - StructuredBuffer");

	//生成したインクのバッファ
	D3D12App::Instance()->GenerateRWStructuredBuffer(
		&m_aliveInkBuffer, &m_aliveInkCounterBuffer,
		sizeof(MaskInk),
		m_aliveInkMax,
		nullptr,
		"WaterPaintBlend - AliveInk - RWStructuredBuffer");

	auto targetSize = D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize();

	//結果の描画先テクスチャ生成
	m_resultTex = D3D12App::Instance()->GenerateTextureBuffer(
		targetSize,
		D3D12App::Instance()->GetBackBuffFormat(),
		"WaterPaintBlend - ResultTex");

	//マスクレイヤー生成
	m_maskLayer = D3D12App::Instance()->GenerateRenderTarget(
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		Color(0, 0, 0, 0),
		targetSize, L"MaskLayer");
}

void WaterPaintBlend::Init()
{
	using namespace KuroEngine;

	//カウント取得
	auto aliveInkCount = *m_aliveInkCounterBuffer->GetResource()->GetBuffOnCpu<int>();

	//マスクインクの初期化（全消し）
	if (aliveInkCount)
	{
		D3D12App::Instance()->DispathOneShot(
			s_initInkPipeline,
			{ aliveInkCount,1,1 },
		{
			{m_aliveInkBuffer,UAV},
		});
	}

	m_updateTimer.Reset(m_updateSpan);
}

void WaterPaintBlend::DropMaskInk(KuroEngine::Vec3<float> arg_pos)
{
	m_appearInkPosArray.emplace_back(arg_pos);
}

void WaterPaintBlend::Register(std::shared_ptr<KuroEngine::TextureBuffer> arg_baseTex, KuroEngine::Camera& arg_cam, std::weak_ptr<KuroEngine::DepthStencil>arg_depthStencil)
{
	using namespace KuroEngine;

	//登録するディスクリプタの情報配列
	std::vector<RegisterDescriptorData>maskInkDescData_Compute =
	{
		{m_aliveInkBuffer,UAV},
		{m_stackInkBuffer,SRV},
		{m_constBuffer,CBV},
	};

	//カウントのポインタ取得
	auto aliveInkCountPtr = m_aliveInkCounterBuffer->GetResource()->GetBuffOnCpu<int>();

	//インクの更新
	if (*aliveInkCountPtr && m_updateTimer.UpdateTimer(1.0f))
	{
		D3D12App::Instance()->DispathOneShot(
			s_updateInkPipeline,
			{ *aliveInkCountPtr,1,1 },
			maskInkDescData_Compute);
		m_updateTimer.Reset(m_updateSpan);
	}

	//インクの出現
	if (!m_appearInkPosArray.empty())
	{
		//一度に生成できるマスクインクの量を超えてる
		int appearNum = static_cast<int>(m_appearInkPosArray.size());
		if (GENERATE_MAX_ONCE < appearNum)
		{
			AppearMessageBox("WaterPaintBlend : Register() 失敗", "一度に生成できるマスクインクの量を超えてるよ");
			exit(1);
		}


		//出現するインクの座標情報配列を送信
		m_stackInkBuffer->Mapping(m_appearInkPosArray.data(), appearNum);

		//インク生成
		D3D12App::Instance()->DispathOneShot(
			s_appearInkPipeline,
			{ appearNum,1,1 },
			maskInkDescData_Compute);

		//上限到達
		if (m_aliveInkMax < *aliveInkCountPtr)
		{
			AppearMessageBox("WaterPaintBlend : Register() 失敗", "生成できるマスクインクの量を超えてるよ");
			exit(1);
		}

		//出現するインクの座標配列リセット
		m_appearInkPosArray.clear();
	}

	//マスクレイヤークリア
	KuroEngineDevice::Instance()->Graphics().ClearRenderTarget(m_maskLayer);

	//マスクレイヤーにインク描画
	std::vector<RegisterDescriptorData>maskInkDescData_Graphics =
	{
		{m_aliveInkBuffer,UAV},
		{arg_cam.GetBuff(),CBV},
	};
	for (int texIdx = 0; texIdx < INK_TEX_NUM; ++texIdx)
	{
		maskInkDescData_Graphics.emplace_back(m_inkTexArray[texIdx], SRV);
	}
	KuroEngineDevice::Instance()->Graphics().SetRenderTargets({ m_maskLayer }, arg_depthStencil);
	KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(s_drawInkPipeline);
	KuroEngineDevice::Instance()->Graphics().ObjectRender(
		s_maskInkPolygon,
		maskInkDescData_Graphics,
		0.0f,
		true,
		*aliveInkCountPtr);

	//水彩画風加工
	KuroEngineDevice::Instance()->Graphics().SetComputePipeline(s_waterPaintPipeline);
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
			{m_resultTex,UAV}
		});
}
