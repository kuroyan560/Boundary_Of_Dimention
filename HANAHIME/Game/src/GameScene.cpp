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
#include"System/SaveDataManager.h"

GameScene::GameScene() :m_fireFlyStage(GPUParticleRender::Instance()->GetStackBuffer()), tutorial(GPUParticleRender::Instance()->GetStackBuffer()), m_goal(GPUParticleRender::Instance()->GetStackBuffer()),
m_guideFly(GPUParticleRender::Instance()->GetStackBuffer()), m_guideInsect(GPUParticleRender::Instance()->GetStackBuffer())
{
	KuroEngine::Vec3<float>dir = { 0.0f,-1.0f,0.0f };
	m_dirLigArray.emplace_back();
	m_dirLigArray.back().SetDir(dir.GetNormal());
	m_dirLigArray.back().SetColor(KuroEngine::Color(0.7f, 0.7f, 0.7f, 1.0f));

	dir = { -1.0f,0.5f,0.0f };
	m_dirLigArray.emplace_back();
	m_dirLigArray.back().SetDir(dir.GetNormal());
	m_dirLigArray.back().SetColor(KuroEngine::Color(0.7f, 0.6f, 0.65f, 1.0f));

	dir = { 1.0f,-0.5f,0.0f };
	m_dirLigArray.emplace_back();
	m_dirLigArray.back().SetDir(dir.GetNormal());
	m_dirLigArray.back().SetColor(KuroEngine::Color(0.6f, 0.7f, 0.7f, 1.0f));

	dir = { 0.0f,0.0f,-1.0f };
	m_dirLigArray.emplace_back();
	m_dirLigArray.back().SetDir(dir.GetNormal());
	m_dirLigArray.back().SetColor(KuroEngine::Color(0.3f, 0.4f, 0.4f, 1.0f));

	for (auto& dirLig : m_dirLigArray)
	{
		m_ligMgr.RegisterDirLight(&dirLig);
	}
	m_ligMgr.RegisterPointLight(m_player.GetPointLig());

	m_hemiLig.SetSkyColor(KuroEngine::Color(115, 221, 249, 255));
	m_hemiLig.SetGroundColor(KuroEngine::Color(81, 151, 118, 255));
	m_ligMgr.RegisterHemiSphereLight(&m_hemiLig);

	auto backBuffTarget = KuroEngine::D3D12App::Instance()->GetBackBuffRenderTarget();
	m_fogPostEffect = std::make_shared<KuroEngine::Fog>(backBuffTarget->GetGraphSize(), backBuffTarget->GetDesc().Format);

	//�����̓ǂݍ���
	SoundConfig::Instance();
	CameraData::Instance()->RegistCameraData("");

	//�X�J�C�h�[��
	m_skyDomeModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Skydome.glb");
	m_skyDomeDrawParam.m_edgeColor.m_a = 0.0f;
	m_skyDomeTransform.SetScale(StageManager::Instance()->GetSkyDomeScaling());

	m_fireFlyStage.ComputeInit({ 0.0f,0.0f,0.0f });

	m_latestVisitGateTransform = StageManager::Instance()->GetStartPointTransform();
}


void GameScene::GameInit()
{
	SoundConfig::Instance()->Play(SoundConfig::BGM(SoundConfig::BGM_IN_GAME_3));
	GrowPlantLight::ResetRegisteredLight();
	StageManager::Instance()->SetStage(m_stageNum);

	if (m_isFastTravel) {

		m_player.Respawn(m_playerInitTransform, StageManager::Instance()->GetNowStage().lock()->GetStartPointTransform().GetPos(), m_fastTravel.GetNowStageIndex(), m_fastTravel.GetNowCheckPointIndex());
		m_isFastTravel = false;

	}
	else if (m_isRetry) {

		std::vector<std::vector<KuroEngine::Transform>> checkPointTransform;
		int stageNum = 0;
		int checkPointNum = 0;

		//���݂̃`�F�b�N�|�C���g�������B
		auto nowStage = StageManager::Instance()->GetUnlockedCheckPointInfo(&checkPointTransform, &stageNum, &checkPointNum);

		m_player.Respawn(m_playerInitTransform, StageManager::Instance()->GetNowStage().lock()->GetStartPointTransform().GetPos(), m_stageNum, checkPointNum);
		m_isRetry = false;

	}
	else {
		m_player.Respawn(m_playerInitTransform, StageManager::Instance()->GetNowStage().lock()->GetStartPointTransform().GetPos(), m_stageNum, 0);
	}

	SoundConfig::Instance()->Init();
	GateManager::Instance()->Init();
	m_opeInfoUI.Init();
	InGameUI::Init();
	m_stageInfoUI.Init(m_stageNum, StageManager::Instance()->GetStarCoinNum());
	m_pauseUI.Init();
	m_checkPointPillar.Init();
	OperationConfig::Instance()->SetInGameOperationActive(true);
	TimeScaleMgr::s_inGame.Set(1.0f);

	//std::vector<KuroEngine::Transform> testTransform;
	//KuroEngine::Transform buff;
	//buff.SetPos(KuroEngine::Vec3<float>(0, 0, 0));
	//testTransform.emplace_back(buff);
	//buff.SetPos(KuroEngine::Vec3<float>(0, -100, 100));
	//testTransform.emplace_back(buff);
	//FastTravel::Instance()->Init(testTransform);
	m_fireFlyStage.ComputeInit(m_fastTravel.GetNowSelectCheckPointTransform().GetPos());
}

