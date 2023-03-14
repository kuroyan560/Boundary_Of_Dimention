#include "Debugger.h"
#include"FrameWork/Fps.h"
#include"KuroEngine.h"

std::vector<KuroEngine::Debugger*>KuroEngine::Debugger::s_debuggerArray;
int KuroEngine::Debugger::s_id = 0;
const std::string KuroEngine::Debugger::s_jsonFileDir = "resource/engine/";
const std::string KuroEngine::Debugger::s_jsonName = "KuroEngineDebugger";
const std::string KuroEngine::Debugger::s_jsonExt = ".json";
KuroEngine::JsonData KuroEngine::Debugger::s_parameterLog;

void KuroEngine::Debugger::Draw()
{
	ImGui::Begin("DebuggerMgr");
	Fps::Instance()->OnImguiItems();
	ImGui::Separator();
	for (auto& debugger : s_debuggerArray)
	{
		ImGui::Checkbox(debugger->m_title.c_str(), &debugger->m_active);
	}
	ImGui::End();

	for (auto& debugger : s_debuggerArray)
	{
		if (!debugger->m_active)continue;
		ImGui::Begin(debugger->m_title.c_str(), nullptr, debugger->m_imguiWinFlags);
		debugger->OnImguiItems();
		ImGui::End();
	}
}

void KuroEngine::Debugger::LoadParameterLog()
{
	using namespace KuroEngine;

	//ログがない
	if (!s_parameterLog.m_jsonData.contains(m_title))return;

	nlohmann::json& json = s_parameterLog.m_jsonData[m_title];

	for (auto& param : m_parameterLogArray)
	{
		//このパラメータのログがない
		if (!json.contains(param.m_key))continue;

		//キーからjsonオブジェクト取得
		auto& obj = json[param.m_key];

		switch (param.m_type)
		{
		case LOG_TYPE::INT:
		{
			auto data = obj.get<int>();
			memcpy(param.m_dataPtr, &data, sizeof(int));
			break;
		}
		case LOG_TYPE::INT_VEC2:
		{
			Vec2<int>data = { (int)obj[0],(int)obj[1] };
			memcpy(param.m_dataPtr, &data, sizeof(int) * 2);
			break;
		}
		case LOG_TYPE::INT_VEC3:
		{
			Vec3<int>data = { (int)obj[0],(int)obj[1],(int)obj[2] };
			memcpy(param.m_dataPtr, &data, sizeof(int) * 3);
			break;
		}
		case LOG_TYPE::INT_VEC4:
		{
			Vec4<int>data = { (int)obj[0],(int)obj[1],(int)obj[2],(int)obj[3] };
			memcpy(param.m_dataPtr, &data, sizeof(int) * 4);
			break;
		}

		case LOG_TYPE::FLOAT:
		{
			auto data = obj.get<float>();
			memcpy(param.m_dataPtr, &data, sizeof(float));
			break;
		}
		case LOG_TYPE::FLOAT_VEC2:
		{
			Vec2<float>data = { (float)obj[0],(float)obj[1] };
			memcpy(param.m_dataPtr, &data, sizeof(float) * 2);
			break;
		}
		case LOG_TYPE::FLOAT_VEC3:
		{
			Vec3<float>data = { (float)obj[0],(float)obj[1],(float)obj[2] };
			memcpy(param.m_dataPtr, &data, sizeof(float) * 3);
			break;
		}
		case LOG_TYPE::FLOAT_VEC4:
		{
			Vec4<float>data = { (float)obj[0],(float)obj[1],(float)obj[2],(float)obj[3] };
			memcpy(param.m_dataPtr, &data, sizeof(float) * 4);
			break;
		}

		case LOG_TYPE::BOOL:
		{
			bool data = obj.get<bool>();
			memcpy(param.m_dataPtr, &data, sizeof(bool));
			break;
		}

		case LOG_TYPE::STRING:
		{
			std::string data = obj.get<std::string>();
			memcpy(param.m_dataPtr, &data, sizeof(data));
			break;
		}
		default:
			AppearMessageBox("Debugger's parameter log ERROR", "\"" + param.m_key + "\" 's parameter's type is none");
			break;
		}
	}
}

void KuroEngine::Debugger::WriteParameterLog()
{
	using namespace KuroEngine;

	if (m_parameterLogArray.empty())return;

	nlohmann::json& json = s_parameterLog.m_jsonData[m_title];

	for (auto& param : m_parameterLogArray)
	{
		switch (param.m_type)
		{
		case LOG_TYPE::INT:
			json[param.m_key] = *(int*)param.m_dataPtr;
			break;
		case LOG_TYPE::INT_VEC2:
			json[param.m_key] = { ((int*)param.m_dataPtr)[0],((int*)param.m_dataPtr)[1] };
			break;
		case LOG_TYPE::INT_VEC3:
			json[param.m_key] = { ((int*)param.m_dataPtr)[0],((int*)param.m_dataPtr)[1],((int*)param.m_dataPtr)[2] };
			break;
		case LOG_TYPE::INT_VEC4:
			json[param.m_key] = { ((int*)param.m_dataPtr)[0],((int*)param.m_dataPtr)[1],((int*)param.m_dataPtr)[2],((int*)param.m_dataPtr)[3] };
			break;

		case LOG_TYPE::FLOAT:
			json[param.m_key] = *(float*)param.m_dataPtr;
			break;
		case LOG_TYPE::FLOAT_VEC2:
			json[param.m_key] = { ((float*)param.m_dataPtr)[0],((float*)param.m_dataPtr)[1] };
			break;
		case LOG_TYPE::FLOAT_VEC3:
			json[param.m_key] = { ((float*)param.m_dataPtr)[0],((float*)param.m_dataPtr)[1],((float*)param.m_dataPtr)[2] };
			break;
		case LOG_TYPE::FLOAT_VEC4:
			json[param.m_key] = { ((float*)param.m_dataPtr)[0],((float*)param.m_dataPtr)[1],((float*)param.m_dataPtr)[2],((float*)param.m_dataPtr)[3] };
			break;

		case LOG_TYPE::BOOL:
			json[param.m_key] = *(bool*)param.m_dataPtr;
			break;

		case LOG_TYPE::STRING:
			json[param.m_key] = *(std::string*)param.m_dataPtr;
			break;
		default:
			AppearMessageBox("Debugger's parameter log ERROR", "\"" + param.m_key + "\" 's parameter's type is none");
			break;
		}
	}
}