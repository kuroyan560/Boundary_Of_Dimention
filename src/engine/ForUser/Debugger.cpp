#include "Debugger.h"
#include"FrameWork/Fps.h"
#include"KuroEngine.h"

std::vector<KuroEngine::Debugger*>KuroEngine::Debugger::s_debuggerArray;
std::vector<KuroEngine::Debugger*>KuroEngine::Debugger::s_registeredDebuggerArray;
int KuroEngine::Debugger::s_id = 0;
const std::string KuroEngine::Debugger::s_jsonFileDir = "resource/engine/";
const std::string KuroEngine::Debugger::s_jsonName = "KuroEngineDebugger";
const std::string KuroEngine::Debugger::s_jsonExt = ".json";

const std::string KuroEngine::Debugger::s_settingKey = "debuggerSetting";
const std::string KuroEngine::Debugger::s_settingColKey = "imageColor";
const std::string KuroEngine::Debugger::s_settingTextColKey = "textColorFlg";
const std::string KuroEngine::Debugger::s_settingActiveKey = "active";
const std::string KuroEngine::Debugger::s_settingCustomParamKey = "customParamActive";

KuroEngine::JsonData KuroEngine::Debugger::s_parameterLog;


void KuroEngine::Debugger::Draw()
{
	ImGui::Begin("DebuggerMgr", nullptr, ImGuiWindowFlags_NoDocking);
	Fps::Instance()->OnImguiItems();
	ImGui::Separator();
	for (auto& debugger : s_registeredDebuggerArray)
	{
		bool colorWidget = false;
		if (ImGui::ColorButton(("ColorWidget" + debugger->m_title).c_str(), debugger->m_debuggerColor))colorWidget = true;
		ImGui::SameLine();

		ImGui::Checkbox(debugger->m_title.c_str(), &debugger->m_active);

		//イメージカラー設定
		if (colorWidget)ImGui::OpenPopup(("PickColor" + debugger->m_title).c_str());
		if (ImGui::BeginPopup(("PickColor" + debugger->m_title).c_str()))
		{
			ImGui::ColorPicker4("ImageColor", (float*)&debugger->m_debuggerColor);
			ImGui::Checkbox("TextColor(White)", &debugger->m_textColor);
			ImGui::EndPopup();
		}
	}
	ImGui::End();

	for (auto& debugger : s_registeredDebuggerArray)
	{
		if (!debugger->m_active)continue;

		ImVec4 color = debugger->m_debuggerColor;
		color.w *= 0.8f;
		ImVec4 activeColor = debugger->m_debuggerColor;
		ImVec4 titleTextColor = debugger->m_textColor ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.2f, 0.2f, 0.2f, 1.0f);

		ImGui::PushStyleColor(ImGuiCol_Border, color);
		ImGui::PushStyleColor(ImGuiCol_TitleBg, color);
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, activeColor);
		ImGui::PushStyleColor(ImGuiCol_Text, titleTextColor);

		ImGui::Begin(std::string(debugger->m_title + " (Debugger)").c_str(), nullptr, debugger->m_imguiWinFlags);
		ImGui::PopStyleColor(1);

		//カスタムパラメータが存在
		if (!debugger->m_customParamList.empty())
		{
			//カスタムパラメータウィンドウのアクティブ状態
			ImGui::Checkbox("CustomParameter", &debugger->m_customParamActive);
			if (debugger->m_customParamActive)
			{
				//カスタムパラメータのimgui処理
				debugger->CustomParameterOnImgui();
			}

			ImGui::Separator();
		}

		//ユーザー定義のimgui処理
		debugger->OnImguiItems();

		ImGui::PopStyleColor(3);

		ImGui::End();
	}
}

