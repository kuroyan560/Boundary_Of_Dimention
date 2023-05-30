#pragma once
#include"Common/Singleton.h"
#include<string>
#include<map>
#include<vector>

struct SoundVolumeData
{
	//マスター音量
	float m_masterVolume = 1.0f;
	//サウンドエフェクト音量
	float m_seVolume = 1.0f;
	//背景音楽音量
	float m_bgmVolume = 1.0f;
};

struct OperationData
{
	//カメラミラーX
	bool m_camMirrorX = false;
	//カメラミラーY
	bool m_camMirrorY = false;
	//カメラ感度
	float m_camSensitivity = 1.0f;
};

struct GameSaveData
{
	static const int INVALID = -1;

	struct GetStarCoinInfo
	{
		int m_stageNum;
		int m_id;
		GetStarCoinInfo() {}
		GetStarCoinInfo(int arg_stageNum, int arg_id) :m_stageNum(arg_stageNum), m_id(arg_id) {}
	};

	//ステージ番号 → 入手したスターコインのID
	std::vector<GetStarCoinInfo>m_getStarCoinInfoArray;

	//たどり着いたステージ
	int m_reachStageNum = INVALID;
	//最後にたどり着いたチェックポイントのID（CheckPointOrder）
	int m_reachCheckPointOrder = INVALID;
	
	//サウンド設定データ
	SoundVolumeData m_soundVol;
	//操作設定データ
	OperationData m_operationSetting;

	bool IsInvalid()const { return m_reachStageNum == INVALID || m_reachCheckPointOrder == INVALID; }
};

class SaveDataManager : public KuroEngine::DesignPattern::Singleton<SaveDataManager>
{
	friend class KuroEngine::DesignPattern::Singleton<SaveDataManager>;
	SaveDataManager();

	friend class SystemSetting;

	//データファイルのディレクトリ
	static const std::string DIR;
	//データファイルの名前
	static const std::string FILE_NAME;
	//データファイルの総合パス
	static const std::string FILE_PATH;

	GameSaveData m_saveData;

public:
	~SaveDataManager();
	void SaveCheckPointOrder(int arg_checkPointOrder);
	void SaveStageNum(int arg_stageNum);
	void SaveStarCoin(int arg_stageNum, int arg_id)
	{
		m_saveData.m_getStarCoinInfoArray.emplace_back(arg_stageNum, arg_id);
	}

	//ステージに関するセーブデータ読み込み
	bool LoadStageSaveData(int* arg_reachStageNum, int* arg_reachCheckPointOreder)
	{ 
		if (arg_reachStageNum)*arg_reachStageNum = m_saveData.m_reachStageNum;
		if (arg_reachCheckPointOreder)*arg_reachCheckPointOreder = m_saveData.m_reachCheckPointOrder;
		return m_saveData.m_reachStageNum != GameSaveData::INVALID && m_saveData.m_reachCheckPointOrder != GameSaveData::INVALID;
	}

	//既に入手したスターコインか
	bool IsGetStarCoin(int arg_stageNum, int arg_id)const
	{
		for (auto& coin : m_saveData.m_getStarCoinInfoArray)
		{
			if (coin.m_stageNum == arg_stageNum && coin.m_id == arg_id)return true;
		}

		return false;
	}

	//サウンドボリュームに関するデータ
	const SoundVolumeData& GetSoundVolumeData() { return m_saveData.m_soundVol; }

	//操作方法に関するデータ
	const OperationData& GetOperationData() { return m_saveData.m_operationSetting; }

	//データ削除
	void ResetSaveData()
	{
		m_saveData.m_reachStageNum = GameSaveData::INVALID;
		m_saveData.m_reachCheckPointOrder = GameSaveData::INVALID;
		m_saveData.m_getStarCoinInfoArray.clear();
	}
};

