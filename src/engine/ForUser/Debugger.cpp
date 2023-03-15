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

	//ƒƒO‚ª‚È‚¢
	if (!s_parameterLog.m_jsonData.contains(m_title))return;

	for (auto& param : m_customParamList)
	{
		const auto* jsonObj = &s_parameterLog.m_jsonData[m_title];
		for (auto& key : param.m_key)
		{
			//ƒL[‚ª‘¶Ý‚µ‚È‚¢
			if (!jsonObj->contains(key))continue;

			//ŠK‘w‚ð‰º‚°‚é
			jsonObj = &jsonObj->at(key);
		}

		switch (param.m_type)
		{
		case PARAM_TYPE::INT:
		{
			auto data = jsonObj->get<int>();
			memcpy(param.m_dataPtr, &data, sizeof(int));
			break;
		}
		case PARAM_TYPE::INT_VEC2:
		{
			Vec2<int>data = { (int)jsonObj[0],(int)jsonObj[1] };
			memcpy(param.m_dataPtr, &data, sizeof(int) * 2);
			break;
		}
		case PARAM_TYPE::INT_VEC3:
		{
			Vec3<int>data = { (int)jsonObj[0],(int)jsonObj[1],(int)jsonObj[2] };
			memcpy(param.m_dataPtr, &data, sizeof(int) * 3);
			break;
		}
		case PARAM_TYPE::INT_VEC4:
		{
			Vec4<int>data = { (int)jsonObj[0],(int)jsonObj[1],(int)jsonObj[2],(int)jsonObj[3] };
			memcpy(param.m_dataPtr, &data, sizeof(int) * 4);
			break;
		}

		case PARAM_TYPE::FLOAT:
		{
			auto data = jsonObj->get<float>();
			memcpy(param.m_dataPtr, &data, sizeof(float));
			break;
		}
		case PARAM_TYPE::FLOAT_VEC2:
		{
			Vec2<float>data = { (float)jsonObj[0],(float)jsonObj[1] };
			memcpy(param.m_dataPtr, &data, sizeof(float) * 2);
			break;
		}
		case PARAM_TYPE::FLOAT_VEC3:
		{
			Vec3<float>data = { (float)jsonObj[0],(float)jsonObj[1],(float)jsonObj[2] };
			memcpy(param.m_dataPtr, &data, sizeof(float) * 3);
			break;
		}
		case PARAM_TYPE::FLOAT_VEC4:
		{
			Vec4<float>data = { (float)jsonObj[0],(float)jsonObj[1],(float)jsonObj[2],(float)jsonObj[3] };
			memcpy(param.m_dataPtr, &data, sizeof(float) * 4);
			break;
		}

		case PARAM_TYPE::BOOL:
		{
			bool data = jsonObj->get<bool>();
			memcpy(param.m_dataPtr, &data, sizeof(bool));
			break;
		}

		case PARAM_TYPE::STRING:
		{
			std::string data = jsonObj->get<std::string>();
			memcpy(param.m_dataPtr, &data, sizeof(data));
			break;
		}

		default:
			std::string name = m_title;
			for (const auto& key : param.m_key)name = name + " - " + key;
			AppearMessageBox("Debugger's parameter log ERROR", "\"" + name + "\" 's parameter's type is none");
			break;
		}
	}
}

void KuroEngine::Debugger::WriteParameterLog()
{
	using namespace KuroEngine;

	if (m_customParamList.empty())return;

	s_parameterLog.m_jsonData[m_title] = nlohmann::json::object();

	for (auto& param : m_customParamList)
	{
		nlohmann::json* parent = &s_parameterLog.m_jsonData[m_title];

		for (int keyIdx = 0; keyIdx < static_cast<int>(param.m_key.size() - 1); ++keyIdx)
		{
			if (!parent->contains(param.m_key[keyIdx]))
			{
				(*parent)[param.m_key[keyIdx]] = nlohmann::json::object();
			}
			parent = &(*parent)[param.m_key[keyIdx]];
		}

		nlohmann::json& paramObj = *parent;
		auto key = param.m_key.back();
		switch (param.m_type)
		{
		case PARAM_TYPE::INT:
			paramObj[key] = *(int*)param.m_dataPtr;
			break;
		case PARAM_TYPE::INT_VEC2:
			paramObj[key] = { ((int*)param.m_dataPtr)[0],((int*)param.m_dataPtr)[1] };
			break;
		case PARAM_TYPE::INT_VEC3:
			paramObj[key] = { ((int*)param.m_dataPtr)[0],((int*)param.m_dataPtr)[1],((int*)param.m_dataPtr)[2] };
			break;
		case PARAM_TYPE::INT_VEC4:
			paramObj[key] = { ((int*)param.m_dataPtr)[0],((int*)param.m_dataPtr)[1],((int*)param.m_dataPtr)[2],((int*)param.m_dataPtr)[3] };
			break;

		case PARAM_TYPE::FLOAT:
			paramObj[key] = *(float*)param.m_dataPtr;
			break;
		case PARAM_TYPE::FLOAT_VEC2:
			paramObj[key] = { ((float*)param.m_dataPtr)[0],((float*)param.m_dataPtr)[1] };
			break;
		case PARAM_TYPE::FLOAT_VEC3:
			paramObj[key] = { ((float*)param.m_dataPtr)[0],((float*)param.m_dataPtr)[1],((float*)param.m_dataPtr)[2] };
			break;
		case PARAM_TYPE::FLOAT_VEC4:
			paramObj[key] = { ((float*)param.m_dataPtr)[0],((float*)param.m_dataPtr)[1],((float*)param.m_dataPtr)[2],((float*)param.m_dataPtr)[3] };
			break;

		case PARAM_TYPE::BOOL:
			paramObj[key] = *(bool*)param.m_dataPtr;
			break;

		case PARAM_TYPE::STRING:
			paramObj[key] = *(std::string*)param.m_dataPtr;
			break;

		default:
			std::string name = m_title;
			for (const auto& key : param.m_key)name = name + " - " + key;
			AppearMessageBox("Debugger's parameter log ERROR", "\"" + name + "\" 's parameter's type is none");
			break;
		}
	}
}