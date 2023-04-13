#include "BasicDraw.h"
#include"ForUser/Object/Model.h"
#include"KuroEngineDevice.h"
#include"ForUser/Object/Object.h"
#include"Render/RenderObject/ModelInfo/ModelAnimator.h"
#include"Render/CubeMap.h"
#include"Render/RenderObject/Camera.h"
#include"Render/RenderObject/LightManager.h"
#include"Render/RenderObject/SpriteMesh.h"
#include"KuroEngineDevice.h"

BasicDraw::BasicDraw() :KuroEngine::Debugger("BasicDraw")
{
	AddCustomParameter("BrightThresholdLow", { "Toon","BrightThreshold","Low" }, PARAM_TYPE::FLOAT,
		&m_toonCommonParam.m_brightThresholdLow, "Toon", true, 0.0f, 1.0f);
	AddCustomParameter("BrightThresholdRange", { "Toon","BrightThreshold","Range" }, PARAM_TYPE::FLOAT,
		&m_toonCommonParam.m_brightThresholdRange, "Toon");
	AddCustomParameter("MonochromeRate", { "Toon","MonochromeRate" }, PARAM_TYPE::FLOAT,
		&m_toonCommonParam.m_monochromeRate, "Toon", true, 0.0f, 1.0f);

	AddCustomParameter("DepthDifferenceThreshold", { "Edge","DepthDifferenceThreshold" }, PARAM_TYPE::FLOAT,
		&m_edgeShaderParam.m_depthThreshold, "Edge");

	auto& defaultParam = IndividualDrawParameter::GetDefault();
	AddCustomParameter("FillColor", { "DefaultDrawParam","FillColor" }, PARAM_TYPE::COLOR,
		&defaultParam.m_fillColor, "DefaultDrawParam");
	AddCustomParameter("BrightMulColor", { "DefaultDrawParam","BrightMulColor" }, PARAM_TYPE::COLOR,
		&defaultParam.m_brightMulColor, "DefaultDrawParam");
	AddCustomParameter("DarkMulColor", { "DefaultDrawParam","DarkMulColor" }, PARAM_TYPE::COLOR,
		&defaultParam.m_darkMulColor, "DefaultDrawParam");
	AddCustomParameter("EdgeColor", { "DefaultDrawParam","EdgeColor" }, PARAM_TYPE::COLOR,
		&defaultParam.m_edgeColor, "DefaultDrawParam");

	LoadParameterLog();

	m_playerMaskTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/playerMaskTex.png");
}

void BasicDraw::OnImguiItems()
{
	m_toonCommonParam.m_brightThresholdRange = std::min(m_toonCommonParam.m_brightThresholdRange, 1.0f - m_toonCommonParam.m_brightThresholdLow);

	if (CustomParamDirty())
	{
		m_toonCommonParamBuff->Mapping(&m_toonCommonParam);
		m_edgeShaderParamBuff->Mapping(&m_edgeShaderParam);
	}

}

