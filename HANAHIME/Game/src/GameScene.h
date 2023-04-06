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
#include"Stage/StageSelect.h"
#include"SceneChange.h"
#include"MovieCamera.h"

class GameScene : public KuroEngine::BaseScene
{
	std::shared_ptr<KuroEngine::TextureBuffer>m_ddsTex;
	std::shared_ptr<KuroEngine::TextureBuffer>m_pngTex;
	Player m_player;

	KuroEngine::DebugCamera m_debugCam;

	//照明情報
	KuroEngine::LightManager m_ligMgr;
	//ディレクションライト
	KuroEngine::Light::Direction m_dirLig;

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
	StageSelect m_stageSelect;

	int m_stageNum;
	SceneChange m_gateSceneChange;

	MovieCamera m_movieCamera;

	std::shared_ptr<KuroEngine::Camera>m_nowCam;

	void OnInitialize()override;
	void OnUpdate()override;
	void OnDraw()override;
	void OnImguiDebug()override;
	void OnFinalize()override;
public:
	GameScene();
};