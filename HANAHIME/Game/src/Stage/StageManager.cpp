#include "StageManager.h"
#include"Stage.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"../Graphics/BasicDraw.h"
#include"../Movie/CameraData.h"
#include"../Player/Player.h"
#include"FrameWork/UsersInput.h"
#include"CheckPointHitFlag.h"
#include"../System/SaveDataManager.h"
#include"StageParts.h"

StageManager::StageManager()
	:KuroEngine::Debugger("StageManager", true, true)
{
	//�f�o�b�K�ł̃J�X�^���p�����[�^�ǉ�
	AddCustomParameter("Skydome", { "scaling", "skydome" }, PARAM_TYPE::FLOAT, &m_skydomeScaling, "Scaling");
	LoadParameterLog();

	//�X�e�[�W��json�t�@�C���̏���
	std::string stageDir = "resource/user/level/";

	float terrianScaling = 1.5f;

	//�z�[���X�e�[�W
	m_homeStage = std::make_shared<Stage>();
	//m_homeStage->Load(0, stageDir, "New_Home.json", 5.0f, false);
	//m_homeStage->Load(0, stageDir, "P_Stage_1.json", terrianScaling, false);

	//�p�Y���X�e�[�W�ꊇ�ǂݍ���
	int loadPazzleIdx = 1;
	while (KuroEngine::ExistFile(stageDir + "P_Stage_" + std::to_string(loadPazzleIdx) + ".json"))
	{
		DebugEnemy::Instance()->StackStage();
		m_stageArray.emplace_back(std::make_shared<Stage>());
		m_stageArray.back()->Load(loadPazzleIdx, stageDir, "P_Stage_" + std::to_string(loadPazzleIdx++) + ".json", terrianScaling, false);
	}

	//�f�[�^����`�F�b�N�|�C���g�̉����ݒ�
	int unlockedStageNum, unlockedCheckPointOrder;
	if (SaveDataManager::Instance()->LoadStageSaveData(&unlockedStageNum, &unlockedCheckPointOrder))
	{
		for (int stageIdx = 0; stageIdx <= unlockedStageNum; ++stageIdx)
		{
			for (auto& checkPoint : m_stageArray[stageIdx]->GetCheckPointArray())
			{
				bool touched = checkPoint.lock()->GetOrder() <= unlockedCheckPointOrder;
				if (!touched)touched = stageIdx < unlockedStageNum;

				//�`�F�b�N�|�C���g��G������Ԃɂ���
				if (touched)checkPoint.lock()->SetTouch(true);
			}
		}
	}

	//���݂̃X�e�[�W�w��i�f�t�H���g�̓z�[���p�X�e�[�W�j
	m_nowStageIdx = 0;
	m_nowStage = m_stageArray[m_nowStageIdx];

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
	m_nowStageIdx = stage_num;

	//�`�F�b�N�|�C���gUI������
	CheckPoint::UI().lock()->Init();
}

void StageManager::Update(Player& arg_player)
{
	CheckPointHitFlag::Instance()->m_isHitCheckPointTrigger = false;

	m_nowStage->Update(arg_player);
	Appearance::ModelsUvUpdate(TimeScaleMgr::s_inGame.GetTimeScale());

	//�}�b�v�s���������ړI�n�Ƃ̓����蔻��
	const auto& mapPinPointArray = m_nowStage->GetMapPinPointArray();
	//�}�b�v�s���C���f�b�N�X���͈͊O�łȂ�
	if (!(m_nowMapPinPointIdx < 0 || static_cast<int>(mapPinPointArray.size()) <= m_nowMapPinPointIdx))
	{
		int oldMapPinPointIdx = m_nowMapPinPointIdx;

		for (int pointIdx = m_nowMapPinPointIdx; pointIdx < static_cast<int>(mapPinPointArray.size()); ++pointIdx)
		{
			//�ړI�n�_���W�擾
			const auto destPos = mapPinPointArray[pointIdx].lock()->GetTransform().GetPosWorld();
			//�}�b�v�s���̓����蔻�蔼�a
			const float MAP_PIN_RADIUS = 10.0f;
			if (destPos.DistanceSq(arg_player.GetTransform().GetPosWorld()) < MAP_PIN_RADIUS * MAP_PIN_RADIUS)
			{
				//�}�b�v�s�������̖ړI�n�ɕύX
				m_nowMapPinPointIdx = pointIdx + 1;

				//�S�Ă̖ړI�n�����񊮗�
				if (static_cast<int>(mapPinPointArray.size()) <= m_nowMapPinPointIdx)m_nowStage->SetCompleteMapPinFlg(true);
			}
		}
	}

	KuroEngine::Transform mapPinPos;
	if (GetNowMapPinTransform(&mapPinPos))
	{
		m_nowStage->SetMapPIN(mapPinPos.GetPos());
	}

	//�`�F�b�N�|�C���gUI�X�V
	CheckPoint::UI().lock()->Update();

}

