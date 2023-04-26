#include "ShadowMapDevice.h"
#include"KuroEngineDevice.h"
#include"ForUser/PostEffect/GaussianBlur.h"
#include"ForUser/Object/Object.h"
#include"ForUser/Object/Model.h"
#include"Render/RenderObject/ModelInfo/ModelAnimator.h"

KuroEngine::ShadowMapDevice::ShadowMapDevice() :m_lightCamera("LightCamera")
{
	//シャドウマップ関連
	m_shadowMap = D3D12App::Instance()->GenerateRenderTarget(DXGI_FORMAT_R32G32_FLOAT, Color(), { 2048,2048 }, L"ShadowMap");
	m_shadowMapDepth = D3D12App::Instance()->GenerateDepthStencil({ 2048,2048 });
	auto& camTransform = m_lightCamera.GetTransform();
	camTransform.SetPos({ 0, 10, 0 });
	camTransform.SetLookAtRotate({ 0,0,0 });
	camTransform.SetRotate(Math::GetLookAtQuaternion(Vec3<float>::GetYAxis(), { 1,0,0 }));
	m_lightCamera.SetViewAngle(Angle(60));

	m_gaussianBlur = std::make_shared<GaussianBlur>(Vec2<int>(2048, 2048), DXGI_FORMAT_R32G32_FLOAT);
}

void KuroEngine::ShadowMapDevice::DrawShadowMap(const std::vector<std::weak_ptr<ModelObject>>& Models)
{
	static std::shared_ptr<GraphicsPipeline>PIPELINE;
	static std::vector<std::shared_ptr<ConstantBuffer>>TRANSFORM_BUFF;

	//パイプライン未生成
	if (!PIPELINE)
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/engine/DrawShadowMapModel.hlsl", "VSmain", "vs_6_4");
		SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/engine/DrawShadowMapModel.hlsl", "PSmain", "ps_6_4");

		//ルートパラメータ
		static std::vector<RootParam>ROOT_PARAMETER =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"ライトカメラ情報バッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"トランスフォームバッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"ボーン行列バッファ"),
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(DXGI_FORMAT_R32G32_FLOAT, AlphaBlendMode_None) };
		//パイプライン生成
		PIPELINE = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, KuroEngine::ModelMesh::Vertex::GetInputLayout(), ROOT_PARAMETER, RENDER_TARGET_INFO, { WrappedSampler(false, false) });
	}

	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(PIPELINE);

	if (TRANSFORM_BUFF.size() < Models.size())
	{
		TRANSFORM_BUFF.emplace_back(D3D12App::Instance()->GenerateConstantBuffer(sizeof(Matrix), 1, nullptr, ("DrawShadowMapMode_Transform -" + std::to_string(TRANSFORM_BUFF.size())).c_str()));
	}

	m_shadowMap->Clear(D3D12App::Instance()->GetCmdList());
	m_shadowMapDepth->Clear(D3D12App::Instance()->GetCmdList());

	//シャドウマップ書き込み
	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetRenderTargets({ m_shadowMap }, m_shadowMapDepth);

	for (int i = 0; i < Models.size(); ++i)
	{
		auto obj = Models[i].lock();
		std::shared_ptr<ConstantBuffer>boneBuff;
		if (obj->m_animator)boneBuff = obj->m_animator->GetBoneMatBuff();

		TRANSFORM_BUFF[i]->Mapping(&obj->m_transform.GetMatWorld());

		for (int meshIdx = 0; meshIdx < obj->m_model->m_meshes.size(); ++meshIdx)
		{
			const auto& mesh = obj->m_model->m_meshes[meshIdx];
			KuroEngine::KuroEngineDevice::Instance()->Graphics().ObjectRender(
				mesh.mesh->vertBuff,
				mesh.mesh->idxBuff,
				{
					{m_lightCamera.GetBuff(),CBV},
					{TRANSFORM_BUFF[i],CBV},
					{boneBuff,CBV}
				},
				0,
				true);
		}
	}

	m_gaussianBlur->Register(m_shadowMap);
}

void KuroEngine::ShadowMapDevice::DrawShadowReceiver(const std::vector<std::weak_ptr<ModelObject>>& Models, Camera& GameCamera, const AlphaBlendMode& BlendMode, int Layer)
{
	static std::shared_ptr<GraphicsPipeline>PIPELINE[AlphaBlendModeNum];
	static std::vector<std::shared_ptr<ConstantBuffer>>TRANSFORM_BUFF;

	//パイプライン未生成
	if (!PIPELINE[BlendMode])
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.m_vs = D3D12App::Instance()->CompileShader("resource/engine/DrawShadowFallModel.hlsl", "VSmain", "vs_6_4");
		SHADERS.m_ps = D3D12App::Instance()->CompileShader("resource/engine/DrawShadowFallModel.hlsl", "PSmain", "ps_6_4");

		//ルートパラメータ
		static std::vector<RootParam>ROOT_PARAMETER =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"カメラ情報バッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"トランスフォームバッファ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"カラーテクスチャ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"シャドウマップ"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"ライトカメラ情報バッファ"),
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), BlendMode) };

		//シャドウマップサンプリング用サンプラー
		auto shadowMapSampler = WrappedSampler(false, false);
		shadowMapSampler.m_sampler.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		shadowMapSampler.m_sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER;
		shadowMapSampler.m_sampler.MaxAnisotropy = 1;
		//パイプライン生成
		PIPELINE[BlendMode] = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, KuroEngine::ModelMesh::Vertex::GetInputLayout(), ROOT_PARAMETER, RENDER_TARGET_INFO, { WrappedSampler(false, false),shadowMapSampler });
	}

	KuroEngine::KuroEngineDevice::Instance()->Graphics().SetGraphicsPipeline(PIPELINE[BlendMode]);

	if (TRANSFORM_BUFF.size() < Models.size())
	{
		TRANSFORM_BUFF.emplace_back(D3D12App::Instance()->GenerateConstantBuffer(sizeof(Matrix), 1, nullptr, ("DrawShadowReceiver_Transform -" + std::to_string(TRANSFORM_BUFF.size() - 1)).c_str()));
	}


	for (int i = 0; i < Models.size(); ++i)
	{
		auto obj = Models[i].lock();

		TRANSFORM_BUFF[i]->Mapping(&obj->m_transform.GetMatWorld());

		auto model = obj->m_model;

		for (int meshIdx = 0; meshIdx < model->m_meshes.size(); ++meshIdx)
		{
			const auto& mesh = model->m_meshes[meshIdx];
			KuroEngine::KuroEngineDevice::Instance()->Graphics().ObjectRender(
				mesh.mesh->vertBuff,
				mesh.mesh->idxBuff,
				{
					{GameCamera.GetBuff(),CBV},
					{TRANSFORM_BUFF[i],CBV},
					{mesh.material->texBuff[COLOR_TEX],SRV},
					{m_gaussianBlur->GetResultTex(),SRV},
					{m_lightCamera.GetBuff(),CBV}
				},
				Layer,
				true);
		}
	}
}

void KuroEngine::ShadowMapDevice::SetPos(const Vec3<float> Pos)
{
	m_lightCamera.GetTransform().SetPos(Pos);
}

void KuroEngine::ShadowMapDevice::SetTarget(const Vec3<float> Target)
{
	m_lightCamera.GetTransform().SetLookAtRotate(Target);
}

void KuroEngine::ShadowMapDevice::SetBlurPower(const float& BlurPower)
{
	m_gaussianBlur->SetBlurPower(BlurPower);
}
