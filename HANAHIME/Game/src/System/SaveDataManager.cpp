#include "SaveDataManager.h"
#include"../Stage/StageManager.h"

//データファイルのディレクトリ
const std::string SaveDataManager::DIR = "resource/user/data/";
//データファイルの名前
const std::string SaveDataManager::FILE_NAME = "info.sd";
//データファイルの総合パス
const std::string SaveDataManager::FILE_PATH = DIR + FILE_NAME;

SaveDataManager::SaveDataManager()
{
	using namespace KuroEngine;

	//データなし
	if (!ExistFile(FILE_PATH))return;

	//ファイルを開く
	FILE* fp;
	fopen_s(&fp, FILE_PATH.c_str(), "rb");

	if (fp != NULL)
	{
		//ロード
		size_t dataSize;
		LoadData(fp, FILE_NAME + "'s Size", &dataSize, sizeof(size_t), 1);
		
		size_t starCoinNum;
		LoadData(fp, FILE_NAME + "'s StarCoinNum", &starCoinNum, sizeof(size_t), 1);
		m_saveData.m_getStarCoinInfoArray.resize(starCoinNum);

		if (dataSize == sizeof(m_saveData))
		{
			LoadData(fp, FILE_NAME, m_saveData.m_getStarCoinInfoArray.data(), sizeof(GameSaveData::GetStarCoinInfo), static_cast<int>(starCoinNum));

			LoadData(fp, FILE_NAME, &m_saveData.m_reachStageNum, sizeof(int), 1);
			LoadData(fp, FILE_NAME, &m_saveData.m_reachCheckPointOrder, sizeof(int), 1);

			LoadData(fp, FILE_NAME, &m_saveData.m_soundVol, sizeof(SoundVolumeData), 1);
			LoadData(fp, FILE_NAME, &m_saveData.m_operationSetting, sizeof(OperationData), 1);
		}
		//ファイルを閉じる
		fclose(fp);
	}
}

SaveDataManager::~SaveDataManager()
{
	using namespace KuroEngine;

	//ファイルを開く
	FILE* fp;
	fopen_s(&fp, FILE_PATH.c_str(), "wb");

	if (fp != NULL)
	{
		//セーブ
		size_t dataSize = sizeof(m_saveData);
		SaveData(fp, FILE_NAME + "'s Size", &dataSize, sizeof(size_t), 1);

		size_t starCoinNum = m_saveData.m_getStarCoinInfoArray.size();
		SaveData(fp, FILE_NAME + "'s StarCoinNum", &starCoinNum, sizeof(size_t), 1);

		SaveData(fp, FILE_NAME, m_saveData.m_getStarCoinInfoArray.data(), sizeof(GameSaveData::GetStarCoinInfo), static_cast<int>(starCoinNum));

		SaveData(fp, FILE_NAME, &m_saveData.m_reachStageNum, sizeof(int), 1);
		SaveData(fp, FILE_NAME, &m_saveData.m_reachCheckPointOrder, sizeof(int), 1);

		SaveData(fp, FILE_NAME, &m_saveData.m_soundVol, sizeof(SoundVolumeData), 1);
		SaveData(fp, FILE_NAME, &m_saveData.m_operationSetting, sizeof(OperationData), 1);

		//ファイルを閉じる
		fclose(fp);
	}
}

void SaveDataManager::SaveCheckPointOrder(int arg_checkPointOrder)
{
	m_saveData.m_reachCheckPointOrder = arg_checkPointOrder;
}

void SaveDataManager::SaveStageNum(int arg_stageNum)
{
	m_saveData.m_reachStageNum = arg_stageNum;
}
