#pragma once
#include"KuroEngineDevice.h"
#include"ForUser/DebugCamera.h"
#include"Player/Player.h"
#include"Render/RenderObject/LightManager.h"
#include"Graphics/CanvasPostEffect.h"
#include"ForUser/PostEffect/Vignette.h"
#include"Stage/Grass.h"
#include"Graphics/WaterPaintBlend.h"
#include"ForUser/PostEffect/Fog.h"
#include"Movie/SceneChange.h"
#include"Movie/MovieCamera.h"
#include"OutGame/Title.h"
#include"GPUParticle/GPUParticleRender.h"
#include"GPUParticle/FireFlyOutStage.h"
#include"Stage/Goal.h"
#include"Tutorial.h"
#include"Movie/CameraData.h"
#include"Stage/Enemy/Enemy.h"
#include"Render/LightBloomDevice.h"
#include"HUD/OperationInfoUI.h"
#include"HUD/StageInfoUI.h"
#include"HUD/PauseUI.h"
#include"Effect/GuideInsect.h"
#include"Stage/CheckPointPillar.h"
#include"System/FastTravel.h"
#include"System/SystemSetting.h"

class GameScene : public KuroEngine::BaseScene
{
	enum SCENE_STATUS { SCENE_TITLE, SCENE_IN_GAME }m_nowScene = SCENE_TITLE;
	SCENE_STATUS m_nextScene = m_nowScene;
	KuroEngine::Transform m_playerInitTransform;

	//�X�J�C�h�[��
	KuroEngine::Transform m_skyDomeTransform;
	IndividualDrawParameter m_skyDomeDrawParam;
	std::shared_ptr<KuroEngine::Model>m_skyDomeModel;

	Player m_player;

	KuroEngine::DebugCamera m_debugCam;

	//�Ɩ����
	KuroEngine::LightManager m_ligMgr;
	//�f�B���N�V�������C�g
	std::vector<KuroEngine::Light::Direction>m_dirLigArray;
	//�X�J�C�h�[�����C�g
	KuroEngine::Light::HemiSphere m_hemiLig;

	//�t�H�O�|�X�g�G�t�F�N�g
	std::shared_ptr<KuroEngine::Fog>m_fogPostEffect;

	//���B�l�b�g�|�X�g�G�t�F�N�g
	KuroEngine::Vignette m_vignettePostEffect;

	//�L�����o�X�ɕ`�������ɂ���|�X�g�G�t�F�N�g�iGris�I�ȁj
	CanvasPostEffect m_canvasPostEffect;

	//���ނ�
	Grass m_grass;

	//���ʉ敗�u�����h�|�X�g�G�t�F�N�g
	WaterPaintBlend m_waterPaintBlend;

	//�`�F�b�N�|�C���g�ɏo��~��
	CheckPointPillar m_checkPointPillar;

	int m_stageNum;
	SceneChange m_gateSceneChange;

	//MovieCamera m_movieCamera;

	std::shared_ptr<KuroEngine::Camera>m_nowCam;

	Title m_title;
	bool m_clearFlag;

	//�t�@�X�g�g���x��
	FastTravel m_fastTravel;
	//�ݒ���
	SystemSetting m_sysSetting;

	//�X�e�[�W�O�̌u�`��
	FireFlyOutStage m_fireFlyStage;

	SignSpotFireFly m_guideFly;

	Tutorial tutorial;

	Goal m_goal;

	//HUD
	OperationInfoUI m_opeInfoUI;
	StageInfoUI m_stageInfoUI;
	PauseUI m_pauseUI;

	KuroEngine::LightBloomDevice m_lightBloomDevice;

	GuideInsect m_guideInsect;

	bool m_deadFlag;

	bool IsSystemAplicationActive()const
	{
		return m_fastTravel.IsActive() || m_sysSetting.IsActive();
	}

	void GameInit();

	void OnInitialize()override;
	void OnUpdate()override;
	void OnDraw()override;
	void OnImguiDebug()override;
	void OnFinalize()override;
public:
	GameScene();
	void Retry();
	void StartGame(int arg_stageNum, KuroEngine::Transform arg_playerInitTransform);
	void GoBackTitle();
	void ActivateFastTravel();
	void ActivateSystemSetting();
};