void GameScene::Retry()
{
	StartGame(m_stageNum, CheckPoint::GetLatestVistTransform(m_latestVisitGateTransform), false, true);
}

void GameScene::StartGame(int arg_stageNum, KuroEngine::Transform arg_playerInitTransform, bool arg_isFastTravel, bool arg_isRetry)
{
	m_stageNum = arg_stageNum;
	m_gateSceneChange.Start();
	m_nextScene = SCENE_STAGESELECT;
	m_playerInitTransform = arg_playerInitTransform;
	m_isFastTravel = arg_isFastTravel;
	m_isRetry = arg_isRetry;

	m_latestVisitGateTransform = arg_playerInitTransform;
}

void GameScene::GoBackTitle()
{
	m_gateSceneChange.Start();
	m_nextScene = SCENE_TITLE;
}

void GameScene::ActivateFastTravel()
{
	std::vector<std::vector<KuroEngine::Transform>>checkPointTransformArray;
	int recentStageNum;
	int recentIdx;

	if (StageManager::Instance()->GetUnlockedCheckPointInfo(&checkPointTransformArray, &recentStageNum, &recentIdx))
	{
		m_fastTravel.Activate(checkPointTransformArray, recentStageNum, recentIdx);
	}
}

void GameScene::ActivateSystemSetting()
{
	m_sysSetting.Activate();
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
	DebugEnemy::Instance()
		});

	m_debugCam.Init({ 0,5,-10 });

	m_player.Init(StageManager::Instance()->GetStartPointTransform(), m_fastTravel.GetNowStageIndex(), m_fastTravel.GetNowCheckPointIndex());

	m_grass.Init();

	m_waterPaintBlend.Init();

	m_title.Init();

	m_pauseUI.Init();
	m_deadFlag = false;


	m_goal.Init(KuroEngine::Transform(), {});
}

