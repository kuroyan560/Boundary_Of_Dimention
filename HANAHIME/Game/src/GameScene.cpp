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
	//デバッグ用
	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_I))
	{
		this->Finalize();
		this->Initialize();
	}

	//デバッグモード更新
	DebugController::Instance()->Update();

	//デバッグモード有効
	if (DebugController::Instance()->IsActive())
	{
		//デバッグカメラの更新
		m_debugCam.Move();

		//以下ゲーム内オブジェクトの処理スキップ
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

	//デバッグモードが有効ならデバッグカメラ
	if (DebugController::Instance()->IsActive())nowCamera = m_debugCam;

	//ステージ描画
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

