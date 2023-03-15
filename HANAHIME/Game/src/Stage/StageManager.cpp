#include "StageManager.h"
#include"Stage.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"

StageManager::StageManager()
	:KuroEngine::Debugger("StageManager", true)
{
	//テスト用ステージ生成
	m_testStage = std::make_shared<Stage>();

	//現在のステージ指定（デフォルトはテスト用ステージ）
	m_nowStage = m_testStage;

	//デバッガでのカスタムパラメータ追加
	AddCustomParameter({ "scaling","skydome" }, PARAM_TYPE::FLOAT, &m_skydomeScaling);
	AddCustomParameter({ "scaling","woods","radius"}, PARAM_TYPE::FLOAT, &m_woodsRadius);
	AddCustomParameter({ "scaling","woods","height"}, PARAM_TYPE::FLOAT, &m_woodsHeight);
	AddCustomParameter({ "scaling","ground" }, PARAM_TYPE::FLOAT, &m_groundScaling);
}

void StageManager::OnImguiItems()
{
	//スケーリング設定
	ImGui::SetNextItemOpen(true);
	ImGui::PushItemWidth(100);
	if (ImGui::TreeNode("Scaling"))
	{
		ImGui::DragFloat("Skydome", &m_skydomeScaling, 0.1f, 0.0f);
		ImGui::DragFloat("Woods_Radius", &m_woodsRadius, 0.1f, 0.0f);
		ImGui::DragFloat("Woods_Height", &m_woodsHeight, 0.1f, 0.0f);
		ImGui::DragFloat("Ground", &m_groundScaling, 0.1f, 0.0f);
		ImGui::TreePop();
	}
}

void StageManager::Draw(KuroEngine::Camera& arg_cam)
{
	using namespace KuroEngine;

	Transform transform;

	//スカイドーム
	transform.SetScale(m_skydomeScaling);
	DrawFunc3D::DrawNonShadingModel(
		m_nowStage->GetSkydomeModel().lock(),
		transform,
		arg_cam);

	//地面
	transform.SetScale({ m_groundScaling,m_groundScaling,1.0f });
	transform.SetRotate({ 1.0f,0.0f,0.0f }, Angle(90));
	DrawFunc3D::DrawNonShadingPlane(
		m_nowStage->GetGroundTex(),
		transform,
		arg_cam);

	//森林円柱
	transform.SetPos({ 0.0f,0.5f * m_woodsHeight,0.0f });
	transform.SetRotate(XMMatrixIdentity());
	transform.SetScale({ m_woodsRadius,m_woodsHeight,m_woodsRadius });
	DrawFunc3D::DrawNonShadingModel(
		m_nowStage->GetWoodsCylinderModel().lock(),
		transform,
		arg_cam);

}