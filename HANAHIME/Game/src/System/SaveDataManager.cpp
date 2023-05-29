#include "SaveDataManager.h"
#include"../Stage/StageManager.h"

//�f�[�^�t�@�C���̃f�B���N�g��
const std::string SaveDataManager::DIR = "resource/user/data/";
//�f�[�^�t�@�C���̖��O
const std::string SaveDataManager::FILE_NAME = "info";
//�f�[�^�t�@�C���̑����p�X
const std::string SaveDataManager::FILE_PATH = DIR + FILE_NAME;

SaveDataManager::SaveDataManager()
{
	using namespace KuroEngine;

	//�f�[�^�Ȃ�
	if (!ExistFile(FILE_PATH))return;

	//�t�@�C�����J��
	FILE* fp;
	fopen_s(&fp, FILE_PATH.c_str(), "rb");

	if (fp != NULL)
	{
		//���[�h
		size_t dataSize;
		LoadData(fp, FILE_NAME + "'s Size", &dataSize, sizeof(size_t), 1);
		if (dataSize == sizeof(GameSaveData))
		{
			LoadData(fp, FILE_NAME, &m_saveData, dataSize, 1);
		}
		//�t�@�C�������
		fclose(fp);
	}
}

SaveDataManager::~SaveDataManager()
{
	using namespace KuroEngine;

	//�t�@�C�����J��
	FILE* fp;
	fopen_s(&fp, FILE_PATH.c_str(), "wb");

	if (fp != NULL)
	{
		//�Z�[�u
		size_t dataSize = sizeof(GameSaveData);
		SaveData(fp, FILE_NAME + "'s Size", &dataSize, sizeof(size_t), 1);
		SaveData(fp, FILE_NAME, &m_saveData, sizeof(GameSaveData), 1);

		//�t�@�C�������
		fclose(fp);
	}
}

void SaveDataManager::Save(int arg_checkPointOrder)
{
	m_saveData.m_reachStageNum = StageManager::Instance()->GetNowStageIdx();
	m_saveData.m_reachCheckPointOrder = arg_checkPointOrder;
}
