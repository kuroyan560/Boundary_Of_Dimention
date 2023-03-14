#include "GameScene.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"ForUser/Debugger.h"
#include"OperationConfig.h"
#include"DebugController.h"
#include"Stage/StageManager.h"

#include"ForUser/JsonData.h"

GameScene::GameScene()
{
	m_ddsTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/test.dds");
	m_pngTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/test.png");
}


void GameScene::OnInitialize()
{
	m_debugCam.Init({ 0,0,-10 }, { 0,0,0 });

	KuroEngine::Transform playerInitTransform;
	playerInitTransform.SetPos({ 0,0,-10 });
	playerInitTransform.SetFront({ 0,0,1 });
	m_player.Init(playerInitTransform);

	KuroEngine::Debugger::Register({ 
		OperationConfig::Instance(),
		&m_player,
		StageManager::Instance(),
		});

	KuroEngine::JsonData test;
	//test.m_jsonData["debugger"]["player"] = { {"move",3},{"jump",4} };
	//test.m_jsonData["debugger"]["camera"] = { { "sensitivity",2 } };
	//test.m_jsonData["debugger"]["stage"] = 5;

	test.m_jsonData["debugger"] = {
		{"player",{
			{"move",3},{"jump",4}}
		},
		{"camera",{
			{"sensitivity",2}}
		},
		{"stage",5}
	};
	test.m_jsonData["name"] = "DebuggerParams";


	std::string name;
	test.Get<std::string>(name, { "name" });

	float sens;
	test.Get<float>(sens, { "debugger","camera","sensitivity" });

	test.Export("resource/user/", "test", ".json");
}

void GameScene::OnUpdate()
{
	//�f�o�b�O�p
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_I))
	{
		this->Finalize();
		this->Initialize();
	}

	//�f�o�b�O���[�h�X�V
	DebugController::Instance()->Update();

	//�f�o�b�O���[�h�L��
	if (DebugController::Instance()->IsActive())
	{
		//�f�o�b�O�J�����̍X�V
		m_debugCam.Move();

		//�ȉ��Q�[�����I�u�W�F�N�g�̏����X�L�b�v
		return;
	}

	m_player.Update();
}

void GameScene::OnDraw()
{
	using namespace KuroEngine;
	static auto ds = D3D12App::Instance()->GenerateDepthStencil(D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize());

	KuroEngineDevice::Instance()->Graphics().ClearDepthStencil(ds);

	KuroEngineDevice::Instance()->Graphics().SetRenderTargets(
		{ D3D12App::Instance()->GetBackBuffRenderTarget() },
		ds
	);

	auto nowCamera = m_player.GetCamera().lock();

	//�f�o�b�O���[�h���L���Ȃ�f�o�b�O�J����
	if (DebugController::Instance()->IsActive())nowCamera = m_debugCam;

	//�X�e�[�W�`��
	StageManager::Instance()->Draw(*nowCamera);
	
	Transform transform;
	transform.SetPos({ -0.5f,0,0 });
	DrawFunc3D::DrawNonShadingPlane(
		m_ddsTex,
		transform,
		*nowCamera);

	transform.SetPos({ 0.5f,0,0 });
	DrawFunc3D::DrawNonShadingPlane(
		m_pngTex,
		transform,
		*nowCamera);

	m_player.Draw(*nowCamera);
}

void GameScene::OnImguiDebug()
{
	if (!DebugController::Instance()->IsActive())return;

	KuroEngine::Debugger::Draw();
}

void GameScene::OnFinalize()
{
	KuroEngine::Debugger::ClearRegister();
}

