#pragma once
#include<string>
#include<vector>
#include"FrameWork/ImguiApp.h"
#include"JsonData.h"

namespace KuroEngine
{
	//imgui�f�o�b�O�̃C���^�[�t�F�[�X
	class Debugger
	{
	private:
		//���蓖�Ă鎯�ʔԍ�
		static int s_id;
		//�N�����̃f�o�b�K�z��
		static std::vector<Debugger*>s_debuggerArray;

		//��exe���Ă��p�����[�^���c�����߂̂���
		//�p�����[�^���O�̃t�@�C���f�B���N�g��
		static const std::string s_jsonFileDir;
		//�p�����[�^���O�̃t�@�C����
		static const std::string s_jsonName;
		//�p�����[�^���O�̃t�@�C���g���q
		static const std::string s_jsonExt;
		//�p�����[�^���O
		static JsonData s_parameterLog;

	public:
		virtual ~Debugger() {  }
		//�f�o�b�O�@�\�\��
		static void Draw();
		//�f�o�b�K�o�^
		static void Register(std::vector<Debugger*>arg_debuggerArray)
		{
			s_debuggerArray = arg_debuggerArray;
			for (auto& debugger : s_debuggerArray)debugger->LoadParameterLog();
		}
		//�f�o�b�K�o�^����
		static void ClearRegister() 
		{
			for (auto& debugger : s_debuggerArray)debugger->WriteParameterLog();
			s_debuggerArray.clear(); 
		}

		//�p�����[�^���O���t�@�C���ǂݍ���
		static void ImportParameterLog()
		{
			s_parameterLog.Import(s_jsonFileDir, s_jsonName + s_jsonExt);
		}
		//�p�����[�^���O���t�@�C���o��
		static void ExportParameterLog()
		{
			ClearRegister();
			s_parameterLog.Export(s_jsonFileDir, s_jsonName, s_jsonExt, false);
		}

	private:
		//���ʔԍ�
		int m_id;
		//�A�N�e�B�u���
		bool m_active = false;
		//imguiWindow�t���O
		ImGuiWindowFlags m_imguiWinFlags;

		void LoadParameterLog();
		void WriteParameterLog();

	protected:
		enum struct LOG_TYPE
		{
			INT, INT_VEC2, INT_VEC3, INT_VEC4,
			FLOAT, FLOAT_VEC2, FLOAT_VEC3, FLOAT_VEC4,
			BOOL,
			STRING,
			NONE
		};
		struct ParameterLogInfo
		{
			std::string m_key;
			LOG_TYPE m_type = LOG_TYPE::NONE;
			void* m_dataPtr = nullptr;
		};

		//�p�����[�^���O�iexe���Ă��c���p�����[�^�j
		std::vector<ParameterLogInfo>m_parameterLogArray;

		//imgui�E�B���h�E��
		std::string m_title;
		Debugger(std::string arg_title, bool arg_active = false, ImGuiWindowFlags arg_imguiWinFlags = 0)
			:m_title(arg_title), m_active(arg_active), m_id(s_id++), m_imguiWinFlags(arg_imguiWinFlags) {}

		//imgui�̍��� Begin ~ End �ԂɌĂяo������
		virtual void OnImguiItems() = 0;
	};
}