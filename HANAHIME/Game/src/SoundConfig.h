#pragma once
#include"Common/Singleton.h"
#include<vector>
#include<array>
#include<string>
#include"ForUser/Debugger.h"

class SoundConfig : public KuroEngine::DesignPattern::Singleton<SoundConfig>, public KuroEngine::Debugger
{
public:
	//SE�i�Đ��@�\�̂݁j
	enum SE
	{
		SE_SELECT,	//�I��
		SE_DONE,	//����
		SE_CANCEL,	//�L�����Z��
		SE_SURFACE_JUMP,	//�ʈړ�
		SE_LEVER_ON,	//���o�[�N��
		SE_LEVER_OFF,	//���o�[����
		SE_MOVE_SCAFFOLD_START,	//�ړ����N��
		SE_MOVE_SCAFFOLD_STOP,	//�ړ�����~
		SE_CAM_MODE_CHANGE,	//�J�������[�h�؂�ւ�
		SE_ZIP_LINE_GET_ON,	//�W�b�v���C���ɏ��
		SE_ZIP_LINE_GET_OFF,	//�W�b�v���C������~���
		SE_GRASS,	//���̉�
		SE_BOOT,	//�N��
		SE_SHUT_DOWN,	//�V���b�g�_�E��
		SE_PLAYER_DAMAGE,	//�v���C���[�̔�_���[�W
		SE_NUM
	};
	//�W���O���i���ݍĐ������̊m�F�Ȃǂ��s���A���[�v�Đ����Ȃ��j
	enum JINGLE
	{
		JINGLE_STAGE_CLEAR,	//�X�e�[�W�N���A
		JINGLE_NUM
	};
	//BGM�i���[�v�Đ�����j
	enum BGM
	{
		BGM_TITLE,
		BGM_IN_GAME,
		BGM_NUM
	};

private:
	friend class KuroEngine::DesignPattern::Singleton<SoundConfig>;
	SoundConfig();
	std::vector<int>LoadSoundArray(std::string arg_dir, std::string arg_name, float arg_volume = 1.0f);

	class SoundSE
	{
	public:
		enum ORDER_TYPE { IN_ORDER, RANDOM };
	private:
		//�ǂݍ��񂾉����̃n���h���z��
		std::vector<int>m_sounds;
		//�Ō�ɍĐ����������̃C���f�b�N�X
		int m_latestIdx = 0;
		//��������ꍇ�̍Đ���
		ORDER_TYPE m_order = IN_ORDER;
		
		int GetPlaySoundHandle();

	public:
		bool m_invalid = true;
		SoundSE() {}

		void Load(int arg_sound)
		{
			m_sounds = { arg_sound };
			m_invalid = false;
		}
		void Load(std::vector<int>arg_sounds, ORDER_TYPE arg_order)
		{
			m_sounds = arg_sounds;
			m_order = arg_order;
			m_invalid = false;
		}

		void Init()
		{
			m_latestIdx = 0;
		}
		void Play(int arg_delay = -1, int arg_soundIdx = -1);
		void SetVolume(float arg_vol);
	};

	static const int INVALID_SOUND = -1;
	std::array<SoundSE, SE_NUM>m_seTable;
	std::array<int, JINGLE_NUM>m_jingleTable = { INVALID_SOUND };
	std::array<int, BGM_NUM>m_bgmTable = { INVALID_SOUND };

	//�e�����̌ʂ̃{�����[��
	std::array<float, SE_NUM>m_seEachVol;
	std::array<float, JINGLE_NUM>m_jingleEachVol;
	std::array<float, BGM_NUM>m_bgmEachVol;

	//���ݍĐ�����BGM�̃n���h��
	int m_nowPlayBGMHandle = INVALID_SOUND;

	void SetIndividualVolume();
	void OnImguiItems() { if (CustomParamDirty())SetIndividualVolume(); }

public:
	void Init();

	void Play(SE arg_se, int arg_delay = -1, int arg_soundIdx = -1);
	void Play(JINGLE arg_jingle);
	void Play(BGM arg_bgm);

	bool NowPlay(JINGLE arg_jingle);
};

