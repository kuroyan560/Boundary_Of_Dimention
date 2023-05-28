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

GameScene::GameScene() :m_fireFlyStage(m_particleRender.GetStackBuffer()), tutorial(m_particleRender.GetStackBuffer()), m_goal(m_particleRender.GetStackBuffer())
{
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

	//音声の読み込み
	SoundConfig::Instance();
	CameraData::Instance()->RegistCameraData("");

	//スカイドーム
	m_skyDomeModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Skydome.glb");
	m_skyDomeDrawParam.m_edgeColor.m_a = 0.0f;
	m_skyDomeTransform.SetScale(StageManager::Instance()->GetSkyDomeScaling());
}


void GameScene::GameInit()
{
	SoundConfig::Instance()->Play(SoundConfig::BGM_IN_GAME);
	GrowPlantLight::ResetRegisteredLight();
	StageManager::Instance()->SetStage(m_stageNum);

	m_player.Respawn(m_playerInitTransform);

	SoundConfig::Instance()->Init();
	GateManager::Instance()->Init();
	m_opeInfoUI.Init();
	InGameUI::Init();
	m_stageInfoUI.Init(m_stageNum, StageManager::Instance()->GetStarCoinNum());
	m_pauseUI.Init();
	OperationConfig::Instance()->SetInGameOperationActive(true);
	m_checkPointPillar.Init();

	//std::vector<KuroEngine::Transform> testTransform;
	//KuroEngine::Transform buff;
	//buff.SetPos(KuroEngine::Vec3<float>(0, 0, 0));
	//testTransform.emplace_back(buff);
	//buff.SetPos(KuroEngine::Vec3<float>(0, -100, 100));
	//testTransform.emplace_back(buff);
	//FastTravel::Instance()->Init(testTransform);

}

void GameScene::Retry()
{
	StartGame(m_stageNum, CheckPoint::GetLatestVistTransform(StageManager::Instance()->GetStartPointTransform()));
}

void GameScene::StartGame(int arg_stageNum, KuroEngine::Transform arg_playerInitTransform)
{
	m_stageNum = arg_stageNum;
	m_gateSceneChange.Start();
	m_nextScene = SCENE_IN_GAME;
	m_playerInitTransform = arg_playerInitTransform;
}

void GameScene::GoBackTitle()
{
	m_gateSceneChange.Start();
	m_nextScene = SCENE_TITLE;
}

void GameScene::ActivateFastTravel()
{
	m_fastTravel.Activate();

	//ファストトラベル
	std::vector<std::vector<KuroEngine::Transform>>transformArray;
	transformArray.emplace_back();
	transformArray.back().emplace_back();
	transformArray.back().back().SetPos({ -100,5,30 });
	transformArray.back().emplace_back();
	transformArray.back().back().SetPos({ 22,10,-500 });
	transformArray.back().emplace_back();
	transformArray.back().back().SetPos({ -52,-5,250 });

	transformArray.emplace_back();
	transformArray.back().emplace_back();
	transformArray.back().back().SetPos({ -100,5,30 });
	transformArray.back().emplace_back();
	transformArray.back().back().SetPos({ 22,10,-500 });
	transformArray.back().emplace_back();
	transformArray.back().back().SetPos({ -52,-5,250 });
	m_fastTravel.Init(transformArray, 0, 0);
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

	m_player.Init(StageManager::Instance()->GetStartPointTransform());

	m_grass.Init();

	m_waterPaintBlend.Init();

	m_title.Init();

	m_pauseUI.Init();
	m_deadFlag = false;
}

