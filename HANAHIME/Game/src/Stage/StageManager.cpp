#include "StageManager.h"
#include"Stage.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"

StageManager::StageManager()
	:KuroEngine::Debugger("StageManager", true, true)
{
	//�e�X�g�p�X�e�[�W����
	m_testStage = std::make_shared<Stage>();
	m_testStage->Load("resource/user/level/", "Debug_2.json");

	//���݂̃X�e�[�W�w��i�f�t�H���g�̓e�X�g�p�X�e�[�W�j
	m_nowStage = m_testStage;

	//�f�o�b�K�ł̃J�X�^���p�����[�^�ǉ�
	AddCustomParameter("Skydome", { "scaling", "skydome" }, PARAM_TYPE::FLOAT, &m_skydomeScaling, "Scaling");
	AddCustomParameter("Woods_Radius", { "scaling", "woods", "radius" }, PARAM_TYPE::FLOAT, &m_woodsRadius, "Scaling");
	AddCustomParameter("Woods_Height", { "scaling", "woods", "height" }, PARAM_TYPE::FLOAT, &m_woodsHeight, "Scaling");
	AddCustomParameter("Ground", { "scaling", "ground" }, PARAM_TYPE::FLOAT, &m_groundScaling, "Scaling");
}

void StageManager::OnImguiItems()
{
}

void StageManager::Draw(KuroEngine::Camera& arg_cam)
{
	using namespace KuroEngine;

	Transform transform;

	////�X�J�C�h�[��
	//transform.SetScale(m_skydomeScaling);
	//DrawFunc3D::DrawNonShadingModel(
	//	m_nowStage->GetSkydomeModel().lock(),
	//	transform,
	//	arg_cam);

	////�n��
	//transform.SetScale({ m_groundScaling,m_groundScaling,1.0f });
	//transform.SetRotate({ 1.0f,0.0f,0.0f }, Angle(90));
	//DrawFunc3D::DrawNonShadingPlane(
	//	m_nowStage->GetGroundTex(),
	//	transform,
	//	arg_cam);

	////�X�щ~��
	//transform.SetPos({ 0.0f,0.5f * m_woodsHeight,0.0f });
	//transform.SetRotate(XMMatrixIdentity());
	//transform.SetScale({ m_woodsRadius,m_woodsHeight,m_woodsRadius });
	//DrawFunc3D::DrawNonShadingModel(
	//	m_nowStage->GetWoodsCylinderModel().lock(),
	//	transform,
	//	arg_cam);

	//�n�`�`��
	for (auto& terrian : m_nowStage->GetTerrianArray())
	{
		DrawFunc3D::DrawNonShadingModel(
			terrian.m_model.lock(),
			terrian.m_transform,
			arg_cam);
	}

}