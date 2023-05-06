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
#include"Stage/HomeStageSelect.h"
#include"Movie/StageChange.h"
#include"Movie/MovieCamera.h"
#include"OutGame/Title.h"
#include"GPUParticle/GPUParticleRender.h"
#include"GPUParticle/FireFlyOutStage.h"
#include"Stage/Goal.h"
#include"Tutorial.h"
#include"Movie/CameraData.h"
#include"Stage/Enemy/Enemy.h"


class GameScene : public KuroEngine::BaseScene
{
	std::shared_ptr<KuroEngine::TextureBuffer>m_ddsTex;
	std::shared_ptr<KuroEngine::TextureBuffer>m_pngTex;

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

	//�X�e�[�W�I�����
	HomeStageSelect m_stageSelect;

	int m_stageNum;
	SceneChange m_gateSceneChange;

	MovieCamera m_movieCamera;

	std::shared_ptr<KuroEngine::Camera>m_nowCam;

	TitleMode m_eTitleMode;
	Title m_title;
	bool m_clearFlag;
	KuroEngine::Timer m_1flameStopTimer;

	//GPU�p�[�e�B�N���̕`��
	GPUParticleRender m_particleRender;
	//�X�e�[�W�O�̌u�`��
	FireFlyOutStage m_fireFlyStage;

	Tutorial tutorial;

	Goal m_goal;

	//�G���G�\��
	std::unique_ptr<MiniBug> miniBug;
	std::unique_ptr<DossunRing> dossun;
	std::shared_ptr<KuroEngine::Model> m_enemyModel;


	void GameInit();

	void OnInitialize()override;
	void OnUpdate()override;
	void OnDraw()override;
	void OnImguiDebug()override;
	void OnFinalize()override;
public:
	GameScene();
};