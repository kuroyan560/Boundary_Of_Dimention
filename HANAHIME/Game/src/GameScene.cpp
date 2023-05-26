#include "GameScene.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"ForUser/Debugger.h"
#include"OperationConfig.h"
#include"SoundConfig.h"
#include"DebugController.h"
#include"Stage/StageManager.h"

#include"ForUser/JsonData.h"
#include"Graphics/BasicDraw.h"
#include"FrameWork/UsersInput.h"
#include"Plant/GrowPlantLight.h"
#include"TimeScaleMgr.h"

#include"FrameWork/Importer.h"
#include"Stage/Enemy/EnemyDataReferenceForCircleShadow.h"

#include"Stage/GateManager.h"

#include"HUD/InGameUI.h"

GameScene::GameScene() :m_fireFlyStage(m_particleRender.GetStackBuffer()), tutorial(m_particleRender.GetStackBuffer()), m_1flameStopTimer(30), m_goal(m_particleRender.GetStackBuffer())
{
	m_ddsTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/test.dds");
	m_pngTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/test.png");

	KuroEngine::Vec3<float>dir = { 0.0f,-1.0f,0.0f };
	m_dirLigArray.emplace_back();
	m_dirLigArray.back().SetDir(dir.GetNormal());
	m_dirLigArray.back().SetColor(KuroEngine::Color(0.8f, 0.8f, 0.8f, 1.0f));

	dir = { 1.0f,-0.5f,0.0f };
	m_dirLigArray.emplace_back();
	m_dirLigArray.back().SetDir(dir.GetNormal());
	m_dirLigArray.back().SetColor(KuroEngine::Color(0.8f, 0.8f, 0.8f, 1.0f));

	for (auto &dirLig : m_dirLigArray)
	{
		m_ligMgr.RegisterDirLight(&dirLig);
	}
	m_ligMgr.RegisterPointLight(m_player.GetPointLig());

	m_hemiLig.SetSkyColor(KuroEngine::Color(107, 196, 198, 255));
	m_hemiLig.SetGroundColor(KuroEngine::Color(0, 0, 0, 1));
	m_ligMgr.RegisterHemiSphereLight(&m_hemiLig);

	auto backBuffTarget = KuroEngine::D3D12App::Instance()->GetBackBuffRenderTarget();
	m_fogPostEffect = std::make_shared<KuroEngine::Fog>(backBuffTarget->GetGraphSize(), backBuffTarget->GetDesc().Format);

	//起動時にタイトル画面を出す為に必要
	m_eTitleMode = TITLE_SELECT;

	//音声の読み込み
	SoundConfig::Instance();
	CameraData::Instance()->RegistCameraData("");

}


void GameScene::GameInit()
{
	SoundConfig::Instance()->Play(SoundConfig::BGM_IN_GAME);
	GrowPlantLight::ResetRegisteredLight();
	StageManager::Instance()->SetStage(m_stageNum);

	if (GateManager::Instance()->IsEnter())
	{
		m_player.Init(StageManager::Instance()->GetGateTransform(GateManager::Instance()->GetDestStageNum(), GateManager::Instance()->GetDestGateID()));
	}
	else
	{
		m_player.Respawn(StageManager::Instance()->GetPlayerSpawnTransform());
	}
	SoundConfig::Instance()->Init();
	GateManager::Instance()->Init();
	m_opeInfoUI.Init();
	InGameUI::Init();
	m_stageInfoUI.Init(m_stageNum, StageManager::Instance()->GetStarCoinNum());
	m_pauseUI.Init();
	OperationConfig::Instance()->SetInGameOperationActive(true);
}

void GameScene::Retry()
{
	m_gateSceneChange.Start();
}

void GameScene::GoBackTitle()
{
	m_gateSceneChange.Start();
	m_eTitleMode = TITLE_SELECT;
	m_gobackTitleFlag = true;
}

void GameScene::OnInitialize()
{
	GrowPlantLight::ResetRegisteredLight();

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
	SoundConfig::Instance(),
		});

	m_debugCam.Init({ 0,5,-10 });

	m_player.Init(StageManager::Instance()->GetPlayerSpawnTransform());

	m_grass.Init();

	m_waterPaintBlend.Init();

	m_title.Init(m_eTitleMode);
	//ゲーム画面からパズルモードに戻る場合にパズルモードとして初期化した後、再び選択できるようSELECTを入れる
	m_eTitleMode = TITLE_SELECT;

	m_pauseUI.Init();
	m_gobackTitleFlag = false;
	m_deadFlag = false;
}