void StageManager::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	using namespace KuroEngine;

	Transform transform;

	m_nowStage->Draw(arg_cam, arg_ligMgr);
}

void StageManager::DrawUI(KuroEngine::Camera& arg_cam, KuroEngine::Vec3<float>arg_playerPos)
{
	//�܂��S�Ă̖ړI�n�����񂵂Ă��Ȃ�
	KuroEngine::Transform mapPinPos;
	if (GetNowMapPinTransform(&mapPinPos))
	{
		const auto& mapPinPointArray = m_nowStage->GetMapPinPointArray();

		m_mapPinUI.Draw(arg_cam, mapPinPos.GetPosWorld(), arg_playerPos);
	}

	//�`�F�b�N�|�C���gUI�`��
	CheckPoint::UI().lock()->Draw();
}

bool StageManager::GetNowMapPinTransform(KuroEngine::Transform* arg_destPos)
{
	if (m_nowStage->GetCompleteMapPin())return false;
	const auto& mapPinPointArray = m_nowStage->GetMapPinPointArray();
	if (m_nowMapPinPointIdx < 0 || mapPinPointArray.size() <= m_nowMapPinPointIdx)return false;
	if (arg_destPos)*arg_destPos = mapPinPointArray[m_nowMapPinPointIdx].lock()->GetTransform();
	return true;
}

KuroEngine::Transform StageManager::GetGateTransform(int arg_stageIdx, int arg_gateID) const
{
	return m_stageArray[arg_stageIdx]->GetGateTransform(arg_gateID);
}

bool StageManager::IsClearNowStage() const
{
	return m_nowStage->IsClear();
}

KuroEngine::Transform StageManager::GetStartPointTransform() const
{
	return m_stageArray[0]->GetStartPointTransform();
}

KuroEngine::Transform StageManager::GetGoalTransform() const
{
	return m_nowStage->GetGoalTransform();
}

std::shared_ptr<GoalPoint>StageManager::GetGoalModel()
{
	return m_nowStage->GetGoalModel();
}

int StageManager::GetStarCoinNum() const
{
	return m_nowStage->GetStarCoinNum();
}

int StageManager::ExistStarCoinNum() const
{
	return m_nowStage->ExistStarCoinNum();
}

bool StageManager::GetUnlockedCheckPointInfo(std::vector<std::vector<KuroEngine::Transform>>* arg_transformArray, int* arg_recentStageNum, int* arg_recentIdx) const
{
	if (!arg_transformArray || !arg_recentStageNum || !arg_recentIdx)return false;

	int reachStageNum, reachCheckPointOrder;
	if (!SaveDataManager::Instance()->LoadStageSaveData(&reachStageNum, &reachCheckPointOrder))return false;

	arg_transformArray->clear();

	for (int stageIdx = 0; stageIdx <= reachStageNum; ++stageIdx)
	{
		arg_transformArray->emplace_back();
		for (auto& checkPoint : m_stageArray[stageIdx]->GetCheckPointArray())
		{
			if (stageIdx == reachStageNum && reachCheckPointOrder < checkPoint.lock()->GetOrder())break;

			arg_transformArray->back().emplace_back(checkPoint.lock()->GetInitTransform());
			if (stageIdx == reachStageNum && reachCheckPointOrder == checkPoint.lock()->GetOrder())*arg_recentIdx = static_cast<int>(arg_transformArray->back().size()) - 1;
		}
	}

	*arg_recentStageNum = reachStageNum;

	return true;
}

void StageManager::AllStageCheckPointReset()
{
	for (auto& stage : m_stageArray)stage->CheckPointReset();
}
