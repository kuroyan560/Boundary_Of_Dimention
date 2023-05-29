#include "SaveDataManager.h"
#include"../Stage/StageManager.h"

void SaveDataManager::Save(int arg_checkPointOrder)
{
	m_saveData.m_reachStageNum = StageManager::Instance()->GetNowStageIdx();
	m_saveData.m_reachCheckPointOrder = arg_checkPointOrder;
}