void GameScene::OnUpdate()
{
	m_particleRender.InitCount();

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
	//ゴール時の演出
	if (StageManager::Instance()->IsClearNowStage() && m_1flameStopTimer.IsTimeUp() && (m_title.IsFinish() || m_title.IsStartOP()))
	{
		m_goal.Start();
		if (m_goal.ChangeCamera())
		{
			m_nowCam = m_goal.GetCamera().lock();
		}
		OperationConfig::Instance()->SetAllInputActive(false);
		m_clearFlag = true;
	}

	if (DebugController::Instance()->IsActive())
	{
		m_debugCam.Move();
		m_nowCam = m_debugCam;
	}

	m_grass.Update(1.0f, m_player.GetIsOverheat(), m_player.GetTransform(), m_player.GetCamera(), m_player.GetGrowPlantLight().m_influenceRange, StageManager::Instance()->GetNowStage(), m_player.GetIsAttack());

	//ホームでの処理----------------------------------------
	if (!m_title.IsFinish() && !m_title.IsStartOP())
	{
		m_title.Update(&m_player.GetCamera().lock()->GetTransform(), m_nowCam);
	}

	//ステージ選択
	int stageNum = -1;
	if (m_title.IsFinish() && GateManager::Instance()->IsEnter() && !m_gateSceneChange.IsActive())
	{
		//stageNum = m_stageSelect.GetStageNumber(m_player.GetTransform().GetPos());
		stageNum = GateManager::Instance()->GetDestStageNum();
	}
	else
	{
		stageNum = m_title.GetStageNum();
	}

	//ゲームクリア演出を終えたら遷移開始
	if (m_goal.IsEnd())
	{
		stageNum = 1;
	}

	//ポーズ画面
	if (OperationConfig::Instance()->GetOperationInput(OperationConfig::PAUSE, OperationConfig::ON_TRIGGER))
	{
		m_pauseUI.SetInverseActive();
	}

	if (m_player.GetIsFinishDeathAnimation() && !m_deadFlag)
	{
		//Retry();
		m_pauseUI.SetInverseActive();
		m_deadFlag = true;
	}

	//ステージ移動時の初期化
	if (stageNum != -1)
	{
		m_stageNum = stageNum;
		m_gateSceneChange.Start();
	}

	if (m_gateSceneChange.IsHide())
	{
		m_deadFlag = false;
		//パズル画面からシーンチェンジしたらカメラモードを切り替える
		if (!m_title.IsFinish())
		{
			m_title.FinishTitle();
			OperationConfig::Instance()->SetAllInputActive(true);
		}
		else
		{
			//タイトル画面に戻る
			SoundConfig::Instance()->Play(SoundConfig::BGM_TITLE);
			StageManager::Instance()->SetStage(std::clamp(GateManager::Instance()->GetDestStageNum() - 1, 0, 1000));
		}

		if (m_gobackTitleFlag)
		{
			m_title.Init(m_eTitleMode);
			m_gobackTitleFlag = false;
		}

		GameInit();
		m_goal.Init(StageManager::Instance()->GetGoalTransform(), StageManager::Instance()->GetGoalModel());

		//ゲームクリア時に遷移する処理
		if (m_clearFlag)
		{
			m_eTitleMode = TITLE_PAZZLE;
			this->Initialize();
			m_clearFlag = false;

			m_1flameStopTimer.Reset();

			m_title.Clear();
		}
	}
	m_1flameStopTimer.UpdateTimer();

	//ゲームシーンでのみ使う物
	if (m_title.IsFinish() || m_title.IsStartOP())
	{
		m_player.Update(StageManager::Instance()->GetNowStage());
		m_goal.Update(&m_player.GetTransform());
		//ステージ選択画面ではギミックを作動させない
		StageManager::Instance()->Update(m_player);
	}
	else
	{
		m_player.GetPointLig()->SetActive(false);
		m_player.DisactiveLight();
	}

	m_gateSceneChange.Update();
	m_movieCamera.Update();
	m_fireFlyStage.ComputeUpdate(m_player.GetTransform().GetPos());

	BasicDraw::Instance()->Update(m_player.GetTransform().GetPosWorld(), *m_nowCam);

	//敵用丸影を更新
	EnemyDataReferenceForCircleShadow::Instance()->UpdateGPUData();

	//UI更新
	InGameUI::Update(TimeScaleMgr::s_inGame.GetTimeScale());
	m_opeInfoUI.Update(TimeScaleMgr::s_inGame.GetTimeScale());
	m_stageInfoUI.Update(TimeScaleMgr::s_inGame.GetTimeScale(), StageManager::Instance()->GetStarCoinNum());
	m_pauseUI.Update(this);

	GateManager::Instance()->FrameEnd();
}

