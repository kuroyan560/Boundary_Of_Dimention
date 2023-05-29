#pragma once
#include<memory>
#include<array>
#include<vector>
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

	//�I���\�ȍ���
	struct SelectItem
	{
		ITEM_STATUS m_status = DEFAULT;
		std::array<std::shared_ptr<KuroEngine::TextureBuffer>, ITEM_STATUS_NUM>m_tex;
		KuroEngine::Vec2<float>m_centerPos;

		void LoadTex(std::string arg_path);
	};

	//���C�����j���[
	class MainMenuGroup
	{
		enum ITEM { ITEM_SOUND, ITEM_OPE, ITEM_BACK, ITEM_NUM }m_nowItem;
		//���o��
		std::shared_ptr<KuroEngine::TextureBuffer>m_headTex;
		//����
		std::array<SelectItem, ITEM_NUM>m_items;

	public:
		MainMenuGroup();
		void Init() { m_nowItem = (ITEM)0; }
		void Update(SystemSetting* arg_parent);
		void Draw();
	}m_mainMenu;

	//���ʐݒ胁�j���[
	class SoundMenuGroup
	{
		enum ITEM { ITEM_MASTER, ITEM_SE, ITEM_BGM, ITEM_BACK, ITEM_NUM }m_nowItem;

		//���o��
		std::shared_ptr<KuroEngine::TextureBuffer>m_headTex;

		//����
		std::array<SelectItem, ITEM_NUM>m_items;
		//�Q�[�W�̐A���i�摜�p�^�[���̃C���f�b�N�X�j
		std::array<std::vector<int>, ITEM_NUM>m_gageParam;

		static int GetPattern()
		{
			static int PATTERN = -1;
			PATTERN++;
			if (GAGE_PATTERN_NUM <= PATTERN)PATTERN = 0;
			return PATTERN;
		}

		//�n�ʃQ�[�W
		std::shared_ptr<KuroEngine::TextureBuffer>m_groundLineTex;
		//�Q�[�W�̃p�^�[����
		static const int GAGE_PATTERN_NUM = 3;
		std::array<std::shared_ptr<KuroEngine::TextureBuffer>, GAGE_PATTERN_NUM>m_gagePatternTex;

		//�p�����[�^�̒i�K��
		static const int VOL_STAGE_NUM = 10;
		static const float VOL_CHANGE;

	public:
		SoundMenuGroup();
		void Init();
		void Update(SystemSetting* arg_parent);
		void Draw();
	}m_soundMenu;

	//����ݒ胁�j���[
	struct OpeMenuGroup
	{
		enum ITEM { ITEM_MIRROR, ITEM_SENSITIVITY,ITEM_BACK, ITEM_NUM }m_nowItem;

		//���o��
		std::shared_ptr<KuroEngine::TextureBuffer>m_headTex;

		//����
		std::array<SelectItem, ITEM_NUM>m_items;

		//���
		std::shared_ptr<KuroEngine::TextureBuffer>m_arrowTex;
		//�J�����̃~���[�̃`�F�b�N�{�b�N�X
		std::shared_ptr<KuroEngine::TextureBuffer>m_camMirrorCheckBoxTex;
		//�`�F�b�N�}�[�N
		std::shared_ptr<KuroEngine::TextureBuffer>m_checkMarkTex;

		//�n�ʃQ�[�W
		std::shared_ptr<KuroEngine::TextureBuffer>m_groundLineTex;
		//�n�ʃQ�[�W�̃C���f�b�N�X�̗t
		std::shared_ptr<KuroEngine::TextureBuffer>m_leafTex;

		//�J�������x�̒i�K��
		static const int CAM_SENSITIVITY_PLUS_MINUS_STAGE_NUM = 5;
		//�J�������x�̊
		static const float CAM_SENSITIVITY_BASE;
		//�J�������x�ő�I�t�Z�b�g
		static const float CAM_SENSITIVITY_OFFSET_MAX;
		//�J�������x�I�t�Z�b�g
		static const float CAM_SENSITIVITY_CHANGE;

		//�J�������x�̃Q�[�W�ʒu
		int m_camSensitivityParam = 0;

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