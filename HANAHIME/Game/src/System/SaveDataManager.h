#pragma once
#include"Common/Singleton.h"
class SaveDataManager : public KuroEngine::DesignPattern::Singleton<SaveDataManager>
{
	friend class KuroEngine::DesignPattern::Singleton<SaveDataManager>;
	SaveDataManager() {}

	struct SaveData
	{
		//���ǂ蒅�����X�e�[�W
		int m_reachStageNum;
		//�Ō�ɂ��ǂ蒅�����`�F�b�N�|�C���g��ID�iCheckPointOrder�j
		int m_reachCheckPointOrder;
	}m_saveData;

public:
	void Save(int arg_checkPointOrder);

	//�Z�[�u�f�[�^�����݂��邩
	bool IsExistSaveData() { return false; }
};