void GameScene::OnDraw()
{
	using namespace KuroEngine;
	static auto targetSize = D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize();
	static auto ds = D3D12App::Instance()->GenerateDepthStencil(targetSize);

	//レンダーターゲットのクリアとセット
	BasicDraw::Instance()->RenderTargetsClearAndSet(ds);

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

	//m_movieCamera.DebugDraw(*m_nowCam, m_ligMgr);

	//KuroEngineDevice::Instance()->Graphics().ClearDepthStencil(ds);
	//m_waterPaintBlend.Register(main, *nowCamera, ds);
	//m_vignettePostEffect.Register(m_waterPaintBlend.GetResultTex());

	//if (m_title.IsStartOP())

	//tutorial.Draw(*m_nowCam);

	if (m_title.IsFinish() || m_title.IsStartOP())
	{
		m_goal.Draw(*m_nowCam);
		m_player.Draw(*m_nowCam, ds, m_ligMgr, DebugController::Instance()->IsActive());
		m_grass.Draw(*m_nowCam, m_ligMgr, m_player.GetGrowPlantLight().m_influenceRange, m_player.GetIsAttack());
	}

	//ステージ描画
	StageManager::Instance()->Draw(*m_nowCam, m_ligMgr);

	//プレイヤーのパーティクル描画
	if (m_title.IsFinish() || m_title.IsStartOP())
	{
		m_player.DrawParticle(*m_nowCam, m_ligMgr);
	}

	m_particleRender.Draw(*m_nowCam);

	//m_canvasPostEffect.Execute();
	BasicDraw::Instance()->DrawEdge(m_nowCam->GetViewMat(), m_nowCam->GetProjectionMat(), ds, m_player.GetIsOverheat());

	m_lightBloomDevice.Draw(BasicDraw::Instance()->GetRenderTarget(BasicDraw::EMISSIVE), BasicDraw::Instance()->GetRenderTarget(BasicDraw::MAIN));


	m_fogPostEffect->Register(
		BasicDraw::Instance()->GetRenderTarget(BasicDraw::MAIN),
		BasicDraw::Instance()->GetRenderTarget(BasicDraw::DEPTH),
		BasicDraw::Instance()->GetRenderTarget(BasicDraw::BRIGHT),
		m_title.IsFinish() || m_title.IsStartOP()
	);

	m_vignettePostEffect.Register(m_fogPostEffect->GetResultTex());

	if (m_title.IsFinish() || m_title.IsStartOP())
	{
		//ポーズ画面
		m_pauseUI.Draw(StarCoin::GetFlowerSum());

		//ポーズ画面でなければ
		if (!m_pauseUI.IsActive())
		{
			m_player.DrawUI(*m_nowCam);
			StageManager::Instance()->DrawUI(*m_nowCam, m_player.GetTransform().GetPosWorld());
			m_opeInfoUI.Draw();
			m_stageInfoUI.Draw(StageManager::Instance()->ExistStarCoinNum(), StageManager::Instance()->GetStarCoinNum());
		}
	}

	m_title.Draw(*m_nowCam, m_ligMgr);

	m_gateSceneChange.Draw();


	KuroEngineDevice::Instance()->Graphics().SetRenderTargets(
		{
			D3D12App::Instance()->GetBackBuffRenderTarget(),
		});

	m_vignettePostEffect.DrawResult(AlphaBlendMode_None);
	m_goal.Draw2D();
}

void GameScene::OnImguiDebug()
{
	if (!DebugController::Instance()->IsActive())return;

	KuroEngine::Debugger::Draw();
}

void GameScene::OnFinalize()
{
	KuroEngine::Debugger::ClearRegister();
	//ゲーム終了時はゴールを使わない
	m_goal.Finalize();
}

