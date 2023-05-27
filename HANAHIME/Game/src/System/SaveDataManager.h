#pragma once
#include"Common/Singleton.h"
class SaveDataManager : public KuroEngine::DesignPattern::Singleton<SaveDataManager>
{
	friend class KuroEngine::DesignPattern::Singleton<SaveDataManager>;
	SaveDataManager() {}

public:
	//セーブデータが存在するか
	bool IsExistSaveData() { return false; }
};

