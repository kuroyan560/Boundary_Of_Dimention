#pragma once
#include"Common/Singleton.h"
class SaveDataManager : public KuroEngine::DesignPattern::Singleton<SaveDataManager>
{
	friend class KuroEngine::DesignPattern::Singleton<SaveDataManager>;
	SaveDataManager() {}

public:
	//�Z�[�u�f�[�^�����݂��邩
	bool IsExistSaveData() { return false; }
};