void BasicDraw::Awake(KuroEngine::Vec2<float>arg_screenSize, int arg_prepareBuffNum)
{
	using namespace KuroEngine;
	m_spriteMesh = std::make_unique<SpriteMesh>("BasicDraw");
	m_spriteMesh->SetSize(arg_screenSize);

	//ルートパラメータ
	static std::vector<RootParam>ROOT_PARAMETER =
	{
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"カメラ情報バッファ"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "アクティブ中のライト数バッファ"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "ディレクションライト情報 (構造化バッファ)"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "ポイントライト情報 (構造化バッファ)"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "スポットライト情報 (構造化バッファ)"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "天球ライト情報 (構造化バッファ)"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"トランスフォームバッファ"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"ボーン行列バッファ"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"ベースカラーテクスチャ"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"マテリアル基本情報バッファ"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"トゥーンの共通パラメータ"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"トゥーンの個別パラメータ"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"プレイヤーの座標情報"),
	};

	//レンダーターゲット描画先情報
	std::array<std::vector<RenderTargetInfo>, RENDER_TARGET_TYPE::NUM>RENDER_TARGET_INFO;
	for (int i = 0; i < AlphaBlendModeNum; ++i)
	{
		RENDER_TARGET_INFO[i] =
		{
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), (AlphaBlendMode)i),	//通常描画
			RenderTargetInfo(DXGI_FORMAT_R32G32B32A32_FLOAT, AlphaBlendMode_Trans),	//エミッシブマップ
			RenderTargetInfo(DXGI_FORMAT_R16_FLOAT, AlphaBlendMode_None),	//深度マップ
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_None),	//エッジカラーマップ
			RenderTargetInfo(DXGI_FORMAT_R16G16B16A16_FLOAT, AlphaBlendMode_None),	//草むらマップ
			RenderTargetInfo(DXGI_FORMAT_R16G16B16A16_FLOAT, AlphaBlendMode_None),	//ノーマルマップ
			RenderTargetInfo(DXGI_FORMAT_R32G32B32A32_FLOAT, AlphaBlendMode_Trans),	//ワールド座標保存用
		};
	}

	//通常描画パイプライン生成
	for (int i = 0; i < AlphaBlendModeNum; ++i)
	{
		auto blendMode = (AlphaBlendMode)i;

		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/user/shaders/BasicShader.hlsl", "VSmain", "vs_6_4");
		SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/user/shaders/BasicShader.hlsl", "PSmain", "ps_6_4");

		//パイプライン生成
		m_drawPipeline[blendMode] = D3D12App::Instance()->GenerateGraphicsPipeline(
			PIPELINE_OPTION, 
			SHADERS,
			ModelMesh::Vertex::GetInputLayout(), 
			ROOT_PARAMETER, 
			RENDER_TARGET_INFO[i],
			{ WrappedSampler(true, true) });
	}

	//プレイヤー用パイプライン生成
	{

		std::vector<RenderTargetInfo> playerRenderTargetInfo =
		{
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), (AlphaBlendMode)0),	//通常描画
			RenderTargetInfo(DXGI_FORMAT_R32G32B32A32_FLOAT, AlphaBlendMode_Trans),	//エミッシブマップ
			RenderTargetInfo(DXGI_FORMAT_R16_FLOAT, AlphaBlendMode_None),	//深度マップ
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_None),	//エッジカラーマップ
			RenderTargetInfo(DXGI_FORMAT_R16G16B16A16_FLOAT, AlphaBlendMode_None),	//草むらマップ
		};

		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/user/shaders/BasicShader_Player.hlsl", "VSmain", "vs_6_4");
		SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/user/shaders/BasicShader_Player.hlsl", "PSmain", "ps_6_4");

		//パイプライン生成
		m_drawPipeline_player = D3D12App::Instance()->GenerateGraphicsPipeline(
			PIPELINE_OPTION,
			SHADERS,
			ModelMesh::Vertex::GetInputLayout(),
			ROOT_PARAMETER,
			playerRenderTargetInfo,
			{ WrappedSampler(true, true) });

	}

	//インスタンシング描画パイプライン生成
	for (int writeDepth = 0; writeDepth < 2; ++writeDepth)
	{
		//通常描画パイプライン生成
		for (int i = 0; i < AlphaBlendModeNum; ++i)
		{
			auto blendMode = (AlphaBlendMode)i;

			//パイプライン設定
			static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			PIPELINE_OPTION.m_depthWriteMask = (writeDepth == WRITE_DEPTH);

			//シェーダー情報
			static Shaders SHADERS;
			SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/user/shaders/BasicShader_Huge.hlsl", "VSmain", "vs_6_4");
			SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/user/shaders/BasicShader_Huge.hlsl", "PSmain", "ps_6_4");

			//パイプライン生成
			m_instancingDrawPipeline[writeDepth][i] = D3D12App::Instance()->GenerateGraphicsPipeline(
				PIPELINE_OPTION,
				SHADERS,
				ModelMesh::Vertex::GetInputLayout(),
				ROOT_PARAMETER,
				RENDER_TARGET_INFO[i],
				{ WrappedSampler(true, true) });
		}
	}

	//トゥーンシェーダー用の共通のバッファを用意
	m_toonCommonParamBuff = D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(m_toonCommonParam),
		1,
		&m_toonCommonParam,
		"BasicDraw - ToonCommonParameter");

	//その他個別の描画に必要な情報のバッファを用意
	for (int i = 0; i < arg_prepareBuffNum; ++i)
	{
		m_drawTransformBuff.emplace_back(
			D3D12App::Instance()->GenerateConstantBuffer(
				sizeof(Matrix),
				1,
				nullptr,
				("BasicDraw - Transform -" + std::to_string(i)).c_str()));

		m_toonIndividualParamBuff.emplace_back(
			D3D12App::Instance()->GenerateConstantBuffer(
				sizeof(IndividualDrawParameter),
				1,
				nullptr,
				("BasicDraw - IndividualDrawParameter -" + std::to_string(i)).c_str()));

		m_drawTransformBuffHuge.emplace_back(
			D3D12App::Instance()->GenerateConstantBuffer(
				sizeof(Matrix),
				s_instanceMax,
				nullptr, ("BasicDraw - InstancingDraw - Transform - " + std::to_string(i)).c_str()));
	}

	//エッジラインパイプライン
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/user/shaders/EdgeShader.hlsl", "VSmain", "vs_6_4");
		SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/user/shaders/EdgeShader.hlsl", "PSmain", "ps_6_4");

		//ルートパラメータ
		static std::vector<RootParam>ROOT_PARAMETER =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"平行投影行列"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"デプスマップ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"光が当たっている範囲のマップ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"エッジカラーマップ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"法線マップ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"専用のパラメータ"),
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO =
		{
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_None),	//通常描画
		};
		//パイプライン生成
		m_edgePipeline = D3D12App::Instance()->GenerateGraphicsPipeline(
			PIPELINE_OPTION, 
			SHADERS, 
			SpriteMesh::Vertex::GetInputLayout(),
			ROOT_PARAMETER,
			RENDER_TARGET_INFO,
			{ WrappedSampler(false, true) });
	}

	//ウィンドウサイズからUVオフセットを求める
	const auto winSize = WinApp::Instance()->GetWinSize().Float();
	m_edgeShaderParam.m_uvOffset = 
	{
		Vec2<float>(0.0f,  1.0f / winSize.y), //上
		Vec2<float>(0.0f, -1.0f / winSize.y), //下
		Vec2<float>(1.0f / winSize.x,           0.0f), //右
		Vec2<float>(-1.0f / winSize.x,           0.0f), //左
		Vec2<float>(1.0f / winSize.x,  1.0f / winSize.y), //右上
		Vec2<float>(-1.0f / winSize.x,  1.0f / winSize.y), //左上
		Vec2<float>(1.0f / winSize.x, -1.0f / winSize.y), //右下
		Vec2<float>(-1.0f / winSize.x, -1.0f / winSize.y)  //左下
	};

	//エッジ出力用のバッファを用意
	m_edgeShaderParamBuff = D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(m_edgeShaderParam),
		1, 
		&m_edgeShaderParam, 
		"BasicDraw - EdgeCommonParameter");

	//プレイヤーの座標を送るための定数バッファ用意
	PlayerInfo initSendPlayerInfo;
	initSendPlayerInfo.m_worldPos = { FLT_MAX,FLT_MAX,FLT_MAX };
	initSendPlayerInfo.m_screenPos = { 0,0 };
	m_playerInfoBuffer = D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(PlayerInfo),
		1,
		&initSendPlayerInfo,
		"BasicDraw - PlayerInfo");

	//レンダーターゲット生成
	std::array<std::string, RENDER_TARGET_TYPE::NUM>targetNames =
	{
		"MainRenderTarget","EmissiveMap","DepthMap","EdgeColorMap","BrightMap", "NormalMap", "WorldPos"
	};
	auto targetSize = D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize();
	for (int targetIdx = 0; targetIdx < RENDER_TARGET_TYPE::NUM; ++targetIdx)
	{
		m_renderTargetArray[targetIdx] = D3D12App::Instance()->GenerateRenderTarget(
			RENDER_TARGET_INFO[0][targetIdx].m_format,
			Color(0.0f, 0.0f, 0.0f, 0.0f),
			targetSize,
			GetWideStrFromStr(("BasicDraw - " + targetNames[targetIdx])).c_str());
	}
}

