#include "BasicDrawParameters.h"
#include"FrameWork/ImguiApp.h"

void IndividualDrawParameter::ImguiDebugItem()
{
	if (ImGui::TreeNode("Toon"))
	{
		if (ImGui::Button("DefaultColor"))
		{
			*this = GetDefault();
		}

		//塗りつぶし色
		if (ImGui::TreeNode("Fill"))
		{
			ImGui::ColorPicker4("FillColor", (float*)&m_fillColor, ImGuiColorEditFlags_AlphaBar);
			ImGui::TreePop();
		}

		//明るい部分に乗算する色
		if (ImGui::TreeNode("BrightMul"))
		{
			ImGui::ColorPicker4("BrightMulColor", (float*)&m_brightMulColor, ImGuiColorEditFlags_AlphaBar);
			ImGui::TreePop();
		}

		//暗い部分に乗算する色
		if (ImGui::TreeNode("DarkMul"))
		{
			ImGui::ColorPicker4("DarkMulColor", (float*)&m_darkMulColor, ImGuiColorEditFlags_AlphaBar);
			ImGui::TreePop();
		}

		//リムライト部分の補正色
		if (ImGui::TreeNode("LimBright"))
		{
			ImGui::ColorPicker4("LimBrightColor", (float*)&m_limBrightColor, ImGuiColorEditFlags_AlphaBar);
			ImGui::TreePop();
		}

		//エッジカラー
		if (ImGui::TreeNode("Edge"))
		{
			ImGui::ColorPicker4("EdgeColor", (float*)&m_edgeColor, ImGuiColorEditFlags_AlphaBar);
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
}
