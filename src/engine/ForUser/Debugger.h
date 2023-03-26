#pragma once
#include<string>
#include<vector>
#include<list>
#include"FrameWork/ImguiApp.h"
#include"JsonData.h"
#include<map>
#include"Common/Color.h"

namespace KuroEngine
{
	//imguiデバッグのインターフェース
	class Debugger
	{
	private:
		//割り当てる識別番号
		static int s_id;
		//起動中のデバッガ配列
		static std::vector<Debugger*>s_registeredDebuggerArray;
		//生成したデバッガ配列
		static std::vector<Debugger*>s_debuggerArray;

		//※exe閉じてもパラメータを残すためのもの
		//パラメータログのファイルディレクトリ
		static const std::string s_jsonFileDir;
		//パラメータログのファイル名
		static const std::string s_jsonName;
		//パラメータログのファイル拡張子
		static const std::string s_jsonExt;

		//デバッガの設定項目キー
		static const std::string s_settingKey;
		//デバッガのイメージカラー項目キー
		static const std::string s_settingColKey;
		//デバッガのテキストカラーーフラグ項目キー
		static const std::string s_settingTextColKey;
		//デバッガのアクティブ状態項目キー
		static const std::string s_settingActiveKey;
		//デバッガのカスタムパラメータのアクティブ状態項目キー
		static const std::string s_settingCustomParamKey;

		//パラメータログ
		static JsonData s_parameterLog;

	protected:
		//カスタムパラメータの型
		enum struct PARAM_TYPE
		{
			INT, INT_VEC2, INT_VEC3, INT_VEC4,
			FLOAT, FLOAT_VEC2, FLOAT_VEC3, FLOAT_VEC4,
			BOOL,
			STRING,
			COLOR,
			NONE
		};

	public:
		virtual ~Debugger() { WriteParameterLog(); }
		//デバッグ機構表示
		static void Draw();
		//デバッガ登録
		static void Register(std::vector<Debugger*>arg_debuggerArray)
		{
			s_registeredDebuggerArray = arg_debuggerArray;
		}
		//デバッガ登録解除
		static void ClearRegister() 
		{
			s_registeredDebuggerArray.clear(); 
		}

		//パラメータログをファイル読み込み
		static void ImportParameterLog()
		{
			s_parameterLog.Import(s_jsonFileDir, s_jsonName + s_jsonExt);
		}
		//パラメータログをファイル出力
		static void ExportParameterLog()
		{
			for (auto debugger : s_debuggerArray)debugger->WriteParameterLog();
			s_parameterLog.Export(s_jsonFileDir, s_jsonName, s_jsonExt, false);
		}

	private:
		//識別番号
		int m_id;
		//アクティブ状態
		bool m_active = false;
		//imguiWindowフラグ
		ImGuiWindowFlags m_imguiWinFlags;

		//カスタムパラメータ（exe閉じても調整した値を残す）
		struct CustomParameter
		{
			//名前
			std::string m_label;
			//jsonに記録する際のキー配列（順列で階層を下りていく）
			std::vector<std::string>m_key;
			//型
			PARAM_TYPE m_type = PARAM_TYPE::NONE;
			//値を同期する変数のポインタ
			void* m_dataPtr = nullptr;
			//上限下限をつけるか
			bool m_isMinMax = false;
			//下限
			float m_min = 0.0f;
			//上限
			float m_max = 0.0f;

			CustomParameter(std::string arg_label, std::initializer_list<std::string>arg_key, PARAM_TYPE arg_type, void* arg_destPtr,
				bool arg_isMinMax, float arg_min, float arg_max)
				:m_label(arg_label), m_key(arg_key), m_type(arg_type), m_dataPtr(arg_destPtr), m_isMinMax(arg_isMinMax), m_min(arg_min), m_max(arg_max) {}
		};

		//パラメータログ（exe閉じても残すパラメータ）
		std::list<CustomParameter>m_customParamList;
		//カスタムパラメータをimgui上で表示する際のTree組分け
		std::map<std::string, std::vector<CustomParameter*>>m_customParamGroup;


		//カスタムパラメータをimgui上でいじる関数
		void CustomParameterOnImgui();
		//カスタムパラメータ調整ウィンドウのアクティブ状態
		bool m_customParamActive = true;
		//（※数値に限り）値をドラッグする際の変化量
		float m_customParamDragSpeed = 0.05f;
		//カスタムパラメータが変化したか
		bool m_customParamDirty = false;

	protected:
		//imguiウィンドウ名
		std::string m_title;
		//色
		ImVec4 m_debuggerColor = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
		//文字色白黒
		bool m_textColor = false;

		Debugger(std::string arg_title, bool arg_active = false, bool arg_customParamActive = false, ImGuiWindowFlags arg_imguiWinFlags = 0)
			:m_title(arg_title), m_active(arg_active), m_customParamActive(arg_customParamActive), m_id(s_id++), m_imguiWinFlags(arg_imguiWinFlags)
		{
			s_debuggerArray.emplace_back(this);
		}

		/// <summary>
		/// カスタムパラメータ以外のImgui処理を自由に定義（Begin ~ End の間で呼ばれる）
		/// </summary>
		virtual void OnImguiItems() {};

		/// <summary>
		/// カスタムパラメータの追加
		/// </summary>
		/// <param name="arg_label">カスタムパラメータの名前</param>
		/// <param name="arg_key">json上で記録する際のキー配列（順列で階層を下りていく）</param>
		/// <param name="arg_type">パラメータの型</param>
		/// <param name="arg_destPtr">カスタムパラメータの同期先</param>
		/// <param name="arg_imguiTreeName">imgui上でのTree組分け先グループ名</param>
		/// <param name="arg_isMinMax">（※数値にかぎり）上限下限をつけるか</param>
		/// <param name="arg_min">下限</param>
		/// <param name="arg_max">上限</param>
		void AddCustomParameter(std::string arg_label, std::initializer_list<std::string>arg_key, PARAM_TYPE arg_type, void* arg_destPtr, std::string arg_imguiTreeName,
			bool arg_isMinMax = false, float arg_min = 0.0f, float arg_max = 0.0f)
		{
			m_customParamList.emplace_back(arg_label, arg_key, arg_type, arg_destPtr, arg_isMinMax, arg_min, arg_max);
			m_customParamGroup[arg_imguiTreeName].push_back(&m_customParamList.back());
		}

		//パラメータログからカスタムパラメータ読込
		void LoadParameterLog();
		//パラメータログにカスタムパラメータを記録
		void WriteParameterLog();

		//カスタムパラメータが変化したか
		const bool& CustomParamDirty()const { return m_customParamDirty; }
	};
}