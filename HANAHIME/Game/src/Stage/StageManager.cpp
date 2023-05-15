#include "StageManager.h"
#include"Stage.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"../Graphics/BasicDraw.h"
#include"../Movie/CameraData.h"

StageManager::StageManager()
	:KuroEngine::Debugger("StageManager", true, true)
{
	//デバッガでのカスタムパラメータ追加
	AddCustomParameter("Skydome", { "scaling", "skydome" }, PARAM_TYPE::FLOAT, &m_skydomeScaling, "Scaling");
	AddCustomParameter("Woods_Radius", { "scaling", "woods", "radius" }, PARAM_TYPE::FLOAT, &m_woodsRadius, "Scaling");
	AddCustomParameter("Woods_Height", { "scaling", "woods", "height" }, PARAM_TYPE::FLOAT, &m_woodsHeight, "Scaling");
	LoadParameterLog();

	//ステージのjsonファイルの所在
	std::string stageDir = "resource/user/level/";

	float terrianScaling = 1.5f;

	//ホームステージ
	m_homeStage = std::make_shared<Stage>();
	//m_homeStage->Load(0, stageDir, "New_Home.json", 5.0f, false);
	m_homeStage->Load(0, stageDir, "P_Stage_1.json", terrianScaling, false);

	//パズルステージ一括読み込み
	int loadPazzleIdx = 1;
	while (KuroEngine::ExistFile(stageDir + "P_Stage_" + std::to_string(loadPazzleIdx) + ".json"))
	{
		m_stageArray.emplace_back(std::make_shared<Stage>());
		m_stageArray.back()->Load(loadPazzleIdx, stageDir, "P_Stage_" + std::to_string(loadPazzleIdx++) + ".json", terrianScaling, false);
	}

	//現在のステージ指定（デフォルトはホーム用ステージ）
	m_nowStage = m_homeStage;

	CameraData::Instance()->RegistCameraData("");
}

void StageManager::SetStage(int stage_num)
{
	if (stage_num == -1)
	{
		m_nowStage = m_homeStage;
	}
	else
	{
		m_nowStage = m_stageArray[stage_num];
	}
	m_nowStage->Init();
}

void StageManager::Update(Player& arg_player)
{
	m_nowStage->Update(arg_player);
	Appearance::ModelsUpdate();
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


	m_nowStage->Draw(arg_cam, arg_ligMgr);
}

KuroEngine::Transform StageManager::GetGateTransform(int arg_stageIdx, int arg_gateID) const
{
	return m_stageArray[arg_stageIdx]->GetGateTransform(arg_gateID);
}

bool StageManager::IsClearNowStage() const
{
	return m_nowStage->IsClear();
}

KuroEngine::Transform StageManager::GetPlayerSpawnTransform() const
{
	return m_nowStage->GetPlayerSpawnTransform();
}

KuroEngine::Transform StageManager::GetGoalTransform() const
{
	return m_nowStage->GetGoalTransform();
}

std::shared_ptr<GoalPoint>StageManager::GetGoalModel()
{
	return m_nowStage->GetGoalModel();
}