void BasicDraw::Update(KuroEngine::Vec3<float> arg_playerPos, KuroEngine::Camera& arg_cam)
{
	using namespace KuroEngine;

	//プレイヤーの座標に変化があったらデータ送信
	if (FLT_EPSILON < m_playerInfoBuffer->GetResource()->GetBuffOnCpu<PlayerInfo>()->m_worldPos.Distance(arg_playerPos))
	{
		PlayerInfo sendInfo;
		sendInfo.m_worldPos = arg_playerPos;
		sendInfo.m_screenPos = ConvertWorldToScreen(arg_playerPos, arg_cam.GetViewMat(), arg_cam.GetProjectionMat(), m_renderTargetArray[MAIN]->GetGraphSize().Float());
		m_playerInfoBuffer->Mapping(&sendInfo);
	}
}

void BasicDraw::RenderTargetsClearAndSet(std::weak_ptr<KuroEngine::DepthStencil>arg_ds)
{
	using namespace KuroEngine;

	std::vector<std::weak_ptr<RenderTarget>>rts;
	for (int targetIdx = 0; targetIdx < RENDER_TARGET_TYPE::NUM; ++targetIdx)
	{
		rts.emplace_back(m_renderTargetArray[targetIdx]);
		KuroEngineDevice::Instance()->Graphics().ClearRenderTarget(m_renderTargetArray[targetIdx]);
	}
	KuroEngineDevice::Instance()->Graphics().ClearDepthStencil(arg_ds);

	KuroEngineDevice::Instance()->Graphics().SetRenderTargets(rts, arg_ds.lock());
}

