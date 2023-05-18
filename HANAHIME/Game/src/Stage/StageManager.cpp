#include "StageManager.h"
#include"Stage.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"../Graphics/BasicDraw.h"
#include"../Movie/CameraData.h"
#include"../Player/Player.h"

StageManager::StageManager()
	:KuroEngine::Debugger("StageManager", true, true)
{
	//�f�o�b�K�ł̃J�X�^���p�����[�^�ǉ�
	AddCustomParameter("Skydome", { "scaling", "skydome" }, PARAM_TYPE::FLOAT, &m_skydomeScaling, "Scaling");
	AddCustomParameter("Woods_Radius", { "scaling", "woods", "radius" }, PARAM_TYPE::FLOAT, &m_woodsRadius, "Scaling");
	AddCustomParameter("Woods_Height", { "scaling", "woods", "height" }, PARAM_TYPE::FLOAT, &m_woodsHeight, "Scaling");
	LoadParameterLog();

	//�X�e�[�W��json�t�@�C���̏���
	std::string stageDir = "resource/user/level/";

	float terrianScaling = 1.5f;

	//�z�[���X�e�[�W
	m_homeStage = std::make_shared<Stage>();
	//m_homeStage->Load(0, stageDir, "New_Home.json", 5.0f, false);
	m_homeStage->Load(0, stageDir, "P_Stage_1.json", terrianScaling, false);

	//�p�Y���X�e�[�W�ꊇ�ǂݍ���
	int loadPazzleIdx = 1;
	while (KuroEngine::ExistFile(stageDir + "P_Stage_" + std::to_string(loadPazzleIdx) + ".json"))
	{
		m_stageArray.emplace_back(std::make_shared<Stage>());
		m_stageArray.back()->Load(loadPazzleIdx, stageDir, "P_Stage_" + std::to_string(loadPazzleIdx++) + ".json", terrianScaling, false);
	}

	//���݂̃X�e�[�W�w��i�f�t�H���g�̓z�[���p�X�e�[�W�j
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

	m_nowMapPinPointIdx = 0;
}

void StageManager::Update(Player& arg_player)
{
	m_nowStage->Update(arg_player);
	Appearance::ModelsUpdate();

	//�}�b�v�s���������ړI�n�Ƃ̓����蔻��
	const auto& mapPinPointArray = m_nowStage->GetMapPinPointArray();
	//�}�b�v�s���C���f�b�N�X���͈͊O�łȂ�
	if (!(m_nowMapPinPointIdx < 0 || static_cast<int>(mapPinPointArray.size()) <= m_nowMapPinPointIdx))
	{
		//�ړI�n�_���W�擾
		const auto destPos = mapPinPointArray[m_nowMapPinPointIdx].lock()->GetTransform().GetPosWorld();
		//�}�b�v�s���̓����蔻�蔼�a
		const float MAP_PIN_RADIUS = 10.0f;
		if (destPos.DistanceSq(arg_player.GetTransform().GetPosWorld()) < MAP_PIN_RADIUS * MAP_PIN_RADIUS)
		{
			//�}�b�v�s�������̖ړI�n�ɕύX
			m_nowMapPinPointIdx++;

			//�S�Ă̖ړI�n�����񊮗�
			if (static_cast<int>(mapPinPointArray.size()) <= m_nowMapPinPointIdx)m_nowStage->SetCompleteMapPinFlg(true);
		}
	}
	
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

	m_nowStage->Draw(arg_cam, arg_ligMgr);
}

void StageManager::DrawUI(KuroEngine::Camera& arg_cam, KuroEngine::Vec3<float>arg_playerPos)
{
	//�S�Ă̖ړI�n�����
	if (m_nowStage->GetCompleteMapPin())return;

	const auto& mapPinPointArray = m_nowStage->GetMapPinPointArray();

	//�f�o�b�O�p�ɃX�^�[�g�n�_��ڕW�n�_�Ƃ���
	auto destPos = mapPinPointArray[m_nowMapPinPointIdx].lock()->GetTransform().GetPosWorld();
	m_mapPinUI.Draw(arg_cam, destPos, arg_playerPos);
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