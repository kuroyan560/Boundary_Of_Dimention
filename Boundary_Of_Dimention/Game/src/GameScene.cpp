#include "GameScene.h"
#include"DirectX12/D3D12App.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"

GameScene::GameScene()
{
	m_ddsTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/test.dds");
	m_pngTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/test.png");
}


void GameScene::OnInitialize()
{
	m_debugCam.Init({ 0,0,-10 }, { 0,0,0 });
}

void GameScene::OnUpdate()
{
	m_debugCam.Move();
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

	Transform transform;
	transform.SetPos({ -0.5f,0,0 });
	DrawFunc3D::DrawNonShadingPlane(
		m_ddsTex,
		transform,
		m_debugCam);

	transform.SetPos({ 0.5f,0,0 });
	DrawFunc3D::DrawNonShadingPlane(
		m_pngTex,
		transform,
		m_debugCam);
}

