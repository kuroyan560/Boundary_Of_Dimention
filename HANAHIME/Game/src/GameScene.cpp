#include "GameScene.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"ForUser/Debugger.h"
#include"OperationConfig.h"
#include"DebugController.h"
#include"Stage/StageManager.h"

#include"ForUser/JsonData.h"
#include"Graphics/BasicDraw.h"
#include"../../../src/engine/FrameWork/UsersInput.h"

GameScene::GameScene()
{
	m_ddsTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/test.dds");
	m_pngTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/test.png");

	KuroEngine::Vec3<float>dir = { 0.0f,-1.0f,0.0f };
	m_dirLig.SetDir(dir.GetNormal());
	m_ligMgr.RegisterDirLight(&m_dirLig);
	m_ligMgr.RegisterPointLight(m_player.GetPointLig());

}


void GameScene::OnInitialize()
{
	KuroEngine::Debugger::Register({
	OperationConfig::Instance(),
	&m_player,
	m_player.GetCameraControllerDebugger(),
	StageManager::Instance(),
	BasicDraw::Instance(),
	&m_vignettePostEffect,
	&m_waterPaintBlend,
	&m_ligMgr,
		});

	m_debugCam.Init({ 0,5,-10 });

	KuroEngine::Transform playerInitTransform;
	playerInitTransform.SetPos({ 3.7f,26.0f,-39.0f });
	m_player.Init(playerInitTransform);

	m_grass.Init();

	m_waterPaintBlend.Init();
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

	if (DebugController::Instance()->IsActive())m_debugCam.Move();

	m_player.Update(StageManager::Instance()->GetNowStage());

	m_grass.Update(1.0f, m_player.GetTransform().GetPos(), m_player.GetTransform().GetRotate(), m_player.GetCamera().lock()->GetTransform(), m_player.GetGrassPosScatter(), m_waterPaintBlend);
	//m_grass.Plant(m_player.GetTransform(), m_player.GetGrassPosScatter(), m_waterPaintBlend);


	//ホームでの処理----------------------------------------

	//ステージ選択
	int stageNum = m_stageSelect.GetStageNumber(m_player.GetTransform().GetPos());
	//ステージ移動時の初期化
	if (stageNum != -1)
	{
		m_stageNum = stageNum;
		m_gateSceneChange.Start();
	}

	if (m_gateSceneChange.IsHide())
	{
		StageManager::Instance()->SetStage(m_stageNum);
		KuroEngine::Transform playerInitTransform;
		playerInitTransform.SetPos({ 30.0f,50.0f,-45 });
		m_player.Init(playerInitTransform);
	}

	m_stageSelect.Update();
	//ホームでの処理----------------------------------------

	m_gateSceneChange.Update();


	if (KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_O))
	{
		std::vector<MovieCameraData>moveDataArray;

		//プレイヤーカメラの親子関係を考慮したワールド行列
		auto matA = m_player.GetCamera().lock()->GetTransform().GetMatWorld();
		//プレイヤーカメラの親子関係を考慮した回転行列
		auto matB = XMMatrixRotationQuaternion(m_player.GetCamera().lock()->GetTransform().GetRotateWorld());


		KuroEngine::Vec3<float> cameraPos(
			matA.r[3].m128_f32[0],
			matA.r[3].m128_f32[1],
			matA.r[3].m128_f32[2]
		);
	
		MovieCameraData data;
		{
			//上向きに見ている
			KuroEngine::Transform upVec;
			upVec.SetPos({ 3.7f,36.0f,-29.0f });
			data.stopTimer = 1;
			data.interpolationTimer = 2;
			moveDataArray.emplace_back(data);
		}

		{
			//下向きに見る
			KuroEngine::Transform downVec;
			downVec.SetPos({ 3.7f,36.0f,-29.0f });
			data.stopTimer = 2;
			data.interpolationTimer = 1;
			moveDataArray.emplace_back(data);
		}

		{
			//プレイヤーの位置に戻る
			data.stopTimer = 2;
			data.interpolationTimer = 3;
			moveDataArray.emplace_back(data);
		}

		m_movieCamera.StartMovie(
			m_player.GetCamera().lock()->GetTransform(),
			moveDataArray
		);
	}

	
	m_movieCamera.Update();




	BasicDraw::Instance()->Update(m_player.GetTransform().GetPosWorld());
}

