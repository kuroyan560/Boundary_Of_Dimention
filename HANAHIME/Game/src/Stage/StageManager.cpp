#include "StageManager.h"
#include"Stage.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"../Graphics/BasicDraw.h"

StageManager::StageManager()
	:KuroEngine::Debugger("StageManager", true, true)
{
	//デバッガでのカスタムパラメータ追加
	AddCustomParameter("Skydome", { "scaling", "skydome" }, PARAM_TYPE::FLOAT, &m_skydomeScaling, "Scaling");
	AddCustomParameter("Woods_Radius", { "scaling", "woods", "radius" }, PARAM_TYPE::FLOAT, &m_woodsRadius, "Scaling");
	AddCustomParameter("Woods_Height", { "scaling", "woods", "height" }, PARAM_TYPE::FLOAT, &m_woodsHeight, "Scaling");
	LoadParameterLog();

	//動く足場の確認用ステージ
	m_stageArray.emplace_back(std::make_shared<Stage>());
	m_stageArray.back()->Load("resource/user/level/", "LoadTestStage.json", 1.0f);

	//テスト用ステージ生成
	m_stageArray.emplace_back(std::make_shared<Stage>());
	m_stageArray.back()->Load("resource/user/level/", "Debug_Stage_1.json", 5.0f);

	//ホームステージ
	m_stageArray.emplace_back(std::make_shared<Stage>());
	m_stageArray.back()->Load("resource/user/level/", "New_Home.json", 5.0f, true);

	//現在のステージ指定（デフォルトはホーム用ステージ）
	m_nowStage = m_stageArray[2];
}

void StageManager::SetStage(int stage_num)
{
	m_nowStage = m_stageArray[stage_num];
	m_nowStage->TerrianInit();
}

void StageManager::Init()
{
	m_nowStage->TerrianInit();
}

void StageManager::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	using namespace KuroEngine;

	Transform transform;

	//スカイドーム
	transform.SetScale(m_skydomeScaling);
	BasicDraw::Instance()->Draw(arg_cam,
		arg_ligMgr,
		m_nowStage->GetSkydomeModel().lock(),
		transform);
	//DrawFunc3D::DrawNonShadingModel(
	//	m_nowStage->GetSkydomeModel().lock(),
	//	transform,
	//	arg_cam);

	//地面
	transform.SetPos({ 0.0f,-0.1f,0.0f });
	transform.SetScale({ m_groundScaling,m_groundScaling,1.0f });
	transform.SetRotate({ 1.0f,0.0f,0.0f }, Angle(90));
	//DrawFunc3D::DrawNonShadingPlane(
	//	m_nowStage->GetGroundTex(),
	//	transform,
	//	arg_cam);

	//森林円柱
	transform.SetPos({ 0.0f,0.5f * m_woodsHeight,0.0f });
	transform.SetRotate(XMMatrixIdentity());
	transform.SetScale({ m_woodsRadius,m_woodsHeight,m_woodsRadius });
	BasicDraw::Instance()->Draw(arg_cam,
		arg_ligMgr,
		m_nowStage->GetWoodsCylinderModel().lock(),
		transform);
	//DrawFunc3D::DrawNonShadingModel(
	//	m_nowStage->GetWoodsCylinderModel().lock(),
	//	transform,
	//	arg_cam);


	m_nowStage->TerrianDraw(arg_cam, arg_ligMgr);
}

KuroEngine::Transform StageManager::GetPlayerSpawnTransform() const
{
	return m_nowStage->GetPlayerSpawnTransform();
}
