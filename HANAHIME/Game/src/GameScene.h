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

	//照明情報
	KuroEngine::LightManager m_ligMgr;
	//ディレクションライト
	std::vector<KuroEngine::Light::Direction>m_dirLigArray;
	//スカイドームライト
	KuroEngine::Light::HemiSphere m_hemiLig;

	//フォグポストエフェクト
	std::shared_ptr<KuroEngine::Fog>m_fogPostEffect;

	//ヴィネットポストエフェクト
	KuroEngine::Vignette m_vignettePostEffect;

	//キャンバスに描いた風にするポストエフェクト（Gris的な）
	CanvasPostEffect m_canvasPostEffect;

	//草むら
	Grass m_grass;

	//水彩画風ブレンドポストエフェクト
	WaterPaintBlend m_waterPaintBlend;

	//ステージ選択画面
	HomeStageSelect m_stageSelect;

	int m_stageNum;
	SceneChange m_gateSceneChange;

	MovieCamera m_movieCamera;

	std::shared_ptr<KuroEngine::Camera>m_nowCam;

	TitleMode m_eTitleMode;
	Title m_title;
	bool m_clearFlag;
	KuroEngine::Timer m_1flameStopTimer;

	//GPUパーティクルの描画
	GPUParticleRender m_particleRender;
	//ステージ外の蛍描画
	FireFlyOutStage m_fireFlyStage;

	Tutorial tutorial;

	Goal m_goal;

	//雑魚敵表示
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