void BasicDraw::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform& arg_transform, const IndividualDrawParameter& arg_toonParam, const KuroEngine::AlphaBlendMode& arg_blendMode, std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff)
{
	using namespace KuroEngine;

	KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(m_drawPipeline[arg_blendMode]);

	//トランスフォームバッファ送信
	if (m_drawTransformBuff.size() < (m_drawCount + 1))
	{
		m_drawTransformBuff.emplace_back(D3D12App::Instance()->GenerateConstantBuffer(sizeof(Matrix), 1, nullptr, ("BasicDraw - Transform -" + std::to_string(m_drawCount)).c_str()));
	}
	m_drawTransformBuff[m_drawCount]->Mapping(&arg_transform.GetMatWorld());

	//トゥーンの個別パラメータバッファ送信
	if (m_toonIndividualParamBuff.size() < (m_individualParamCount + 1))
	{
		m_toonIndividualParamBuff.emplace_back(D3D12App::Instance()->GenerateConstantBuffer(sizeof(IndividualDrawParameter), 1, nullptr, ("BasicDraw - IndividualDrawParameter -" + std::to_string(m_individualParamCount)).c_str()));
	}
	m_toonIndividualParamBuff[m_individualParamCount]->Mapping(&arg_toonParam);

	auto model = arg_model.lock();

	for (int meshIdx = 0; meshIdx < model->m_meshes.size(); ++meshIdx)
	{
		const auto& mesh = model->m_meshes[meshIdx];
		KuroEngineDevice::Instance()->Graphics().ObjectRender(
			mesh.mesh->vertBuff,
			mesh.mesh->idxBuff,
			{
				{arg_cam.GetBuff(),CBV},
				{arg_ligMgr.GetLigNumInfo(),CBV},
				{arg_ligMgr.GetLigInfo(Light::DIRECTION),SRV},
				{arg_ligMgr.GetLigInfo(Light::POINT),SRV},
				{arg_ligMgr.GetLigInfo(Light::SPOT),SRV},
				{arg_ligMgr.GetLigInfo(Light::HEMISPHERE),SRV},
				{m_drawTransformBuff[m_drawCount],CBV},
				{arg_boneBuff,CBV},
				{mesh.material->texBuff[COLOR_TEX],SRV},
				{mesh.material->buff,CBV},
				{m_toonCommonParamBuff,CBV},
				{m_toonIndividualParamBuff[m_individualParamCount],CBV},
				{m_playerInfoBuffer,CBV},
			},
			arg_transform.GetPos().z,
			arg_blendMode == AlphaBlendMode_Trans);
	}

	m_drawCount++;
	m_individualParamCount++;
}