void GameScene::OnUpdate()
{
	GPUParticleRender::Instance()->InitCount();

	//�f�o�b�O���[�h�X�V
	DebugController::Instance()->Update();

	//�t�@�X�g�g���x����ʍX�V
	m_fastTravel.Update(this);

	if (m_fastTravel.GetNowSelectStage() != m_prevIndex)
	{
		m_fireFlyStage.ComputeInit(m_fastTravel.GetNowSelectCheckPointTransform().GetPos());
	}
	m_prevIndex = m_fastTravel.GetNowSelectStage();

	//�t�@�X�g�g���x��
	if (m_fastTravel.IsActive())
	{
		m_nowCam = m_fastTravel.GetCamera();
	}
	else if (IsSystemAplicationActive()) {}
	//�^�C�g�����
	else if (m_nowScene == SCENE_TITLE)
	{
		m_nowCam = m_title.GetCamera().lock();
		m_title.Update(&m_player.GetCamera().lock()->GetTransform(), m_nowCam, this);
	}
	//�C���Q�[��
	else if (m_nowScene == SCENE_IN_GAME)
	{
		//�ʏ�̃v���C���[�J����
		m_nowCam = m_player.GetCamera().lock();

		bool flag = StageManager::Instance()->IsClearNowStage();
		//�N���A���̃J����
		if (flag)
		{
			m_goal.Start();
			if (m_goal.ChangeCamera())
			{
				m_nowCam = m_goal.GetCamera().lock();
			}
			OperationConfig::Instance()->SetAllInputActive(false);
			m_clearFlag = true;
		}

		//�Q�[�g����������
		if (GateManager::Instance()->IsEnter() && !m_gateSceneChange.IsActive())
		{
			StartGame(GateManager::Instance()->GetDestStageNum(),
				StageManager::Instance()->GetGateTransform(GateManager::Instance()->GetDestStageNum(), GateManager::Instance()->GetDestGateID()));
			SoundConfig::Instance()->Play(SoundConfig::SE_GATE);
		}
		//�Q�[���N���A���o���I������J�ڊJ�n
		if (m_goal.IsEnd())
		{
			m_gateSceneChange.Start();
			GoBackTitle();
		}

		//�|�[�Y���
		if (OperationConfig::Instance()->GetOperationInput(OperationConfig::PAUSE, OperationConfig::ON_TRIGGER))
		{
			m_pauseUI.SetInverseActive();
		}

		//UI�X�V
		InGameUI::Update(TimeScaleMgr::s_inGame.GetTimeScale());
		m_opeInfoUI.Update(TimeScaleMgr::s_inGame.GetTimeScale());
		m_stageInfoUI.Update(TimeScaleMgr::s_inGame.GetTimeScale(), StageManager::Instance()->GetStarCoinNum());
		m_pauseUI.Update(this);
	}
	else if (m_nowScene == SCENE_STAGESELECT)
	{
		if (m_gateSceneChange.IsAppear() && m_stageSelect.Done())
		{
			m_stageNum = m_stageSelect.GetNumber();
			m_nextScene = SCENE_IN_GAME;
			m_gateSceneChange.Start();
		}

		m_stageSelect.Update(m_nowCam);
	}


	//�ݒ��ʍX�V
	m_sysSetting.Update();

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
		DebugEnemy::Instance()->SetStageNum(m_stageNum);

		m_deadFlag = false;

		if (m_nextScene == SCENE_TITLE)
		{
			//�^�C�g����ʂɖ߂�
			SoundConfig::Instance()->Play(SoundConfig::BGM_TITLE);
			m_title.Init();
		}
		else if (m_nextScene == SCENE_IN_GAME)
		{
			//�C���Q�[��
			GameInit();
			m_goal.Init(StageManager::Instance()->GetGoalTransform(), StageManager::Instance()->GetGoalModel());
		}
		else if (m_nextScene == SCENE_STAGESELECT)
		{
			m_stageSelect.Init();
		}

		//�Q�[���N���A���ɑJ�ڂ��鏈��
		if (m_clearFlag)
		{
			this->Initialize();
			m_clearFlag = false;
		}

		//�t�@�X�g�g���x����ʂ����
		m_fastTravel.DisActivate();

		m_nowScene = m_nextScene;
	}

	//�Q�[���V�[���ł̂ݎg����
	if (m_nowScene == SCENE_IN_GAME)
	{
		if (!IsSystemAplicationActive())
		{
			m_player.Update(StageManager::Instance()->GetNowStage());
			m_goal.Update(&m_player.GetTransform(), m_player);
			//�X�e�[�W�I����ʂł̓M�~�b�N���쓮�����Ȃ�
			StageManager::Instance()->Update(m_player);
		}
	}
	else
	{
		m_player.GetPointLig()->SetActive(false);
		m_player.DisactiveLight();
	}

	//�v���C���[������ł����烊�g���C�B
	if (m_player.GetIsFinishDeadEffect()) {
		Retry();
	}


	////���̃X�e�[�W�ɂ��邷�ׂĂ̓G�Ƀ��[�v����
	//if (OperationConfig::Instance()->DebugKeyInputOnTrigger(DIK_0))
	//{
	//	m_player.Init(DebugEnemy::Instance()->GetTransform());
	//}
	////���̃X�e�[�W�ɂ���~�j�o�O�Ƀ��[�v����
	//if (OperationConfig::Instance()->DebugKeyInputOnTrigger(DIK_1))
	//{
	//	m_player.Init(DebugEnemy::Instance()->GetSpecificTransform(ENEMY_MINIBUG));
	//}
	////���̃X�e�[�W�ɂ���~�j�o�O�Ƀ��[�v����
	//if (OperationConfig::Instance()->DebugKeyInputOnTrigger(DIK_2))
	//{
	//	m_player.Init(DebugEnemy::Instance()->GetSpecificTransform(ENEMY_DOSSUN_ALLWAYS));
	//}
	//if (OperationConfig::Instance()->DebugKeyInputOnTrigger(DIK_3))
	//{
	//	m_player.Init(DebugEnemy::Instance()->GetSpecificTransform(ENEMY_DOSSUN_NORMAL));
	//}
	//if (OperationConfig::Instance()->DebugKeyInputOnTrigger(DIK_LSHIFT))
	//{
	//	KuroEngine::Transform &transform = m_nowCam->GetTransform();
	//	transform.SetPos(m_player.GetTransform().GetPos());
	//}
	//if (OperationConfig::Instance()->DebugKeyInputOnTrigger(DIK_LSHIFT))
	//{
	//	m_player.Init(StageManager::Instance()->GetGoalTransform());
	//}

	m_gateSceneChange.Update();
	m_fireFlyStage.ComputeUpdate(m_player.GetTransform().GetPos());

	BasicDraw::Instance()->Update(m_player.GetTransform().GetPosWorld(), *m_nowCam);

	//�G�p�ۉe���X�V
	EnemyDataReferenceForCircleShadow::Instance()->UpdateGPUData();

	GateManager::Instance()->FrameEnd();

	//�`�F�b�N�|�C���g�̉~�����X�V�B
	m_checkPointPillar.Update(m_player.GetTransform());


}

