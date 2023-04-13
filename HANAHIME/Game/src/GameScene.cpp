#include "GameScene.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"ForUser/Debugger.h"
#include"OperationConfig.h"
#include"DebugController.h"
#include"Stage/StageManager.h"

#include"ForUser/JsonData.h"
#include"Graphics/BasicDraw.h"
#include"FrameWork/UsersInput.h"

GameScene::GameScene() :m_fireFlyStage(m_particleRender.GetStackBuffer())
{
	m_ddsTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/test.dds");
	m_pngTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/test.png");

	KuroEngine::Vec3<float>dir = { 0.0f,-1.0f,0.0f };
	m_dirLigArray.emplace_back();
	m_dirLigArray.back().SetDir(dir.GetNormal());

	dir = { 1.0f,-0.5f,0.0f };
	m_dirLigArray.emplace_back();
	m_dirLigArray.back().SetDir(dir.GetNormal());


	for (auto& dirLig : m_dirLigArray)
	{
		m_ligMgr.RegisterDirLight(&dirLig);
	}
	m_ligMgr.RegisterPointLight(m_player.GetPointLig());

	auto backBuffTarget = KuroEngine::D3D12App::Instance()->GetBackBuffRenderTarget();
	m_fogPostEffect = std::make_shared<KuroEngine::Fog>(backBuffTarget->GetGraphSize(), backBuffTarget->GetDesc().Format);
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
	m_fogPostEffect.get(),
		});

	m_debugCam.Init({ 0,5,-10 });

	m_player.Init(StageManager::Instance()->GetPlayerSpawnTransform());

	m_grass.Init();

	m_waterPaintBlend.Init();
	m_title.Init();

	//タイトル画面に戻る
	StageManager::Instance()->SetStage();
}

void GameScene::OnUpdate()
{
	m_particleRender.InitCount();

	//デバッグ用
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_I))
	{
		this->Finalize();
		this->Initialize();
	}

	//デバッグモード更新
	DebugController::Instance()->Update();

	m_nowCam = m_player.GetCamera().lock();
	//タイトル画面モード
	if (!m_title.IsFinish())
	{
		m_nowCam = m_title.GetCamera().lock();
	}
	//ホームでの演出
	if (m_movieCamera.IsStart())
	{
		m_nowCam = m_movieCamera.GetCamera().lock();
	}
	if (DebugController::Instance()->IsActive())
	{
		m_debugCam.Move();
		m_nowCam = m_debugCam;
	}

	StageManager::Instance()->Update(m_player);

	m_player.Update(StageManager::Instance()->GetNowStage());

	m_grass.Update(1.0f, m_player.GetTransform(), m_player.GetCamera(), m_player.GetGrassPosScatter(), m_waterPaintBlend);

	//ホームでの処理----------------------------------------
	m_title.Update(&m_player.GetCamera().lock()->GetTransform());

	//ステージ選択
	int stageNum = -1;
	if (m_title.IsFinish())
	{
		stageNum = m_stageSelect.GetStageNumber(m_player.GetTransform().GetPos());
	}
	else
	{
		stageNum = m_title.GetStageNum();
	}


	//ステージ移動時の初期化
	if (stageNum != -1)
	{
		m_stageNum = stageNum;
		m_gateSceneChange.Start();
	}

	if (m_gateSceneChange.IsHide())
	{
		StageManager::Instance()->SetStage(m_stageNum);
		m_player.Init(StageManager::Instance()->GetPlayerSpawnTransform());
		//パズル画面からシーンチェンジしたらカメラモードを切り替える
		if (!m_title.IsFinish())
		{
			m_title.FinishTitle();
			OperationConfig::Instance()->SetActive(true);
		}
	}
	

	m_stageSelect.Update();
	//ホームでの処理----------------------------------------

	m_gateSceneChange.Update();

	m_movieCamera.Update();

	m_fireFlyStage.ComputeUpdate();



	BasicDraw::Instance()->Update(m_player.GetTransform().GetPosWorld(), *m_nowCam);
}

void GameScene::OnDraw()
{
	using namespace KuroEngine;
	static auto targetSize = D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize();
	static auto ds = D3D12App::Instance()->GenerateDepthStencil(targetSize);

	//レンダーターゲットのクリアとセット
	BasicDraw::Instance()->RenderTargetsClearAndSet(ds);

	//ステージ描画
	StageManager::Instance()->Draw(*m_nowCam, m_ligMgr);

	Transform transform;
	transform.SetPos({ -0.5f,0,0 });
	DrawFunc3D::DrawNonShadingPlane(
		m_ddsTex,
		transform,
		*m_nowCam);

	transform.SetPos({ 0.5f,0,0 });
	DrawFunc3D::DrawNonShadingPlane(
		m_pngTex,
		transform,
		*m_nowCam);

	m_player.Draw(*m_nowCam, m_ligMgr, DebugController::Instance()->IsActive());

	m_grass.Draw(*m_nowCam, m_ligMgr);

	m_stageSelect.Draw(*m_nowCam, m_ligMgr);

	//m_movieCamera.DebugDraw(*m_nowCam, m_ligMgr);


	//m_canvasPostEffect.Execute();
	BasicDraw::Instance()->DrawEdge();

	//KuroEngineDevice::Instance()->Graphics().ClearDepthStencil(ds);
	//m_waterPaintBlend.Register(main, *nowCamera, ds);
	//m_vignettePostEffect.Register(m_waterPaintBlend.GetResultTex());

	if (m_title.IsStartOP())
	{
		m_particleRender.Draw(*m_nowCam);
	}



	m_fogPostEffect->Register(
		BasicDraw::Instance()->GetRenderTarget(BasicDraw::MAIN),
		BasicDraw::Instance()->GetRenderTarget(BasicDraw::DEPTH),
		BasicDraw::Instance()->GetRenderTarget(BasicDraw::BRIGHT)
	);

	m_vignettePostEffect.Register(m_fogPostEffect->GetResultTex());


	m_title.Draw(*m_nowCam, m_ligMgr);



	m_gateSceneChange.Draw();


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

