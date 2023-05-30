#pragma once
#include"Common/Singleton.h"
#include<string>
#include<map>
#include<vector>

struct SoundVolumeData
{
	//�}�X�^�[����
	float m_masterVolume = 1.0f;
	//�T�E���h�G�t�F�N�g����
	float m_seVolume = 1.0f;
	//�w�i���y����
	float m_bgmVolume = 1.0f;
};

struct OperationData
{
	//�J�����~���[X
	bool m_camMirrorX = false;
	//�J�����~���[Y
	bool m_camMirrorY = false;
	//�J�������x
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

	//�X�e�[�W�ԍ� �� ���肵���X�^�[�R�C����ID
	std::vector<GetStarCoinInfo>m_getStarCoinInfoArray;

	//���ǂ蒅�����X�e�[�W
	int m_reachStageNum = INVALID;
	//�Ō�ɂ��ǂ蒅�����`�F�b�N�|�C���g��ID�iCheckPointOrder�j
	int m_reachCheckPointOrder = INVALID;
	
	//�T�E���h�ݒ�f�[�^
	SoundVolumeData m_soundVol;
	//����ݒ�f�[�^
	OperationData m_operationSetting;

	bool IsInvalid()const { return m_reachStageNum == INVALID || m_reachCheckPointOrder == INVALID; }
};

class SaveDataManager : public KuroEngine::DesignPattern::Singleton<SaveDataManager>
{
	friend class KuroEngine::DesignPattern::Singleton<SaveDataManager>;
	SaveDataManager();

	friend class SystemSetting;

	//�f�[�^�t�@�C���̃f�B���N�g��
	static const std::string DIR;
	//�f�[�^�t�@�C���̖��O
	static const std::string FILE_NAME;
	//�f�[�^�t�@�C���̑����p�X
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

	//�X�e�[�W�Ɋւ���Z�[�u�f�[�^�ǂݍ���
	bool LoadStageSaveData(int* arg_reachStageNum, int* arg_reachCheckPointOreder)
	{ 
		if (arg_reachStageNum)*arg_reachStageNum = m_saveData.m_reachStageNum;
		if (arg_reachCheckPointOreder)*arg_reachCheckPointOreder = m_saveData.m_reachCheckPointOrder;
		return m_saveData.m_reachStageNum != GameSaveData::INVALID && m_saveData.m_reachCheckPointOrder != GameSaveData::INVALID;
	}

	//���ɓ��肵���X�^�[�R�C����
	bool IsGetStarCoin(int arg_stageNum, int arg_id)const
	{
		for (auto& coin : m_saveData.m_getStarCoinInfoArray)
		{
			if (coin.m_stageNum == arg_stageNum && coin.m_id == arg_id)return true;
		}

		return false;
	}

	//�T�E���h�{�����[���Ɋւ���f�[�^
	const SoundVolumeData& GetSoundVolumeData() { return m_saveData.m_soundVol; }

	//������@�Ɋւ���f�[�^
	const OperationData& GetOperationData() { return m_saveData.m_operationSetting; }

	//�f�[�^�폜
	void ResetSaveData()
	{
		m_saveData.m_reachStageNum = GameSaveData::INVALID;
		m_saveData.m_reachCheckPointOrder = GameSaveData::INVALID;
		m_saveData.m_getStarCoinInfoArray.clear();
	}
};

