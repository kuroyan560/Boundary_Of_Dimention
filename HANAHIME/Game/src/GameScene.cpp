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

	//�N�����Ƀ^�C�g����ʂ��o���ׂɕK�v
	m_eTitleMode = TITLE_SELECT;

	//�����̓ǂݍ���
	SoundConfig::Instance();
	CameraData::Instance()->RegistCameraData("");
}


void GameScene::GameInit()
{
	SoundConfig::Instance()->Play(SoundConfig::BGM_IN_GAME);
	GrowPlantLight::ResetRegisteredLight();
	StageManager::Instance()->SetStage(m_stageNum);
	m_player.Init(StageManager::Instance()->GetPlayerSpawnTransform());
	SoundConfig::Instance()->Init();
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
	//�Q�[����ʂ���p�Y�����[�h�ɖ߂�ꍇ�Ƀp�Y�����[�h�Ƃ��ď�����������A�ĂёI���ł���悤SELECT������
	m_eTitleMode = TITLE_SELECT;
}

void GameScene::OnUpdate()
{
	m_particleRender.InitCount();

	//�f�o�b�O�p
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_I) || KuroEngine::UsersInput::Instance()->ControllerOnTrigger(0, KuroEngine::START) || m_player.GetIsFinishDeathAnimation())
	{
		m_eTitleMode = TITLE_PAZZLE;
		SoundConfig::Instance()->Play(SoundConfig::BGM_TITLE);
		this->Finalize();
		this->Initialize();
	}

	//�f�o�b�O���[�h�X�V
	DebugController::Instance()->Update();

	m_nowCam = m_player.GetCamera().lock();
	//�^�C�g����ʃ��[�h
	if (!m_title.IsFinish())
	{
		m_nowCam = m_title.GetCamera().lock();
	}
	//�z�[���ł̉��o
	if (m_movieCamera.IsStart())
	{
		m_nowCam = m_movieCamera.GetCamera().lock();
	}
	//�S�[�����̉��o
	if (StageManager::Instance()->IsClearNowStage() && m_1flameStopTimer.IsTimeUp() && (m_title.IsFinish() || m_title.IsStartOP()))
	{
		m_goal.Start();
		if (m_goal.ChangeCamera())
		{
			m_nowCam = m_goal.GetCamera().lock();
		}
		OperationConfig::Instance()->SetActive(false);
		m_clearFlag = true;
	}

	if (DebugController::Instance()->IsActive())
	{
		m_debugCam.Move();
		m_nowCam = m_debugCam;
	}




	m_grass.Update(1.0f, m_player.GetTransform(), m_player.GetCamera(), m_player.GetGrassPosScatter(), m_waterPaintBlend);

	//�z�[���ł̏���----------------------------------------
	if (!m_title.IsFinish() && !m_title.IsStartOP())
	{
		m_title.Update(&m_player.GetCamera().lock()->GetTransform(), m_nowCam);
	}

	//�X�e�[�W�I��
	int stageNum = -1;
	if (m_title.IsFinish())
	{
		stageNum = m_stageSelect.GetStageNumber(m_player.GetTransform().GetPos());
	}
	else
	{
		stageNum = m_title.GetStageNum();
	}

	//�Q�[���N���A���o���I������J�ڊJ�n
	if (m_goal.IsEnd())
	{
		stageNum = 1;
	}

	//�X�e�[�W�ړ����̏�����
	if (stageNum != -1)
	{
		m_stageNum = stageNum;
		m_gateSceneChange.Start();
	}

	if (m_gateSceneChange.IsHide())
	{
		GameInit();
		//�p�Y����ʂ���V�[���`�F���W������J�������[�h��؂�ւ���
		if (!m_title.IsFinish())
		{
			m_title.FinishTitle();
			OperationConfig::Instance()->SetActive(true);
		}
		else
		{
			//�^�C�g����ʂɖ߂�
			SoundConfig::Instance()->Play(SoundConfig::BGM_TITLE);
			StageManager::Instance()->SetStage();
		}

		m_goal.Init(StageManager::Instance()->GetGoalTransform(), StageManager::Instance()->GetGoalModel());

		//�Q�[���N���A���ɑJ�ڂ��鏈��
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

	//�Q�[���V�[���ł̂ݎg����
	if (m_title.IsFinish() || m_title.IsStartOP())
	{
		m_player.Update(StageManager::Instance()->GetNowStage());
		m_goal.Update(&m_player.GetTransform());
		//�X�e�[�W�I����ʂł̓M�~�b�N���쓮�����Ȃ�
		StageManager::Instance()->Update(m_player);
	}
	else
	{
		m_player.GetPointLig()->SetActive(false);
		m_player.DisactiveLight();
	}

	m_gateSceneChange.Update();
	m_movieCamera.Update();
	m_fireFlyStage.ComputeUpdate();


	m_stageSelect.Update();


	BasicDraw::Instance()->Update(m_player.GetTransform().GetPosWorld(), *m_nowCam);

}

void GameScene::OnDraw()
{
	using namespace KuroEngine;
	static auto targetSize = D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize();
	static auto ds = D3D12App::Instance()->GenerateDepthStencil(targetSize);

	//�����_�[�^�[�Q�b�g�̃N���A�ƃZ�b�g
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

	if (m_title.IsFinish() || m_title.IsStartOP())
	{
		m_player.Draw(*m_nowCam, m_ligMgr, DebugController::Instance()->IsActive());
		m_grass.Draw(*m_nowCam, m_ligMgr);
	}

	//�X�e�[�W�`��
	StageManager::Instance()->Draw(*m_nowCam, m_ligMgr);

	m_stageSelect.Draw(*m_nowCam, m_ligMgr);

	//m_movieCamera.DebugDraw(*m_nowCam, m_ligMgr);


	//m_canvasPostEffect.Execute();
	BasicDraw::Instance()->DrawEdge();

	//KuroEngineDevice::Instance()->Graphics().ClearDepthStencil(ds);
	//m_waterPaintBlend.Register(main, *nowCamera, ds);
	//m_vignettePostEffect.Register(m_waterPaintBlend.GetResultTex());

	//if (m_title.IsStartOP())
	{
		m_particleRender.Draw(*m_nowCam);
	}

	//tutorial.Draw(*m_nowCam);

	m_fogPostEffect->Register(
		BasicDraw::Instance()->GetRenderTarget(BasicDraw::MAIN),
		BasicDraw::Instance()->GetRenderTarget(BasicDraw::DEPTH),
		BasicDraw::Instance()->GetRenderTarget(BasicDraw::BRIGHT)
	);

	m_vignettePostEffect.Register(m_fogPostEffect->GetResultTex());

	m_player.DrawUI();


	m_title.Draw(*m_nowCam, m_ligMgr);

	if (m_title.IsFinish() || m_title.IsStartOP())
	{
		m_goal.Draw(*m_nowCam);
	}


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