void GameScene::OnUpdate()
{
	m_particleRender.InitCount();

	//デバッグモード更新
	DebugController::Instance()->Update();

	//ファストトラベル
	if (m_fastTravel.IsActive())
	{
		m_nowCam = m_fastTravel.GetCamera();
	}
	//タイトル画面
	else if (m_nowScene == SCENE_TITLE)
	{
		m_nowCam = m_title.GetCamera().lock();
		m_title.Update(&m_player.GetCamera().lock()->GetTransform(), m_nowCam, this);
	}
	//インゲーム
	else if (m_nowScene == SCENE_IN_GAME)
	{
		//通常のプレイヤーカメラ
		m_nowCam = m_player.GetCamera().lock();
		//クリア時のカメラ
		if (StageManager::Instance()->IsClearNowStage())
		{
			m_goal.Start();
			if (m_goal.ChangeCamera())
			{
				m_nowCam = m_goal.GetCamera().lock();
			}
			OperationConfig::Instance()->SetAllInputActive(false);
			m_clearFlag = true;
		}

		//ゲートをくぐった
		if (GateManager::Instance()->IsEnter() && !m_gateSceneChange.IsActive())
		{
			StartGame(GateManager::Instance()->GetDestStageNum(), 
				StageManager::Instance()->GetGateTransform(GateManager::Instance()->GetDestStageNum(), GateManager::Instance()->GetDestGateID()));
		}
		//ゲームクリア演出を終えたら遷移開始
		if (m_goal.IsEnd())
		{
			m_gateSceneChange.Start();
			GoBackTitle();
		}

		//ポーズ画面
		if (OperationConfig::Instance()->GetOperationInput(OperationConfig::PAUSE, OperationConfig::ON_TRIGGER))
		{
			m_pauseUI.SetInverseActive();
		}

		//UI更新
		InGameUI::Update(TimeScaleMgr::s_inGame.GetTimeScale());
		m_opeInfoUI.Update(TimeScaleMgr::s_inGame.GetTimeScale());
		m_stageInfoUI.Update(TimeScaleMgr::s_inGame.GetTimeScale(), StageManager::Instance()->GetStarCoinNum());
		m_pauseUI.Update(this);
	}

	if (DebugController::Instance()->IsActive())
	{
		m_debugCam.Move();
		m_nowCam = m_debugCam;
	}

	m_grass.Update(1.0f, m_player.GetIsOverheat(), m_player.GetTransform(), m_player.GetCamera(), m_player.GetGrowPlantLight().m_influenceRange, StageManager::Instance()->GetNowStage(), m_player.GetIsAttack(), m_player.GetMoveSpeed());


	if (m_player.GetIsFinishDeathAnimation() && !m_deadFlag)
	{
		//Retry();
		m_pauseUI.SetInverseActive();
		m_deadFlag = true;
	}

	if (m_gateSceneChange.IsHide())
	{
		m_deadFlag = false;

		if (m_nextScene == SCENE_TITLE)
		{
			//タイトル画面に戻る
			SoundConfig::Instance()->Play(SoundConfig::BGM_TITLE);
			m_title.Init();
		}
		else if (m_nextScene == SCENE_IN_GAME)
		{
			//インゲーム
			GameInit();
			m_goal.Init(StageManager::Instance()->GetGoalTransform(), StageManager::Instance()->GetGoalModel());
		}

		//ゲームクリア時に遷移する処理
		if (m_clearFlag)
		{
			this->Initialize();
			m_clearFlag = false;
		}

		m_nowScene = m_nextScene;
	}

	//ゲームシーンでのみ使う物
	if (m_nowScene == SCENE_IN_GAME)
	{
		if (!m_fastTravel.IsActive())
		{
			m_player.Update(StageManager::Instance()->GetNowStage());
			m_goal.Update(&m_player.GetTransform());
			//ステージ選択画面ではギミックを作動させない
			StageManager::Instance()->Update(m_player);
		}
	}
	else
	{
		m_player.GetPointLig()->SetActive(false);
		m_player.DisactiveLight();
	}

	m_gateSceneChange.Update();
	m_fireFlyStage.ComputeUpdate(m_player.GetTransform().GetPos());

	BasicDraw::Instance()->Update(m_player.GetTransform().GetPosWorld(), *m_nowCam);

	//敵用丸影を更新
	EnemyDataReferenceForCircleShadow::Instance()->UpdateGPUData();

	GateManager::Instance()->FrameEnd();

	//チェックポイントの円柱を更新。
	m_checkPointPillar.Update(m_player.GetTransform().GetPosWorld());
	
	//ファストトラベル画面更新
	m_fastTravel.Update();
}

void GameScene::OnDraw()
{
	using namespace KuroEngine;
	static auto targetSize = D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize();
	static auto ds = D3D12App::Instance()->GenerateDepthStencil(targetSize);

	//レンダーターゲットのクリアとセット
	BasicDraw::Instance()->RenderTargetsClearAndSet(ds);

	//スカイドームの描画
	DrawFunc3D::DrawNonShadingModel(m_skyDomeModel, m_skyDomeTransform, *m_nowCam, 1.0f, nullptr, AlphaBlendMode_None);
	//BasicDraw::Instance()->Draw_Stage(*m_nowCam, m_ligMgr, m_skyDomeModel, m_skyDomeTransform, m_skyDomeDrawParam);

	//スカイドームを最背面描画するため、デプスステンシルのクリア
	KuroEngineDevice::Instance()->Graphics().ClearDepthStencil(ds);

	if (!m_fastTravel.IsActive() && m_nowScene == SCENE_IN_GAME)
	{
		m_goal.Draw(*m_nowCam);
		m_player.Draw(*m_nowCam, ds, m_ligMgr, DebugController::Instance()->IsActive());
		m_grass.Draw(*m_nowCam, m_ligMgr, m_player.GetGrowPlantLight().m_influenceRange, m_player.GetIsAttack());
	}

	//ステージ描画
	StageManager::Instance()->Draw(*m_nowCam, m_ligMgr);

	//プレイヤーのパーティクル描画
	if (!m_fastTravel.IsActive() && m_nowScene == SCENE_IN_GAME)
	{
		m_player.DrawParticle(*m_nowCam, m_ligMgr);
	}

	m_particleRender.Draw(*m_nowCam);

	//チェックポイントの円柱を描画
	m_checkPointPillar.Draw(*m_nowCam, m_ligMgr, ds);

	//m_canvasPostEffect.Execute();
	BasicDraw::Instance()->DrawEdge(m_nowCam->GetViewMat(), m_nowCam->GetProjectionMat(), ds, m_player.GetIsOverheat());

	m_lightBloomDevice.Draw(BasicDraw::Instance()->GetRenderTarget(BasicDraw::EMISSIVE), BasicDraw::Instance()->GetRenderTarget(BasicDraw::MAIN));

	m_fogPostEffect->Register(
		BasicDraw::Instance()->GetRenderTarget(BasicDraw::MAIN),
		BasicDraw::Instance()->GetRenderTarget(BasicDraw::DEPTH),
		BasicDraw::Instance()->GetRenderTarget(BasicDraw::BRIGHT),
		m_nowScene == SCENE_IN_GAME);

	m_vignettePostEffect.Register(m_fogPostEffect->GetResultTex());

	//UI描画
	if (!m_fastTravel.IsActive() && m_nowScene == SCENE_IN_GAME)
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
		m_goal.Draw2D();
	}
	else if (!m_fastTravel.IsActive() && m_nowScene == SCENE_TITLE)
	{
		m_title.Draw(*m_nowCam, m_ligMgr);
	}

	//ファストトラベル画面描画
	m_fastTravel.Draw(*m_nowCam);

	//シーン遷移描画
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
	//ゲーム終了時はゴールを使わない
	m_goal.Finalize();
}

