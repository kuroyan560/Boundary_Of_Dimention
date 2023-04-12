#include "HomeStageSelect.h"

HomeStageSelect::HomeStageSelect()
{
	//ステージ選択用のゲート生成----------------------------------------
	m_gateDataArray = ConvertModelToGateData::Convert("resource/user/level/", "New_Home.json", 5.0f);
	for (auto &obj : m_gateDataArray)
	{
		m_gateArray.emplace_back(std::make_unique<Gate>(obj.m_transform, obj.m_stageNum));
	}
	//ステージ選択用のゲート生成----------------------------------------
}

void HomeStageSelect::Init()
{
}

void HomeStageSelect::Update()
{
	for (auto &obj : m_gateArray)
	{
		obj->Update();
	}
}

void HomeStageSelect::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
#ifdef _DEBUG
	for (auto &obj : m_gateArray)
	{
		obj->DebugDraw(arg_cam, arg_ligMgr);
	}
#endif
}

int HomeStageSelect::GetStageNumber(const KuroEngine::Vec3<float> &player_pos)
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