void KuroEngine::Debugger::LoadParameterLog()
{
	using namespace KuroEngine;

	//ログがない
	if (!s_parameterLog.m_jsonData.contains(m_title))return;

	//デバッガの設定読み込み
	if (s_parameterLog.m_jsonData[m_title].contains(s_settingKey))
	{
		const auto& settings = s_parameterLog.m_jsonData[m_title][s_settingKey];
		//デバッガのイメージカラー読込
		if (settings.contains(s_settingColKey))
			m_debuggerColor = { (float)settings[s_settingColKey][0],(float)settings[s_settingColKey][1],(float)settings[s_settingColKey][2],(float)settings[s_settingColKey][3] };
		//テキストカラーフラグ読込
		if (settings.contains(s_settingTextColKey))
			m_textColor = settings[s_settingTextColKey].get<bool>();
		//デバッガのアクティブ状態女見込み
		if (settings.contains(s_settingActiveKey))
			m_active = settings[s_settingActiveKey].get<bool>();
		//デバッガのカスタムパラメータアクティブ状態
		if (settings.contains(s_settingCustomParamKey))
			m_customParamActive = settings[s_settingCustomParamKey].get<bool>();
	}

	for (auto& param : m_customParamList)
	{
		const auto* jsonObj = &s_parameterLog.m_jsonData[m_title];
		bool noKey = false;

		for (auto& key : param.m_key)
		{
			//キーが存在しない
			if (!jsonObj->contains(key))
			{
				noKey = true;
				break;
			}

			//階層を下げる
			jsonObj = &jsonObj->at(key);
		}

		if (noKey)continue;

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
			Vec2<int>data = { (int)jsonObj->at(0),(int)jsonObj->at(1) };
			memcpy(param.m_dataPtr, &data, sizeof(int) * 2);
			break;
		}
		case PARAM_TYPE::INT_VEC3:
		{
			Vec3<int>data = { (int)jsonObj->at(0),(int)jsonObj->at(1),(int)jsonObj->at(2) };
			memcpy(param.m_dataPtr, &data, sizeof(int) * 3);
			break;
		}
		case PARAM_TYPE::INT_VEC4:
		{
			Vec4<int>data = { (int)jsonObj->at(0),(int)jsonObj->at(1),(int)jsonObj->at(2),(int)jsonObj->at(3) };
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
			Vec2<float>data = { (float)jsonObj->at(0),(float)jsonObj->at(1) };
			memcpy(param.m_dataPtr, &data, sizeof(float) * 2);
			break;
		}
		case PARAM_TYPE::FLOAT_VEC3:
		{
			Vec3<float>data = { (float)jsonObj->at(0),(float)jsonObj->at(1),(float)jsonObj->at(2) };
			memcpy(param.m_dataPtr, &data, sizeof(float) * 3);
			break;
		}
		case PARAM_TYPE::FLOAT_VEC4:
		{
			Vec4<float>data = { (float)jsonObj->at(0),(float)jsonObj->at(1),(float)jsonObj->at(2),(float)jsonObj->at(3) };
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

		case PARAM_TYPE::COLOR:
		{
			Color data = { (float)jsonObj->at(0),jsonObj->at(1),jsonObj->at(2),jsonObj->at(3) };
			memcpy(param.m_dataPtr, &data, sizeof(float) * 4);
			break;
		}

		default:
			AppearMessageBox("Debugger：LoadParameterLog()失敗", param.m_label + "のタイプがNONEだったので上手く読み込めなかったよ");
			break;
		}
	}
}

void KuroEngine::Debugger::WriteParameterLog()
{
	using namespace KuroEngine;

	s_parameterLog.m_jsonData[m_title] = nlohmann::json::object();

	//デバッガの設定項目オブジェクト生成
	if (!s_parameterLog.m_jsonData[m_title].contains(s_settingKey))
		s_parameterLog.m_jsonData[m_title][s_settingKey] = nlohmann::json::object();

	//デバッガのイメージカラー項目
	auto& settings = s_parameterLog.m_jsonData[m_title][s_settingKey];
	settings[s_settingColKey] = { m_debuggerColor.x,m_debuggerColor.y,m_debuggerColor.z,m_debuggerColor.w };
	//テキストカラーフラグ項目
	settings[s_settingTextColKey] = m_textColor;
	//デバッガのアクティブ状態
	settings[s_settingActiveKey] = m_active;
	//デバッガのカスタムパラメータアクティブ状態
	settings[s_settingCustomParamKey] = m_customParamActive;

	//カスタムパラメータがなければここで終了
	if (m_customParamList.empty())return;

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

			case PARAM_TYPE::COLOR:
				paramObj[key] = { ((float*)param.m_dataPtr)[0],((float*)param.m_dataPtr)[1],((float*)param.m_dataPtr)[2],((float*)param.m_dataPtr)[3] };
				break;

			default:
				AppearMessageBox("Debugger：WriteParameterLog()失敗", param.m_label + "のタイプがNONEだったので上手く書き込めなかったよ");
				break;
		}
	}
}

