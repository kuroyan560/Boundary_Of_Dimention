#pragma once
#include"Common/Singleton.h"
#include<vector>
#include<array>
#include<string>
#include"ForUser/Debugger.h"

class SoundConfig : public KuroEngine::DesignPattern::Singleton<SoundConfig>, public KuroEngine::Debugger
{
public:
	//SE（再生機能のみ）
	enum SE
	{
		SE_SELECT,	//選択
		SE_DONE,	//決定
		SE_CANCEL,	//キャンセル
		SE_SURFACE_JUMP,	//面移動
		SE_LEVER_ON,	//レバー起動
		SE_LEVER_OFF,	//レバー解除
		SE_MOVE_SCAFFOLD_START,	//移動床起動
		SE_MOVE_SCAFFOLD_STOP,	//移動床停止
		SE_CAM_MODE_CHANGE,	//カメラモード切り替え
		SE_ZIP_LINE_GET_ON,	//ジップラインに乗る
		SE_ZIP_LINE_GET_OFF,	//ジップラインから降りる
		SE_GRASS,	//草の音
		SE_BOOT,	//起動
		SE_SHUT_DOWN,	//シャットダウン
		SE_PLAYER_DAMAGE,	//プレイヤーの被ダメージ
		SE_NUM
	};
	//ジングル（現在再生中かの確認なども行う、ループ再生しない）
	enum JINGLE
	{
		JINGLE_STAGE_CLEAR,	//ステージクリア
		JINGLE_NUM
	};
	//BGM（ループ再生あり）
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
		//読み込んだ音声のハンドル配列
		std::vector<int>m_sounds;
		//最後に再生した音声のインデックス
		int m_latestIdx = 0;
		//複数ある場合の再生順
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

	//各音声の個別のボリューム
	std::array<float, SE_NUM>m_seEachVol;
	std::array<float, JINGLE_NUM>m_jingleEachVol;
	std::array<float, BGM_NUM>m_bgmEachVol;

	//現在再生中のBGMのハンドル
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

