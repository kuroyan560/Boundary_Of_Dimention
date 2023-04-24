#include "SoundConfig.h"
#include"FrameWork/AudioApp.h"
#include"KuroEngine.h"

std::vector<int> SoundConfig::LoadSoundArray(std::string arg_dir, std::string arg_name, float arg_volume)
{
	using namespace KuroEngine;

	std::vector<int>result;
	int soundIdx = 0;
	while (ExistFile(arg_dir + arg_name + "_" + std::to_string(soundIdx) + ".wav"))
	{
		result.emplace_back(AudioApp::Instance()->LoadAudio(arg_dir + arg_name + "_" + std::to_string(soundIdx++) + ".wav", arg_volume));
	}

	return result;
}

SoundConfig::SoundConfig()
{
	using namespace KuroEngine;

	auto audioApp = AudioApp::Instance();
	std::string seDir = "resource/user/sound/se/";

	//SE読み込み
	m_seTable[SE_SELECT].Load(audioApp->LoadAudio(seDir + "select.wav"));
	m_seTable[SE_DONE].Load(audioApp->LoadAudio(seDir + "done.wav"));
	m_seTable[SE_CANCEL].Load(audioApp->LoadAudio(seDir + "cancel.wav"));
	m_seTable[SE_SURFACE_JUMP].Load(LoadSoundArray(seDir, "surface_jump"), SoundSE::ORDER_TYPE::RANDOM);
	m_seTable[SE_LEVER_ON].Load(LoadSoundArray(seDir, "lever_on"), SoundSE::ORDER_TYPE::IN_ORDER);
	m_seTable[SE_LEVER_OFF].Load(audioApp->LoadAudio(seDir + "lever_off.wav"));
	m_seTable[SE_MOVE_SCAFFOLD_START].Load(audioApp->LoadAudio(seDir + "move_scaffold_start.wav"));
	m_seTable[SE_MOVE_SCAFFOLD_STOP].Load(audioApp->LoadAudio(seDir + "move_scaffold_stop.wav"));
	m_seTable[SE_CAM_MODE_CHANGE].Load(LoadSoundArray(seDir, "cam_mode"), SoundSE::ORDER_TYPE::IN_ORDER);
	m_seTable[SE_ZIP_LINE_GET_ON].Load(audioApp->LoadAudio(seDir + "ivy_zip_line_ride_on.wav"));
	m_seTable[SE_ZIP_LINE_GET_OFF].Load(audioApp->LoadAudio(seDir + "ivy_zip_line_ride_off.wav"));
	m_seTable[SE_GRASS].Load(audioApp->LoadAudio(seDir + "grass.wav"));
	m_seTable[SE_BOOT].Load(audioApp->LoadAudio(seDir + "boot.wav"));
	m_seTable[SE_SHUT_DOWN].Load(audioApp->LoadAudio(seDir + "shutdown.wav"));

	//ジングル読み込み
	std::string jingleDir = "resource/user/sound/jingle/";
	m_jingleTable[JINGLE_STAGE_CLEAR] = audioApp->LoadAudio(jingleDir + "clear.wav");

	//BGM読み込み
	std::string bgmDir = "resource/user/sound/bgm/";
	m_bgmTable[BGM_TITLE] = audioApp->LoadAudio(bgmDir + "title.wav");
	m_bgmTable[BGM_IN_GAME] = audioApp->LoadAudio(bgmDir + "in_game.wav");

	//全部読み込んだか確認
	for (int seIdx = 0; seIdx < SE_NUM; ++seIdx)
	{
		if (!m_seTable[seIdx].m_invalid)continue;
		AppearMessageBox("SoundConfig コンストラクタ", "読み込まれていないSEがあるよ。");
		exit(1);
	}
	for (int jingleIdx = 0; jingleIdx < JINGLE_NUM; ++jingleIdx)
	{
		if (m_jingleTable[jingleIdx] != INVALID_SOUND)continue;
		AppearMessageBox("SoundConfig コンストラクタ", "読み込まれていないジングルがあるよ。");
		exit(1);
	}
	for (int bgmIdx = 0; bgmIdx < BGM_NUM; ++bgmIdx)
	{
		if (m_bgmTable[bgmIdx] != INVALID_SOUND)continue;
		AppearMessageBox("SoundConfig コンストラクタ", "読み込まれていないBGMがあるよ。");
		exit(1);
	}
}

int SoundConfig::SoundSE::GetPlaySoundHandle()
{
	int result = m_sounds[m_latestIdx];

	//複数ある場合次に再生するサウンドのインデックス更新
	if (1 < m_sounds.size())
	{
		switch (m_order)
		{
			//順列
			case SoundConfig::SoundSE::IN_ORDER:
				if (static_cast<int>(m_sounds.size()) <= ++m_latestIdx)m_latestIdx = 0;
				break;
			//ランダム
			case SoundConfig::SoundSE::RANDOM:
				do {
					m_latestIdx = KuroEngine::GetRand(static_cast<int>(m_sounds.size() - 1));
				} while (m_latestIdx == result);
				break;
			default:
				break;
		}
	}

	return result;
}

void SoundConfig::SoundSE::Play(int arg_delay, int arg_soundIdx)
{
	int soundIdx = arg_soundIdx == -1 ? GetPlaySoundHandle() : m_sounds[arg_soundIdx];
	arg_delay == -1 ? KuroEngine::AudioApp::Instance()->PlayWave(soundIdx) : KuroEngine::AudioApp::Instance()->PlayWaveDelay(arg_delay);
}

void SoundConfig::Init()
{
	for (auto& se : m_seTable)se.Init();
}

void SoundConfig::Play(SE arg_se, int arg_delay, int arg_soundIdx)
{
	m_seTable[arg_se].Play(arg_delay, arg_soundIdx);
}

void SoundConfig::Play(JINGLE arg_jingle)
{
	KuroEngine::AudioApp::Instance()->PlayWave(m_jingleTable[arg_jingle]);
}

void SoundConfig::Play(BGM arg_bgm)
{
	//既に再生中ならそれを停止
	if (m_nowPlayBGMHandle != INVALID_SOUND)
	{
		KuroEngine::AudioApp::Instance()->StopWave(m_nowPlayBGMHandle);
	}
	KuroEngine::AudioApp::Instance()->PlayWave(m_bgmTable[arg_bgm], true);

	m_nowPlayBGMHandle = m_bgmTable[arg_bgm];
}

bool SoundConfig::NowPlay(JINGLE arg_jingle)
{
	return KuroEngine::AudioApp::Instance()->NowPlay(m_jingleTable[arg_jingle]);
}
