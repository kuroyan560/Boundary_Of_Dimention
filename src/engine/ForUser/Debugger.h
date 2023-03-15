#pragma once
#include<string>
#include<vector>
#include<list>
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

	protected:
		enum struct PARAM_TYPE
		{
			INT, INT_VEC2, INT_VEC3, INT_VEC4,
			FLOAT, FLOAT_VEC2, FLOAT_VEC3, FLOAT_VEC4,
			BOOL,
			STRING,
			NONE
		};

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

		struct CustomParameter
		{
			std::vector<std::string>m_key;
			PARAM_TYPE m_type = PARAM_TYPE::NONE;
			void* m_dataPtr = nullptr;

			CustomParameter(std::initializer_list<std::string>arg_key, PARAM_TYPE arg_type, void* arg_destPtr)
				:m_key(arg_key), m_type(arg_type), m_dataPtr(arg_destPtr) {}
		};

		//�p�����[�^���O�iexe���Ă��c���p�����[�^�j
		std::list<CustomParameter>m_customParamList;

	protected:
		//imgui�E�B���h�E��
		std::string m_title;
		Debugger(std::string arg_title, bool arg_active = false, ImGuiWindowFlags arg_imguiWinFlags = 0)
			:m_title(arg_title), m_active(arg_active), m_id(s_id++), m_imguiWinFlags(arg_imguiWinFlags) {}

		//imgui�̍��� Begin ~ End �ԂɌĂяo������
		virtual void OnImguiItems() = 0;

		void AddCustomParameter(std::initializer_list<std::string>arg_key, PARAM_TYPE arg_type, void* arg_destPtr)
		{
			m_customParamList.emplace_back(arg_key, arg_type, arg_destPtr);
		}
	};
}