#pragma once
#include"Common/Singleton.h"

struct SaveData
{
	const int INVALID = -1;

	//���ǂ蒅�����X�e�[�W
	int m_reachStageNum = INVALID;
	//�Ō�ɂ��ǂ蒅�����`�F�b�N�|�C���g��ID�iCheckPointOrder�j
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

	//�Z�[�u�f�[�^�����݂��邩
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

