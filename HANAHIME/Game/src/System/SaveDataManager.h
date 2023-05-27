#pragma once
#include"Common/Singleton.h"
class SaveDataManager : public KuroEngine::DesignPattern::Singleton<SaveDataManager>
{
	friend class KuroEngine::DesignPattern::Singleton<SaveDataManager>;
	SaveDataManager() {}

	struct SaveData
	{
		//たどり着いたステージ
		int m_reachStageNum;
		//最後にたどり着いたチェックポイントのID（CheckPointOrder）
		int m_reachCheckPointOrder;
	}m_saveData;

public:
	void Save(int arg_checkPointOrder);

	//セーブデータが存在するか
	bool IsExistSaveData() { return false; }
};