void BasicDraw::Draw_Player(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform& arg_transform, const IndividualDrawParameter& arg_toonParam, const KuroEngine::AlphaBlendMode& arg_blendMode, std::shared_ptr<KuroEngine::ConstantBuffer> arg_boneBuff)
{

	using namespace KuroEngine;

	KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(m_drawPipeline_player);

	//トランスフォームバッファ送信
	if (m_drawTransformBuff.size() < (m_drawCount + 1))
	{
		m_drawTransformBuff.emplace_back(D3D12App::Instance()->GenerateConstantBuffer(sizeof(Matrix), 1, nullptr, ("BasicDraw - Transform -" + std::to_string(m_drawCount)).c_str()));
	}
	m_drawTransformBuff[m_drawCount]->Mapping(&arg_transform.GetMatWorld());

	//トゥーンの個別パラメータバッファ送信
	if (m_toonIndividualParamBuff.size() < (m_individualParamCount + 1))
	{
		m_toonIndividualParamBuff.emplace_back(D3D12App::Instance()->GenerateConstantBuffer(sizeof(IndividualDrawParameter), 1, nullptr, ("BasicDraw - IndividualDrawParameter -" + std::to_string(m_individualParamCount)).c_str()));
	}
	m_toonIndividualParamBuff[m_individualParamCount]->Mapping(&arg_toonParam);

	auto model = arg_model.lock();

	for (int meshIdx = 0; meshIdx < model->m_meshes.size(); ++meshIdx)
	{
		const auto& mesh = model->m_meshes[meshIdx];
		KuroEngineDevice::Instance()->Graphics().ObjectRender(
			mesh.mesh->vertBuff,
			mesh.mesh->idxBuff,
			{
				{arg_cam.GetBuff(),CBV},
				{arg_ligMgr.GetLigNumInfo(),CBV},
				{arg_ligMgr.GetLigInfo(Light::DIRECTION),SRV},
				{arg_ligMgr.GetLigInfo(Light::POINT),SRV},
				{arg_ligMgr.GetLigInfo(Light::SPOT),SRV},
				{arg_ligMgr.GetLigInfo(Light::HEMISPHERE),SRV},
				{m_drawTransformBuff[m_drawCount],CBV},
				{arg_boneBuff,CBV},
				{mesh.material->texBuff[COLOR_TEX],SRV},
				{mesh.material->buff,CBV},
				{m_toonCommonParamBuff,CBV},
				{m_toonIndividualParamBuff[m_individualParamCount],CBV},
				{m_playerInfoBuffer,CBV},
			},
			arg_transform.GetPos().z,
			arg_blendMode == AlphaBlendMode_Trans);
	}

	m_drawCount++;
	m_individualParamCount++;

}

void BasicDraw::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform& arg_transform, const KuroEngine::AlphaBlendMode& arg_blendMode, std::shared_ptr<KuroEngine::ConstantBuffer> arg_boneBuff)
{
	BasicDraw::Draw(arg_cam, arg_ligMgr, arg_model, arg_transform, IndividualDrawParameter::GetDefault(), arg_blendMode, arg_boneBuff);
}

void BasicDraw::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, const std::weak_ptr<KuroEngine::ModelObject> arg_modelObj, const IndividualDrawParameter& arg_toonParam, const KuroEngine::AlphaBlendMode& arg_blendMode)
{
	using namespace KuroEngine;

	auto obj = arg_modelObj.lock();
	//ボーン行列バッファ取得（アニメーターがnullptrなら空）
	auto model = obj->m_model;
	std::shared_ptr<ConstantBuffer>boneBuff;
	if (obj->m_animator)boneBuff = obj->m_animator->GetBoneMatBuff();
	Draw(arg_cam, arg_ligMgr, model, obj->m_transform, arg_toonParam, arg_blendMode, boneBuff);
}

void BasicDraw::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, const std::weak_ptr<KuroEngine::ModelObject> arg_modelObj, const KuroEngine::AlphaBlendMode& arg_blendMode)
{
	Draw(arg_cam, arg_ligMgr, arg_modelObj, IndividualDrawParameter::GetDefault(), arg_blendMode);
}

