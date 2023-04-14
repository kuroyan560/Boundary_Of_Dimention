#include "StageManager.h"
#include"Stage.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"../Graphics/BasicDraw.h"

StageManager::StageManager()
	:KuroEngine::Debugger("StageManager", true, true)
{
	//�f�o�b�K�ł̃J�X�^���p�����[�^�ǉ�
	AddCustomParameter("Skydome", { "scaling", "skydome" }, PARAM_TYPE::FLOAT, &m_skydomeScaling, "Scaling");
	AddCustomParameter("Woods_Radius", { "scaling", "woods", "radius" }, PARAM_TYPE::FLOAT, &m_woodsRadius, "Scaling");
	AddCustomParameter("Woods_Height", { "scaling", "woods", "height" }, PARAM_TYPE::FLOAT, &m_woodsHeight, "Scaling");
	LoadParameterLog();

	//�z�[���X�e�[�W
	m_homeStage = std::make_shared<Stage>();
	m_homeStage->Load("resource/user/level/", "New_Home.json", 5.0f, false);

	float terrianScaling = 1.5f;

	m_stageArray.emplace_back(std::make_shared<Stage>());
	m_stageArray.back()->Load("resource/user/level/", "P_Stage_1.json", terrianScaling);
	m_stageArray.emplace_back(std::make_shared<Stage>());
	m_stageArray.back()->Load("resource/user/level/", "P_Stage_2.json", terrianScaling);
	m_stageArray.emplace_back(std::make_shared<Stage>());
	m_stageArray.back()->Load("resource/user/level/", "P_Stage_3.json", terrianScaling);
	m_stageArray.emplace_back(std::make_shared<Stage>());
	m_stageArray.back()->Load("resource/user/level/", "P_Stage_4.json", terrianScaling);
	m_stageArray.emplace_back(std::make_shared<Stage>());
	m_stageArray.back()->Load("resource/user/level/", "P_Stage_5.json", terrianScaling);
	m_stageArray.emplace_back(std::make_shared<Stage>());
	m_stageArray.back()->Load("resource/user/level/", "P_Stage_6.json", terrianScaling);
	m_stageArray.emplace_back(std::make_shared<Stage>());
	m_stageArray.back()->Load("resource/user/level/", "P_Stage_7.json", terrianScaling);
	m_stageArray.emplace_back(std::make_shared<Stage>());
	m_stageArray.back()->Load("resource/user/level/", "P_Stage_8.json", terrianScaling);
	m_stageArray.emplace_back(std::make_shared<Stage>());
	m_stageArray.back()->Load("resource/user/level/", "P_Stage_9.json", terrianScaling);
	m_stageArray.emplace_back(std::make_shared<Stage>());
	m_stageArray.back()->Load("resource/user/level/", "P_Stage_10.json", terrianScaling);


	//���݂̃X�e�[�W�w��i�f�t�H���g�̓z�[���p�X�e�[�W�j
	m_nowStage = m_homeStage;
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
	m_nowStage->GimmickInit();
}

void StageManager::Update(Player& arg_player)
{
	m_nowStage->GimmickUpdate(arg_player);
}

void StageManager::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	using namespace KuroEngine;

	Transform transform;

	//�X�J�C�h�[��
	transform.SetScale(m_skydomeScaling);
	BasicDraw::Instance()->Draw(arg_cam,
		arg_ligMgr,
		m_nowStage->GetSkydomeModel().lock(),
		transform);
	//DrawFunc3D::DrawNonShadingModel(
	//	m_nowStage->GetSkydomeModel().lock(),
	//	transform,
	//	arg_cam);

	//�n��
	transform.SetPos({ 0.0f,-0.1f,0.0f });
	transform.SetScale({ m_groundScaling,m_groundScaling,1.0f });
	transform.SetRotate({ 1.0f,0.0f,0.0f }, Angle(90));
	//DrawFunc3D::DrawNonShadingPlane(
	//	m_nowStage->GetGroundTex(),
	//	transform,
	//	arg_cam);

	//�X�щ~��
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

KuroEngine::Transform StageManager::GetGoalTransform() const
{
	return m_nowStage->GetGoalTransform();
}

std::shared_ptr<GoalPoint>StageManager::GetGoalModel()
{
	return m_nowStage->GetGoalModel();
}