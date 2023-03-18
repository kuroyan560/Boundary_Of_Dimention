#include "BasicDraw.h"
#include"ForUser/Object/Model.h"
#include"KuroEngineDevice.h"
#include"ForUser/Object/Object.h"
#include"Render/RenderObject/ModelInfo/ModelAnimator.h"
#include"Render/CubeMap.h"
#include"Render/RenderObject/Camera.h"
#include"Render/RenderObject/LightManager.h"
#include"Render/RenderObject/SpriteMesh.h"

BasicDraw::BasicDraw() :KuroEngine::Debugger("BasicDraw")
{
	AddCustomParameter("BrightThresholdLow", { "Toon","BrightThreshold","Low" }, PARAM_TYPE::FLOAT,
		&m_toonCommonParam.m_brightThresholdLow, "Toon", true, 0.0f, 1.0f);
	AddCustomParameter("BrightThresholdRange", { "Toon","BrightThreshold","Range" }, PARAM_TYPE::FLOAT,
		&m_toonCommonParam.m_brightThresholdRange, "Toon");
	AddCustomParameter("LimThreshold", { "Toon","LimThreshold" }, PARAM_TYPE::FLOAT,
		&m_toonCommonParam.m_limThreshold, "Toon");

	AddCustomParameter("DepthDifferenceThreshold", { "Edge","DepthDifferenceThreshold" }, PARAM_TYPE::FLOAT,
		&m_edgeShaderParam.m_depthThreshold, "Edge");

	auto& defaultParam = IndividualDrawParameter::GetDefault();
	AddCustomParameter("FillColor", { "DefaultDrawParam","FillColor" }, PARAM_TYPE::COLOR,
		&defaultParam.m_fillColor, "DefaultDrawParam");
	AddCustomParameter("BrightMulColor", { "DefaultDrawParam","BrightMulColor" }, PARAM_TYPE::COLOR,
		&defaultParam.m_brightMulColor, "DefaultDrawParam");
	AddCustomParameter("DarkMulColor", { "DefaultDrawParam","DarkMulColor" }, PARAM_TYPE::COLOR,
		&defaultParam.m_darkMulColor, "DefaultDrawParam");
	AddCustomParameter("LimBrightColor", { "DefaultDrawParam","LimBrightColor" }, PARAM_TYPE::COLOR,
		&defaultParam.m_limBrightColor, "DefaultDrawParam");
	AddCustomParameter("EdgeColor", { "DefaultDrawParam","EdgeColor" }, PARAM_TYPE::COLOR,
		&defaultParam.m_edgeColor, "DefaultDrawParam");
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

	//通常描画パイプライン生成
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/user/shaders/BasicShader.hlsl", "VSmain", "vs_6_4");
		SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/user/shaders/BasicShader.hlsl", "PSmain", "ps_6_4");

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

		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO =
		{
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_Trans),	//通常描画
			RenderTargetInfo(DXGI_FORMAT_R32G32B32A32_FLOAT, AlphaBlendMode_Trans),	//エミッシブマップ
			RenderTargetInfo(DXGI_FORMAT_R32_FLOAT, AlphaBlendMode_None),	//深度マップ
			RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_None),	//エッジカラーマップ
		};
		//パイプライン生成
		m_drawPipeline = D3D12App::Instance()->GenerateGraphicsPipeline(
			PIPELINE_OPTION, 
			SHADERS,
			ModelMesh::Vertex::GetInputLayout(), 
			ROOT_PARAMETER, 
			RENDER_TARGET_INFO,
			{ WrappedSampler(true, true) });
	}

	//事前にバッファを用意
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
	}

	//トゥーンシェーダー用のバッファを用意
	m_toonCommonParamBuff = D3D12App::Instance()->GenerateConstantBuffer(
		sizeof(m_toonCommonParam),
		1, 
		&m_toonCommonParam, 
		"BasicDraw - ToonCommonParameter");

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
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"エッジカラーマップ"),
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
}

void BasicDraw::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform& arg_transform, const IndividualDrawParameter& arg_toonParam, std::shared_ptr<KuroEngine::ConstantBuffer>arg_boneBuff)
{
	using namespace KuroEngine;

	KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(m_drawPipeline);

	//トランスフォームバッファ送信
	if (m_drawTransformBuff.size() < (m_drawCount + 1))
	{
		m_drawTransformBuff.emplace_back(D3D12App::Instance()->GenerateConstantBuffer(sizeof(Matrix), 1, nullptr, ("BasicDraw - Transform -" + std::to_string(m_drawCount)).c_str()));
	}
	m_drawTransformBuff[m_drawCount]->Mapping(&arg_transform.GetWorldMat());

	//トゥーンの個別パラメータバッファ送信
	if (m_toonIndividualParamBuff.size() < (m_drawCount + 1))
	{
		m_toonIndividualParamBuff.emplace_back(D3D12App::Instance()->GenerateConstantBuffer(sizeof(IndividualDrawParameter), 1, nullptr, ("BasicDraw - IndividualDrawParameter -" + std::to_string(m_drawCount)).c_str()));
	}
	m_toonIndividualParamBuff[m_drawCount]->Mapping(&arg_toonParam);

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
				{m_toonIndividualParamBuff[m_drawCount],CBV},
			},
			arg_transform.GetPos().z,
			true);
	}

	m_drawCount++;
}

void BasicDraw::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform& arg_transform, std::shared_ptr<KuroEngine::ConstantBuffer> arg_boneBuff)
{
	BasicDraw::Draw(arg_cam,arg_ligMgr, arg_model, arg_transform, IndividualDrawParameter::GetDefault(), arg_boneBuff);
}

void BasicDraw::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, const std::weak_ptr<KuroEngine::ModelObject> arg_modelObj, const IndividualDrawParameter& arg_toonParam)
{
	using namespace KuroEngine;

	auto obj = arg_modelObj.lock();
	//ボーン行列バッファ取得（アニメーターがnullptrなら空）
	auto model = obj->m_model;
	std::shared_ptr<ConstantBuffer>boneBuff;
	if (obj->m_animator)boneBuff = obj->m_animator->GetBoneMatBuff();
	Draw(arg_cam, arg_ligMgr, model, obj->m_transform, arg_toonParam, boneBuff);
}

void BasicDraw::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, const std::weak_ptr<KuroEngine::ModelObject> arg_modelObj)
{
	Draw(arg_cam, arg_ligMgr, arg_modelObj, IndividualDrawParameter::GetDefault());
}

void BasicDraw::DrawEdge(std::shared_ptr<KuroEngine::TextureBuffer> arg_depthMap, std::shared_ptr<KuroEngine::TextureBuffer>arg_edgeColorMap)
{
	using namespace KuroEngine;

	KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(m_edgePipeline);

	std::vector<RegisterDescriptorData>descDatas =
	{
		{KuroEngineDevice::Instance()->GetParallelMatProjBuff(),CBV},
		{arg_depthMap,SRV},
		{arg_edgeColorMap,SRV},
		{m_edgeShaderParamBuff,CBV},
	};
	m_spriteMesh->Render(descDatas);
}