void BasicDraw::InstancingDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, std::weak_ptr<KuroEngine::Model> arg_model, std::vector<KuroEngine::Matrix>& arg_matArray, const IndividualDrawParameter& arg_toonParam, bool arg_depthWriteMask, const KuroEngine::AlphaBlendMode& arg_blendMode, const float& arg_depth, std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff)
{
	using namespace KuroEngine;

	if (s_instanceMax < static_cast<int>(arg_matArray.size()))
	{
		KuroEngine::AppearMessageBox("BasicDraw : InstancingDraw() 失敗", "インスタンシング描画の最大数を超えてるよ");
		exit(-1);
	}
	if (arg_matArray.empty())return;

	using namespace KuroEngine;

	KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(m_instancingDrawPipeline[arg_depthWriteMask ? WRITE_DEPTH : NOT_WRITE_DEPTH][arg_blendMode]);

	//トランスフォームバッファ送信
	if (m_drawTransformBuffHuge.size() < (m_drawCountHuge + 1))
	{
		m_drawTransformBuffHuge.emplace_back(D3D12App::Instance()->GenerateConstantBuffer(sizeof(Matrix), 1, nullptr, ("BasicDraw - InstancingDraw - Transform -" + std::to_string(m_drawCountHuge)).c_str()));
	}
	m_drawTransformBuffHuge[m_drawCountHuge]->Mapping(arg_matArray.data(),static_cast<int>(arg_matArray.size()));

	//トゥーンの個別パラメータバッファ送信
	if (m_toonIndividualParamBuff.size() < (m_individualParamCount + 1))
	{
		m_toonIndividualParamBuff.emplace_back(D3D12App::Instance()->GenerateConstantBuffer(sizeof(IndividualDrawParameter), 1, nullptr, ("BasicDraw - IndividualDrawParameter -" + std::to_string(m_individualParamCount)).c_str()));
	}
	m_toonIndividualParamBuff[m_individualParamCount]->Mapping(&arg_toonParam);

	auto model = arg_model.lock();

	for (int meshIdx = 0; meshIdx < model->m_meshes.size(); ++meshIdx)
	{
		const auto& mesh = model->m_meshes[meshIdx];
		KuroEngineDevice::Instance()->Graphics().ObjectRender(
			mesh.mesh->vertBuff,
			mesh.mesh->idxBuff,
			{
				{arg_cam.GetBuff(),CBV},
				{arg_ligMgr.GetLigNumInfo(),CBV},
				{arg_ligMgr.GetLigInfo(Light::DIRECTION),SRV},
				{arg_ligMgr.GetLigInfo(Light::POINT),SRV},
				{arg_ligMgr.GetLigInfo(Light::SPOT),SRV},
				{arg_ligMgr.GetLigInfo(Light::HEMISPHERE),SRV},
				{m_drawTransformBuffHuge[m_drawCountHuge],CBV},
				{arg_boneBuff,CBV},
				{mesh.material->texBuff[COLOR_TEX],SRV},
				{mesh.material->buff,CBV},
				{m_toonCommonParamBuff,CBV},
				{m_toonIndividualParamBuff[m_individualParamCount],CBV},
				{m_playerInfoBuffer,CBV},
			},
			arg_depth,
			arg_blendMode == AlphaBlendMode_Trans,
			static_cast<int>(arg_matArray.size()));
	}

	m_drawCountHuge++;
	m_individualParamCount++;
}

void BasicDraw::InstancingDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, std::weak_ptr<KuroEngine::Model> arg_model, std::vector<KuroEngine::Matrix>& arg_matArray, bool arg_depthWriteMask, const KuroEngine::AlphaBlendMode& arg_blendMode, const float& arg_depth, std::shared_ptr<KuroEngine::ConstantBuffer> arg_boneBuff)
{
	InstancingDraw(arg_cam,
		arg_ligMgr,
		arg_model,
		arg_matArray,
		IndividualDrawParameter::GetDefault(),
		arg_depthWriteMask,
		arg_blendMode,
		arg_depth,
		arg_boneBuff);
}

void BasicDraw::DrawEdge()
{
	using namespace KuroEngine;

	KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(m_edgePipeline);

	std::vector<RegisterDescriptorData>descDatas =
	{
		{KuroEngineDevice::Instance()->GetParallelMatProjBuff(),CBV},
		{m_renderTargetArray[DEPTH],SRV},
		{m_renderTargetArray[BRIGHT],SRV},
		{m_renderTargetArray[EDGE_COLOR],SRV},
		{m_renderTargetArray[NORMAL],SRV},
		{m_edgeShaderParamBuff,CBV},
	};
	m_spriteMesh->Render(descDatas);
}