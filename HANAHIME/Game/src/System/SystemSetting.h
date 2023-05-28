#pragma once
#include<memory>
#include<array>
#include<string>
#include"Common/Vec.h"

namespace KuroEngine
{
	class TextureBuffer;
};

class SystemSetting
{
	static std::string TEX_DIR;

	enum MODE { MODE_MAIN_MENU, MODE_SOUND, MODE_OPERATION }m_mode = MODE_MAIN_MENU;
	bool m_isActive = false;
	
	enum ITEM_STATUS { DEFAULT, SELECTED, ITEM_STATUS_NUM };

	//選択可能な項目
	struct SelectItem
	{
		ITEM_STATUS m_status = DEFAULT;
		std::array<std::shared_ptr<KuroEngine::TextureBuffer>, ITEM_STATUS_NUM>m_tex;
		KuroEngine::Vec2<float>m_centerPos;

		void LoadTex(std::string arg_path);
	};

	//メインメニュー
	class MainMenuGroup
	{
		enum ITEM { ITEM_SOUND, ITEM_OPE, ITEM_BACK, ITEM_NUM }m_nowItem;
		//見出し
		std::shared_ptr<KuroEngine::TextureBuffer>m_headTex;
		//項目
		std::array<SelectItem, ITEM_NUM>m_items;

	public:
		MainMenuGroup();
		void Init() { m_nowItem = (ITEM)0; }
		void Update(SystemSetting* arg_parent);
		void Draw();
	}m_mainMenu;

	//音量設定メニュー
	class SoundMenuGroup
	{
		enum ITEM { ITEM_MASTER, ITEM_SE, ITEM_BGM, ITEM_BACK, ITEM_NUM }m_nowItem;

		//見出し
		std::shared_ptr<KuroEngine::TextureBuffer>m_headTex;
		//項目
		std::array<SelectItem, ITEM_NUM>m_items;
		//地面ゲージ
		std::shared_ptr<KuroEngine::TextureBuffer>m_groundLineTex;
		//ゲージのパターン数
		static const int GAGE_PATTERN_NUM = 3;
		std::array<std::shared_ptr<KuroEngine::TextureBuffer>, GAGE_PATTERN_NUM>m_gagePatternTex;

	public:
		SoundMenuGroup();
		void Init() { m_nowItem = ITEM_MASTER; }
		void Update(SystemSetting* arg_parent);
		void Draw();
	}m_soundMenu;

	//操作設定メニュー
	struct OpeMenuGroup
	{
		enum ITEM { ITEM_MIRROR, ITEM_SENSITIVITY,ITEM_BACK, ITEM_NUM }m_nowItem;

		//見出し
		std::shared_ptr<KuroEngine::TextureBuffer>m_headTex;

		//項目
		std::array<SelectItem, ITEM_NUM>m_items;

		//矢印
		std::shared_ptr<KuroEngine::TextureBuffer>m_arrowTex;
		//カメラのミラーのチェックボックス
		std::shared_ptr<KuroEngine::TextureBuffer>m_camMirrorCheckBoxTex;
		//チェックマーク
		std::shared_ptr<KuroEngine::TextureBuffer>m_checkMarkTex;

		//地面ゲージ
		std::shared_ptr<KuroEngine::TextureBuffer>m_groundLineTex;
		//地面ゲージのインデックスの葉
		std::shared_ptr<KuroEngine::TextureBuffer>m_leafTex;

	public:
		OpeMenuGroup();
		void Init() { m_nowItem = ITEM_MIRROR; }
		void Update(SystemSetting* arg_parent);
		void Draw();
	}m_opeMenu;

public:
	void Update();
	void Draw();

	void Activate();
	
	const bool& IsActive()const { return m_isActive; }
};