void KuroEngine::Debugger::CustomParameterOnImgui()
{
	static const float INDENT_AMOUNT = 10.0f;
	ImGui::Indent(INDENT_AMOUNT);

	m_customParamDirty = false;

	auto& dirty = m_customParamDirty;

	//登録されたカスタムパラメータの設定
	for (auto& paramGroup : m_customParamGroup)
	{
		if (ImGui::TreeNode(paramGroup.first.c_str()))
		{
			auto& itemList = paramGroup.second;

			std::vector<CustomParameter*>::iterator itr = itemList.begin();
			for (; itr != itemList.end();)
			{
				const auto& item = **itr;
				const char* label = item.m_label.c_str();
				bool error = false;
				float min = item.m_isMinMax ? item.m_min : 0.0f;
				float max = item.m_isMinMax ? item.m_max : 0.0f;

				if (item.m_type != PARAM_TYPE::BOOL && item.m_type != PARAM_TYPE::STRING && item.m_type != PARAM_TYPE::COLOR)
				{
					ImGui::PushItemWidth(100);
				}

				switch (item.m_type)
				{
					case PARAM_TYPE::INT:
						dirty = dirty ? dirty : ImGui::DragInt(label, (int*)(item.m_dataPtr), m_customParamDragSpeed, (int)min, (int)max);
						break;
					case PARAM_TYPE::INT_VEC2:
						dirty = dirty ? dirty : ImGui::DragInt2(label, (int*)(item.m_dataPtr), m_customParamDragSpeed, (int)min, (int)max);
						break;
					case PARAM_TYPE::INT_VEC3:
						dirty = dirty ? dirty : ImGui::DragInt3(label, (int*)(item.m_dataPtr), m_customParamDragSpeed, (int)min, (int)max);
						break;
					case PARAM_TYPE::INT_VEC4:
						dirty = dirty ? dirty : ImGui::DragInt4(label, (int*)(item.m_dataPtr), m_customParamDragSpeed, (int)min, (int)max);
						break;

					case PARAM_TYPE::FLOAT:
						dirty = dirty ? dirty : ImGui::DragFloat(label, (float*)(item.m_dataPtr), m_customParamDragSpeed, min, max);
						break;
					case PARAM_TYPE::FLOAT_VEC2:
						dirty = dirty ? dirty : ImGui::DragFloat2(label, (float*)(item.m_dataPtr), m_customParamDragSpeed, min, max);
						break;
					case PARAM_TYPE::FLOAT_VEC3:
						dirty = dirty ? dirty : ImGui::DragFloat3(label, (float*)(item.m_dataPtr), m_customParamDragSpeed, min, max);
						break;
					case PARAM_TYPE::FLOAT_VEC4:
						dirty = dirty ? dirty : ImGui::DragFloat4(label, (float*)(item.m_dataPtr), m_customParamDragSpeed, min, max);
						break;

					case PARAM_TYPE::BOOL:
						dirty = dirty ? dirty : ImGui::Checkbox(label, (bool*)item.m_dataPtr);
						break;

					case PARAM_TYPE::STRING:
					{
						std::string* strPtr = (std::string*)item.m_dataPtr;
						dirty = dirty ? dirty : ImGui::InputText(label, (char*)strPtr->c_str(), strPtr->capacity() + 1);
						break;
					}

					case PARAM_TYPE::COLOR:
						dirty = dirty ? dirty : ImGui::ColorPicker4(label, (float*)(item.m_dataPtr), ImGuiColorEditFlags_AlphaBar);
						break;

					default:
						AppearMessageBox("Debugger：CustomParamterOnImgui()失敗", item.m_label + "のタイプがNONEだったのでimgui上でいじれないよ");
						error = true;
						break;
				}

				if (item.m_type != PARAM_TYPE::BOOL && item.m_type != PARAM_TYPE::STRING && item.m_type != PARAM_TYPE::COLOR)
				{
					ImGui::PopItemWidth();
				}

				if (error)itr = itemList.erase(itr);
				else itr++;
			}

			ImGui::TreePop();
		}
	}
	ImGui::Unindent(INDENT_AMOUNT);
}
