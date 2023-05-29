#include "SaveDataManager.h"
#include"../Stage/StageManager.h"

//データファイルのディレクトリ
const std::string SaveDataManager::DIR = "resource/user/data/";
//データファイルの名前
const std::string SaveDataManager::FILE_NAME = "info";
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
		if (dataSize == sizeof(GameSaveData))
		{
			LoadData(fp, FILE_NAME, &m_saveData, dataSize, 1);
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
		size_t dataSize = sizeof(GameSaveData);
		SaveData(fp, FILE_NAME + "'s Size", &dataSize, sizeof(size_t), 1);
		SaveData(fp, FILE_NAME, &m_saveData, sizeof(GameSaveData), 1);

		//ファイルを閉じる
		fclose(fp);
	}
}

void SaveDataManager::Save(int arg_checkPointOrder)
{
	m_saveData.m_reachStageNum = StageManager::Instance()->GetNowStageIdx();
	m_saveData.m_reachCheckPointOrder = arg_checkPointOrder;
}
