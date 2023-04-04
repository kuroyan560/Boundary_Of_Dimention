#include "GameScene.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"ForUser/Debugger.h"
#include"OperationConfig.h"
#include"DebugController.h"
#include"Stage/StageManager.h"

#include"ForUser/JsonData.h"
#include"Graphics/BasicDraw.h"

GameScene::GameScene()
{
	m_ddsTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/test.dds");
	m_pngTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/test.png");

	KuroEngine::Vec3<float>dir = { 0.0f,-1.0f,0.0f };
	m_dirLig.SetDir(dir.GetNormal());
	m_ligMgr.RegisterDirLight(&m_dirLig);
	m_ligMgr.RegisterPointLight(m_player.GetPointLig());

}


void GameScene::OnInitialize()
{
	KuroEngine::Debugger::Register({
	OperationConfig::Instance(),
	&m_player,
	m_player.GetCameraControllerDebugger(),
	StageManager::Instance(),
	BasicDraw::Instance(),
	&m_vignettePostEffect,
	&m_waterPaintBlend,
	&m_ligMgr,
		});

	m_debugCam.Init({ 0,5,-10 });

	KuroEngine::Transform playerInitTransform;
	playerInitTransform.SetPos({ 30.0f,50.0f,-45 });
	m_player.Init(playerInitTransform);

	m_grass.Init();

	m_waterPaintBlend.Init();
}

void GameScene::OnUpdate()
{
	//デバッグ用
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_I))
	{
		this->Finalize();
		this->Initialize();
	}

	//デバッグモード更新
	DebugController::Instance()->Update();

	if (DebugController::Instance()->IsActive())m_debugCam.Move();

	m_player.Update(StageManager::Instance()->GetNowStage());

	m_grass.Update(1.0f, m_player.GetTransform().GetPos(), m_player.GetTransform().GetRotate(), m_player.GetCamera().lock()->GetTransform(), m_player.GetGrassPosScatter(), m_waterPaintBlend);
	//m_grass.Plant(m_player.GetTransform(), m_player.GetGrassPosScatter(), m_waterPaintBlend);


	//ホームでの処理----------------------------------------

	//ステージ選択
	int stageNum = stageSelect.GetStageNumber(m_player.GetTransform().GetPos());
	//ステージ移動時の初期化
	if (stageNum != -1)
	{
		StageManager::Instance()->SetStage(stageNum);
		KuroEngine::Transform playerInitTransform;
		playerInitTransform.SetPos({ 30.0f,50.0f,-45 });
		m_player.Init(playerInitTransform);
	}
	stageSelect.Update();

	//ホームでの処理----------------------------------------



	BasicDraw::Instance()->Update(m_player.GetTransform().GetPosWorld());
}

void GameScene::OnDraw()
{
	using namespace KuroEngine;
	static auto targetSize = D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize();
	static auto ds = D3D12App::Instance()->GenerateDepthStencil(targetSize);

	static auto main = D3D12App::Instance()->GenerateRenderTarget(
		D3D12App::Instance()->GetBackBuffFormat(),
		Color(0.0f, 0.0f, 0.0f, 0.0f),
		targetSize,
		L"MainRenderTarget");
	static auto emissiveMap = D3D12App::Instance()->GenerateRenderTarget(
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		Color(0.0f, 0.0f, 0.0f, 1.0f),
		targetSize, L"EmissiveMap");
	static auto depthMap = D3D12App::Instance()->GenerateRenderTarget(
		DXGI_FORMAT_R16_FLOAT,
		Color(FLT_MAX, 0.0f, 0.0f, 0.0f),
		targetSize, L"DepthMap");
	static auto normalMap = D3D12App::Instance()->GenerateRenderTarget(
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		Color(0.0f, 0.0f, 0.0f, 0.0f),
		targetSize, L"NormalMap");
	static auto edgeColMap = D3D12App::Instance()->GenerateRenderTarget(
		D3D12App::Instance()->GetBackBuffFormat(),
		Color(0.0f, 0.0f, 0.0f, 1.0f),
		targetSize, L"EdgeColorMap");

	//レンダーターゲットのクリア
	KuroEngineDevice::Instance()->Graphics().ClearRenderTarget(main);
	KuroEngineDevice::Instance()->Graphics().ClearRenderTarget(emissiveMap);
	KuroEngineDevice::Instance()->Graphics().ClearRenderTarget(depthMap);
	KuroEngineDevice::Instance()->Graphics().ClearRenderTarget(normalMap);
	KuroEngineDevice::Instance()->Graphics().ClearRenderTarget(edgeColMap);
	KuroEngineDevice::Instance()->Graphics().ClearDepthStencil(ds);

	//レンダーターゲットをセット
	KuroEngineDevice::Instance()->Graphics().SetRenderTargets(
		{
			main,
			emissiveMap,
			depthMap,
			normalMap,
			edgeColMap
		},
		ds
	);

	auto nowCamera = m_player.GetCamera().lock();
	if (DebugController::Instance()->IsActive())nowCamera = m_debugCam;

	//ステージ描画
	StageManager::Instance()->Draw(*nowCamera, m_ligMgr);

	Transform transform;
	transform.SetPos({ -0.5f,0,0 });
	DrawFunc3D::DrawNonShadingPlane(
		m_ddsTex,
		transform,
		*nowCamera);

	transform.SetPos({ 0.5f,0,0 });
	DrawFunc3D::DrawNonShadingPlane(
		m_pngTex,
		transform,
		*nowCamera);

	m_player.Draw(*nowCamera, m_ligMgr, DebugController::Instance()->IsActive());

	m_grass.Draw(*nowCamera, m_ligMgr);

	stageSelect.Draw(*nowCamera, m_ligMgr);

	//m_canvasPostEffect.Execute();

	BasicDraw::Instance()->DrawEdge(depthMap, normalMap, edgeColMap);

	//KuroEngineDevice::Instance()->Graphics().ClearDepthStencil(ds);
	//m_waterPaintBlend.Register(main, *nowCamera, ds);
	//m_vignettePostEffect.Register(m_waterPaintBlend.GetResultTex());

	m_vignettePostEffect.Register(main);

	KuroEngineDevice::Instance()->Graphics().SetRenderTargets(
		{
			D3D12App::Instance()->GetBackBuffRenderTarget(),
		});

	m_vignettePostEffect.DrawResult(AlphaBlendMode_None);
}

void GameScene::OnImguiDebug()
{
	if (!DebugController::Instance()->IsActive())return;

	KuroEngine::Debugger::Draw();
}

void GameScene::OnFinalize()
{
	KuroEngine::Debugger::ClearRegister();
}

