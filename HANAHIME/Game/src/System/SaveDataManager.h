#pragma once
#include"Common/Singleton.h"

struct SaveData
{
	const int INVALID = -1;

	//たどり着いたステージ
	int m_reachStageNum = INVALID;
	//最後にたどり着いたチェックポイントのID（CheckPointOrder）
	int m_reachCheckPointOrder = INVALID;

	bool IsInvalid()const { return m_reachStageNum == INVALID || m_reachCheckPointOrder == INVALID; }
};

class SaveDataManager : public KuroEngine::DesignPattern::Singleton<SaveDataManager>
{
	friend class KuroEngine::DesignPattern::Singleton<SaveDataManager>;
	SaveDataManager() {}

	SaveData m_saveData;

public:
	void Save(int arg_checkPointOrder);

	//セーブデータが存在するか
	bool LoadSaveData(SaveData* arg_loadData) 
	{ 
		if (arg_loadData)
		{
			arg_loadData->m_reachStageNum = m_saveData.m_reachStageNum;
			arg_loadData->m_reachCheckPointOrder = m_saveData.m_reachCheckPointOrder;
		}
		return !m_saveData.IsInvalid(); 
	}
};

