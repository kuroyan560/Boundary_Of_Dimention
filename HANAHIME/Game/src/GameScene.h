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

	//�Ɩ����
	KuroEngine::LightManager m_ligMgr;
	//�f�B���N�V�������C�g
	KuroEngine::Light::Direction m_dirLig;

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