#pragma once
#include"KuroEngineDevice.h"
#include"ForUser/DebugCamera.h"
#include"Player/Player.h"
#include"Render/RenderObject/LightManager.h"
#include"Graphics/CanvasPostEffect.h"
#include"ForUser/PostEffect/Vignette.h"
#include"Stage/Grass.h"

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

	//���B�l�b�g�|�X�g�G�t�F�N�g
	KuroEngine::Vignette m_vignettePostEffect;

	//�L�����o�X�ɕ`�������ɂ���|�X�g�G�t�F�N�g�iGris�I�ȁj
	CanvasPostEffect m_canvasPostEffect;

	Grass m_grass;

	void OnInitialize()override;
	void OnUpdate()override;
	void OnDraw()override;
	void OnImguiDebug()override;
	void OnFinalize()override;
public:
	GameScene();
};