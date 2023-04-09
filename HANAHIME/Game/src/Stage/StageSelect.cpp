#include "StageSelect.h"

StageSelect::StageSelect()
{
	//ステージ選択用のゲート生成----------------------------------------
	m_gateDataArray = ConvertModelToGateData::Convert("resource/user/level/", "Home.json", 5.0f);
	for (auto &obj : m_gateDataArray)
	{
		m_gateArray.emplace_back(std::make_unique<Gate>(obj.m_transform, obj.m_stageNum));
	}
	//ステージ選択用のゲート生成----------------------------------------
}

void StageSelect::Init()
{
}

void StageSelect::Update()
{
	for (auto &obj : m_gateArray)
	{
		obj->Update();
	}
}

void StageSelect::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
#ifdef _DEBUG
	for (auto &obj : m_gateArray)
	{
		obj->DebugDraw(arg_cam, arg_ligMgr);
	}
#endif
}

int StageSelect::GetStageNumber(const KuroEngine::Vec3<float> &player_pos)
{
	for (auto &obj : m_gateArray)
	{
		if (obj->IsHit(player_pos))
		{
			return obj->GetStageNum();
		}
	}
	return -1;
}