void GameScene::OnDraw()
{
	using namespace KuroEngine;
	static auto targetSize = D3D12App::Instance()->GetBackBuffRenderTarget()->GetGraphSize();
	static auto ds = D3D12App::Instance()->GenerateDepthStencil(targetSize);

	static auto main = D3D12App::Instance()->GenerateRenderTarget(
		D3D12App::Instance()->GetBackBuffFormat(),
		Color(0.0f, 0.0f, 0.0f, 0.0f),
		targetSize,
		L"MainRenderTarget");
	static auto emissiveMap = D3D12App::Instance()->GenerateRenderTarget(
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		Color(0.0f, 0.0f, 0.0f, 1.0f),
		targetSize, L"EmissiveMap");
	static auto depthMap = D3D12App::Instance()->GenerateRenderTarget(
		DXGI_FORMAT_R16_FLOAT,
		Color(FLT_MAX, 0.0f, 0.0f, 0.0f),
		targetSize, L"DepthMap");
	static auto normalMap = D3D12App::Instance()->GenerateRenderTarget(
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		Color(0.0f, 0.0f, 0.0f, 0.0f),
		targetSize, L"NormalMap");
	static auto edgeColMap = D3D12App::Instance()->GenerateRenderTarget(
		D3D12App::Instance()->GetBackBuffFormat(),
		Color(0.0f, 0.0f, 0.0f, 1.0f),
		targetSize, L"EdgeColorMap");

	//レンダーターゲットのクリア
	KuroEngineDevice::Instance()->Graphics().ClearRenderTarget(main);
	KuroEngineDevice::Instance()->Graphics().ClearRenderTarget(emissiveMap);
	KuroEngineDevice::Instance()->Graphics().ClearRenderTarget(depthMap);
	KuroEngineDevice::Instance()->Graphics().ClearRenderTarget(normalMap);
	KuroEngineDevice::Instance()->Graphics().ClearRenderTarget(edgeColMap);
	KuroEngineDevice::Instance()->Graphics().ClearDepthStencil(ds);

	//レンダーターゲットをセット
	KuroEngineDevice::Instance()->Graphics().SetRenderTargets(
		{
			main,
			emissiveMap,
			depthMap,
			normalMap,
			edgeColMap
		},
		ds
	);


	auto nowCamera = m_player.GetCamera().lock();
	if (m_movieCamera.IsStart())
	{
		nowCamera = m_movieCamera.GetCamera().lock();
	}

	if (DebugController::Instance()->IsActive())nowCamera = m_debugCam;

	//ステージ描画
	StageManager::Instance()->Draw(*nowCamera, m_ligMgr);

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

	m_player.Draw(*nowCamera, m_ligMgr, DebugController::Instance()->IsActive());

	m_grass.Draw(*nowCamera, m_ligMgr);

	m_stageSelect.Draw(*nowCamera, m_ligMgr);

	m_movieCamera.DebugDraw(*nowCamera, m_ligMgr);

	//m_canvasPostEffect.Execute();
	BasicDraw::Instance()->DrawEdge(depthMap, normalMap, edgeColMap);


	m_gateSceneChange.Draw();

	//KuroEngineDevice::Instance()->Graphics().ClearDepthStencil(ds);
	//m_waterPaintBlend.Register(main, *nowCamera, ds);
	//m_vignettePostEffect.Register(m_waterPaintBlend.GetResultTex());

	m_vignettePostEffect.Register(main);

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
}

