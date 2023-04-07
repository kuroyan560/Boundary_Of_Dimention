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
	AddCustomParameter("Ground", { "scaling", "ground" }, PARAM_TYPE::FLOAT, &m_groundScaling, "Scaling");
	AddCustomParameter("Terrian", { "scaling", "terrian" }, PARAM_TYPE::FLOAT, &m_terrianScaling, "Scaling");
	LoadParameterLog();

	//�e�X�g�p�X�e�[�W����
	m_testStage = std::make_shared<Stage>();
	m_testStage->Load("resource/user/level/", "Debug_Stage_1.json");
	m_testStage->TerrianInit(m_terrianScaling);

	//���݂̃X�e�[�W�w��i�f�t�H���g�̓e�X�g�p�X�e�[�W�j
	m_nowStage = m_testStage;
}

void StageManager::OnImguiItems()
{
	if (m_terrianScaling != m_oldTerrianScaling)
	{
		m_nowStage->TerrianInit(m_terrianScaling);
	}
	m_oldTerrianScaling = m_terrianScaling;
}

void StageManager::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	using namespace KuroEngine;

	Transform transform;

	//�X�J�C�h�[��
	transform.SetScale(m_skydomeScaling);
	DrawFunc3D::DrawNonShadingModel(
		m_nowStage->GetSkydomeModel().lock(),
		transform,
		arg_cam);

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
	DrawFunc3D::DrawNonShadingModel(
		m_nowStage->GetWoodsCylinderModel().lock(),
		transform,
		arg_cam);


	m_nowStage->TerrianDraw(arg_cam, arg_ligMgr);
}