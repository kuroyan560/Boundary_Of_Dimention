#pragma once
#include"Common/Singleton.h"

struct SoundVolumeData
{
	//マスター音量
	float m_masterVolume = 1.0f;
	//サウンドエフェクト音量
	float m_seVolume = 1.0f;
	//背景音楽音量
	float m_bgmVolume = 1.0f;
};

struct SaveData
{
	static const int INVALID = -1;

	//たどり着いたステージ
	int m_reachStageNum = INVALID;
	//最後にたどり着いたチェックポイントのID（CheckPointOrder）
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

	//ステージに関するセーブデータ読み込み
	bool LoadStageSaveData(int* arg_reachStageNum, int* arg_reachCheckPointOreder)
	{ 
		if (arg_reachStageNum)*arg_reachStageNum = m_saveData.m_reachStageNum;
		if (arg_reachCheckPointOreder)*arg_reachCheckPointOreder = m_saveData.m_reachCheckPointOrder;
		return m_saveData.m_reachStageNum != SaveData::INVALID && m_saveData.m_reachCheckPointOrder != SaveData::INVALID;
	}

	//サウンドボリュームに関するデータ
	const SoundVolumeData& GetSoundVolumeData() { return m_saveData.m_soundVol; }
};

