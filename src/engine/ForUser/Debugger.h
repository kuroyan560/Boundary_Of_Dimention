#pragma once
#include<string>
#include<vector>
#include"FrameWork/ImguiApp.h"
#include"JsonData.h"

namespace KuroEngine
{
	//imguiデバッグのインターフェース
	class Debugger
	{
	private:
		//割り当てる識別番号
		static int s_id;
		//起動中のデバッガ配列
		static std::vector<Debugger*>s_debuggerArray;

		//※exe閉じてもパラメータを残すためのもの
		//パラメータログのファイルディレクトリ
		static const std::string s_jsonFileDir;
		//パラメータログのファイル名
		static const std::string s_jsonName;
		//パラメータログのファイル拡張子
		static const std::string s_jsonExt;
		//パラメータログ
		static JsonData s_parameterLog;

	public:
		virtual ~Debugger() {  }
		//デバッグ機構表示
		static void Draw();
		//デバッガ登録
		static void Register(std::vector<Debugger*>arg_debuggerArray)
		{
			s_debuggerArray = arg_debuggerArray;
			for (auto& debugger : s_debuggerArray)debugger->LoadParameterLog();
		}
		//デバッガ登録解除
		static void ClearRegister() 
		{
			for (auto& debugger : s_debuggerArray)debugger->WriteParameterLog();
			s_debuggerArray.clear(); 
		}

		//パラメータログをファイル読み込み
		static void ImportParameterLog()
		{
			s_parameterLog.Import(s_jsonFileDir, s_jsonName + s_jsonExt);
		}
		//パラメータログをファイル出力
		static void ExportParameterLog()
		{
			ClearRegister();
			s_parameterLog.Export(s_jsonFileDir, s_jsonName, s_jsonExt, false);
		}

	private:
		//識別番号
		int m_id;
		//アクティブ状態
		bool m_active = false;
		//imguiWindowフラグ
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

		//パラメータログ（exe閉じても残すパラメータ）
		std::vector<ParameterLogInfo>m_parameterLogArray;

		//imguiウィンドウ名
		std::string m_title;
		Debugger(std::string arg_title, bool arg_active = false, ImGuiWindowFlags arg_imguiWinFlags = 0)
			:m_title(arg_title), m_active(arg_active), m_id(s_id++), m_imguiWinFlags(arg_imguiWinFlags) {}

		//imguiの項目 Begin ~ End 間に呼び出す処理
		virtual void OnImguiItems() = 0;
	};
}