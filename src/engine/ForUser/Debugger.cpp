#include "Debugger.h"
#include"FrameWork/Fps.h"

std::vector<KuroEngine::Debugger*>KuroEngine::Debugger::s_debuggerArray;
int KuroEngine::Debugger::s_id = 0;

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
