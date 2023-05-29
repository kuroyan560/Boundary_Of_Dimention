#pragma once
#include"Common/Singleton.h"

struct SoundVolumeData
{
	//�}�X�^�[����
	float m_masterVolume = 1.0f;
	//�T�E���h�G�t�F�N�g����
	float m_seVolume = 1.0f;
	//�w�i���y����
	float m_bgmVolume = 1.0f;
};

struct SaveData
{
	static const int INVALID = -1;

	//���ǂ蒅�����X�e�[�W
	int m_reachStageNum = INVALID;
	//�Ō�ɂ��ǂ蒅�����`�F�b�N�|�C���g��ID�iCheckPointOrder�j
	int m_reachCheckPointOrder = INVALID;
	
	SoundVolumeData m_soundVol;

	bool IsInvalid()const { return m_reachStageNum == INVALID || m_reachCheckPointOrder == INVALID; }
};

class SaveDataManager : public KuroEngine::DesignPattern::Singleton<SaveDataManager>
{
	friend class KuroEngine::DesignPattern::Singleton<SaveDataManager>;
	SaveDataManager();

	friend class SystemSetting;

	void Load();
	SaveData m_saveData;

public:
	void Save(int arg_checkPointOrder);

	//�X�e�[�W�Ɋւ���Z�[�u�f�[�^�ǂݍ���
	bool LoadStageSaveData(int* arg_reachStageNum, int* arg_reachCheckPointOreder)
	{ 
		if (arg_reachStageNum)*arg_reachStageNum = m_saveData.m_reachStageNum;
		if (arg_reachCheckPointOreder)*arg_reachCheckPointOreder = m_saveData.m_reachCheckPointOrder;
		return m_saveData.m_reachStageNum != SaveData::INVALID && m_saveData.m_reachCheckPointOrder != SaveData::INVALID;
	}

	//�T�E���h�{�����[���Ɋւ���f�[�^
	const SoundVolumeData& GetSoundVolumeData() { return m_saveData.m_soundVol; }
};