void GameScene::OnDraw()
{
	using namespace KuroEngine;
	static auto targetSize = D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize();
	static auto ds = D3D12App::Instance()->GenerateDepthStencil(targetSize);

	//�����_�[�^�[�Q�b�g�̃N���A�ƃZ�b�g
	BasicDraw::Instance()->RenderTargetsClearAndSet(ds);

	//�X�J�C�h�[���̕`��
	DrawFunc3D::DrawNonShadingModel(m_skyDomeModel, m_skyDomeTransform, *m_nowCam, 1.0f, nullptr, AlphaBlendMode_None);
	//BasicDraw::Instance()->Draw_Stage(*m_nowCam, m_ligMgr, m_skyDomeModel, m_skyDomeTransform, m_skyDomeDrawParam);


	//�X�J�C�h�[�����Ŕw�ʕ`�悷�邽�߁A�f�v�X�X�e���V���̃N���A
	KuroEngineDevice::Instance()->Graphics().ClearDepthStencil(ds);

	if (!IsSystemAplicationActive() && m_nowScene == SCENE_IN_GAME)
	{
		m_goal.Draw(*m_nowCam);
		m_player.Draw(*m_nowCam, ds, m_ligMgr, DebugController::Instance()->IsActive());
		m_grass.Draw(*m_nowCam, m_ligMgr, m_player.GetGrowPlantLight().m_influenceRange, m_player.GetIsAttack());
	}
	
	//�X�e�[�W�`��
	StageManager::Instance()->Draw(*m_nowCam, m_ligMgr);

	//�v���C���[�̃p�[�e�B�N���`��
	if (!IsSystemAplicationActive() && m_nowScene == SCENE_IN_GAME)
	{
		m_player.DrawParticle(*m_nowCam, m_ligMgr);
	}

	GPUParticleRender::Instance()->Draw(*m_nowCam);

	//�`�F�b�N�|�C���g�̉~����`��
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

	//UI�`��
	if (!IsSystemAplicationActive() && m_nowScene == SCENE_IN_GAME)
	{
		if (!m_clearFlag)
		{
			//�|�[�Y���
			m_pauseUI.Draw(StarCoin::GetFlowerSum());

			//�|�[�Y��ʂłȂ����
			if (!m_pauseUI.IsActive())
			{
				m_player.DrawUI(*m_nowCam);
				StageManager::Instance()->DrawUI(*m_nowCam, m_player.GetTransform().GetPosWorld());
				m_opeInfoUI.Draw();
				m_stageInfoUI.Draw(StageManager::Instance()->ExistStarCoinNum(), StageManager::Instance()->GetStarCoinNum());
			}
		}
		m_goal.Draw2D();
	}
	else if (!IsSystemAplicationActive() && m_nowScene == SCENE_TITLE)
	{
		m_title.Draw(*m_nowCam, m_ligMgr);
	}
	else if (m_nowScene == SCENE_STAGESELECT)
	{
		m_stageSelect.Draw(*m_nowCam);
	}

	//�t�@�X�g�g���x����ʕ`��
	m_fastTravel.Draw(*m_nowCam);

	//�ݒ��ʕ`��
	m_sysSetting.Draw();

	//�V�[���J�ڕ`��
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
	//�Q�[���I�����̓S�[�����g��Ȃ�
	m_goal.Finalize();
}

