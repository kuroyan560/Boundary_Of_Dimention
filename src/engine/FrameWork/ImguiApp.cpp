#include "ImguiApp.h"
#include"DirectX12/D3D12App.h"
#include"imgui_spectrum.h"

KuroEngine::ImguiApp::ImguiApp(const ComPtr<ID3D12Device>& Device, const HWND& Hwnd)
{
	using namespace ImGui;

	// SRV_CBV_UAV のディスクリプタヒープ
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	HRESULT result = Device->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(m_heap.GetAddressOf()));

	if (m_heap == nullptr)assert(0);

	if (ImGui::CreateContext() == nullptr)assert(0);

	auto& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	bool blnResult = ImGui_ImplWin32_Init(Hwnd);
	if (!blnResult)assert(0);

	blnResult = ImGui_ImplDX12_Init(
		Device.Get(),
		2,
		DXGI_FORMAT_R8G8B8A8_UNORM,	//書き込み先のRTVのフォーマット
		m_heap.Get(),
		m_heap.Get()->GetCPUDescriptorHandleForHeapStart(),	//CPUハンドル
		m_heap.Get()->GetGPUDescriptorHandleForHeapStart());	//GPUハンドル

	ImGuiStyle* style = &ImGui::GetStyle();
	style->GrabRounding = 4.0f;

	ImVec4* colors = style->Colors;
	colors[ImGuiCol_Text] = ColorConvertU32ToFloat4(Spectrum::GRAY800); // text on hovered controls is gray900
	colors[ImGuiCol_TextDisabled] = ColorConvertU32ToFloat4(Spectrum::GRAY500);
	colors[ImGuiCol_WindowBg] = ColorConvertU32ToFloat4(Spectrum::GRAY200);
	colors[ImGuiCol_ChildBg] = ColorConvertU32ToFloat4(Spectrum::GRAY400);
	colors[ImGuiCol_PopupBg] = ColorConvertU32ToFloat4(Spectrum::GRAY50); // not sure about this. Note: applies to tooltips too.
	colors[ImGuiCol_Border] = ColorConvertU32ToFloat4(Spectrum::GRAY300);
	colors[ImGuiCol_BorderShadow] = ColorConvertU32ToFloat4(Spectrum::Static::NONE); // We don't want shadows. Ever.
	colors[ImGuiCol_FrameBg] = ColorConvertU32ToFloat4(Spectrum::GRAY75); // this isnt right, spectrum does not do this, but it's a good fallback
	colors[ImGuiCol_FrameBgHovered] = ColorConvertU32ToFloat4(Spectrum::GRAY50);
	colors[ImGuiCol_FrameBgActive] = ColorConvertU32ToFloat4(Spectrum::GRAY200);
	colors[ImGuiCol_TitleBg] = ColorConvertU32ToFloat4(Spectrum::GRAY300); // those titlebar values are totally made up, spectrum does not have this.
	colors[ImGuiCol_TitleBgActive] = ColorConvertU32ToFloat4(Spectrum::GRAY200);
	colors[ImGuiCol_TitleBgCollapsed] = ColorConvertU32ToFloat4(Spectrum::GRAY400);
	colors[ImGuiCol_MenuBarBg] = ColorConvertU32ToFloat4(Spectrum::GRAY100);
	colors[ImGuiCol_ScrollbarBg] = ColorConvertU32ToFloat4(Spectrum::GRAY100); // same as regular background
	colors[ImGuiCol_ScrollbarGrab] = ColorConvertU32ToFloat4(Spectrum::GRAY400);
	colors[ImGuiCol_ScrollbarGrabHovered] = ColorConvertU32ToFloat4(Spectrum::GRAY600);
	colors[ImGuiCol_ScrollbarGrabActive] = ColorConvertU32ToFloat4(Spectrum::GRAY700);
	colors[ImGuiCol_CheckMark] = ColorConvertU32ToFloat4(Spectrum::BLUE500);
	colors[ImGuiCol_SliderGrab] = ColorConvertU32ToFloat4(Spectrum::GRAY700);
	colors[ImGuiCol_SliderGrabActive] = ColorConvertU32ToFloat4(Spectrum::GRAY800);
	colors[ImGuiCol_Button] = ColorConvertU32ToFloat4(Spectrum::GRAY75); // match default button to Spectrum's 'Action Button'.
	colors[ImGuiCol_ButtonHovered] = ColorConvertU32ToFloat4(Spectrum::GRAY50);
	colors[ImGuiCol_ButtonActive] = ColorConvertU32ToFloat4(Spectrum::GRAY200);
	colors[ImGuiCol_Header] = ColorConvertU32ToFloat4(Spectrum::BLUE400);
	colors[ImGuiCol_HeaderHovered] = ColorConvertU32ToFloat4(Spectrum::BLUE500);
	colors[ImGuiCol_HeaderActive] = ColorConvertU32ToFloat4(Spectrum::BLUE600);
	colors[ImGuiCol_Separator] = ColorConvertU32ToFloat4(Spectrum::GRAY400);
	colors[ImGuiCol_SeparatorHovered] = ColorConvertU32ToFloat4(Spectrum::GRAY600);
	colors[ImGuiCol_SeparatorActive] = ColorConvertU32ToFloat4(Spectrum::GRAY700);
	colors[ImGuiCol_ResizeGrip] = ColorConvertU32ToFloat4(Spectrum::GRAY400);
	colors[ImGuiCol_ResizeGripHovered] = ColorConvertU32ToFloat4(Spectrum::GRAY600);
	colors[ImGuiCol_ResizeGripActive] = ColorConvertU32ToFloat4(Spectrum::GRAY700);
	colors[ImGuiCol_PlotLines] = ColorConvertU32ToFloat4(Spectrum::BLUE400);
	colors[ImGuiCol_PlotLinesHovered] = ColorConvertU32ToFloat4(Spectrum::BLUE600);
	colors[ImGuiCol_PlotHistogram] = ColorConvertU32ToFloat4(Spectrum::BLUE400);
	colors[ImGuiCol_PlotHistogramHovered] = ColorConvertU32ToFloat4(Spectrum::BLUE600);
	colors[ImGuiCol_TextSelectedBg] = ColorConvertU32ToFloat4((Spectrum::BLUE400 & 0x00FFFFFF) | 0x33000000);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ColorConvertU32ToFloat4((Spectrum::GRAY900 & 0x00FFFFFF) | 0x0A000000);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

KuroEngine::ImguiApp::~ImguiApp()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void KuroEngine::ImguiApp::BeginImgui(const ComPtr<ID3D12GraphicsCommandList>& CmdList)
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void KuroEngine::ImguiApp::EndImgui(const ComPtr<ID3D12GraphicsCommandList>& CmdList)
{
	ImGui::Render();
	CmdList->SetDescriptorHeaps(1, m_heap.GetAddressOf());

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), CmdList.Get());

	auto& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault(NULL, (void*)CmdList.Get());
	}
}

#include"Render/RenderObject/ModelInfo/Material.h"
void KuroEngine::ImguiApp::DebugMaterial(std::weak_ptr<Material> Material, const IMGUI_DEBUG_MODE& Mode)
{
	ImGui::Begin(("DebugMaterial - " + m_modeName[Mode]).c_str());

	auto m = Material.lock();

	//参照
	if(Mode == REFERENCE)
	{
		//Lambert
		{
			ImGui::Text("Lambert");
			ImGui::Indent(m_indent);

			ImGui::Text("Ambient - ");
			ImGui::SameLine();
			const auto ambient = m->constData.lambert.ambient;
			const auto ambientFactor = m->constData.lambert.ambientFactor;
			ImGui::ColorButton("AmbientColor", ImVec4(ambient.x, ambient.y, ambient.z, ambientFactor));

			ImGui::Text("Diffuse - ");
			ImGui::SameLine();
			const auto diffuse = m->constData.lambert.diffuse;
			const auto diffuseFactor = m->constData.lambert.diffuseFactor;
			ImGui::ColorButton("DiffuseColor", ImVec4(diffuse.x, diffuse.y, diffuse.z, diffuseFactor));

			ImGui::Text("Emissive - ");
			ImGui::SameLine();
			const auto emissive = m->constData.lambert.emissive;
			const auto emissiveFactor = m->constData.lambert.emissiveFactor;
			ImGui::ColorButton("Emissive", ImVec4(emissive.x, emissive.y, emissive.z, emissiveFactor));

			ImGui::Text("Transparent - %.3f", m->constData.lambert.transparent);

			ImGui::Unindent(m_indent);
		}

		ImGui::Separator();

		//Phong
		{
			ImGui::Text("Phong");
			ImGui::Indent(m_indent);

			ImGui::Text("Specular - ");
			ImGui::SameLine();
			const auto specular = m->constData.phong.specular;
			const auto specularFactor = m->constData.phong.specularFactor;
			ImGui::ColorButton("Specular", ImVec4(specular.x, specular.y, specular.z, specularFactor));

			ImGui::Text("Shininess - %.3f", m->constData.phong.shininess);

			ImGui::Text("Reflection - %.3f", m->constData.phong.reflection);

			ImGui::Unindent(m_indent);
		}

		ImGui::Separator();

		//PBR
		{
			ImGui::Text("PBR");
			ImGui::Indent(m_indent);

			ImGui::Text("BaseColor - ");
			ImGui::SameLine();
			const auto baseColor = m->constData.pbr.baseColor;
			ImGui::ColorButton("BaseColor", ImVec4(baseColor.x, baseColor.y, baseColor.z, 1.0f));

			ImGui::Text("Metalness - %.3f", m->constData.pbr.metalness);

			ImGui::Text("Specular - %.3f", m->constData.pbr.specular);

			ImGui::Text("Roughness - %.3f", m->constData.pbr.roughness);
		}
	}
	//書き換え
	else if (Mode == REWRITE)
	{
		enum TYPE { LAMBERT, PHONG, PBR };
		static const std::array<std::string, 3> TYPE_STR = { "Lambert","Phong","PBR" };
		static int TYPE_IDX = PBR;
		static std::string CURRENT_TYPE = TYPE_STR[TYPE_IDX];

		bool changed = false;

		if (ImGui::BeginCombo("Type", CURRENT_TYPE.c_str()))
		{
			for (int n = 0; n < TYPE_STR.size(); ++n)
			{
				bool isSelected = (CURRENT_TYPE == TYPE_STR[n]);
				if (ImGui::Selectable(TYPE_STR[n].c_str(), isSelected))
				{
					CURRENT_TYPE = TYPE_STR[n];
					TYPE_IDX = n;
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		ImGui::Separator();

		//Lambert
		if (TYPE_IDX == LAMBERT)
		{
			if (ImGui::TreeNode("Ambient"))
			{
				Vec4<float>ambient;
				ambient.x = m->constData.lambert.ambient.x;
				ambient.y = m->constData.lambert.ambient.y;
				ambient.z = m->constData.lambert.ambient.z;
				ambient.w = m->constData.lambert.ambientFactor;
				if (ImGui::ColorPicker4("Ambient", &ambient.x))
				{
					m->constData.lambert.ambient.x = ambient.x;
					m->constData.lambert.ambient.y = ambient.y;
					m->constData.lambert.ambient.z = ambient.z;
					m->constData.lambert.ambientFactor = ambient.w;
					changed = true;
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Diffuse"))
			{
				Vec4<float>diffuse;
				diffuse.x = m->constData.lambert.diffuse.x;
				diffuse.y = m->constData.lambert.diffuse.y;
				diffuse.z = m->constData.lambert.diffuse.z;
				diffuse.w = m->constData.lambert.diffuseFactor;
				if (ImGui::ColorPicker4("Diffuse", &diffuse.x))
				{
					m->constData.lambert.diffuse.x = diffuse.x;
					m->constData.lambert.diffuse.y = diffuse.y;
					m->constData.lambert.diffuse.z = diffuse.z;
					m->constData.lambert.diffuseFactor = diffuse.w;
					changed = true;
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Emissive"))
			{
				Vec4<float>emissive;
				emissive.x = m->constData.lambert.emissive.x;
				emissive.y = m->constData.lambert.emissive.y;
				emissive.z = m->constData.lambert.emissive.z;
				emissive.w = m->constData.lambert.emissiveFactor;
				if (ImGui::ColorPicker4("Emissive", &emissive.x))
				{
					m->constData.lambert.emissive.x = emissive.x;
					m->constData.lambert.emissive.y = emissive.y;
					m->constData.lambert.emissive.z = emissive.z;
					m->constData.lambert.emissiveFactor = emissive.w;
					changed = true;
				}
				ImGui::TreePop();
			}
			if (ImGui::SliderFloat("Transparent", &m->constData.lambert.transparent, 0.0f, 1.0f))changed = true;
		}
		//Phong
		else if (TYPE_IDX == PHONG)
		{
			if (ImGui::TreeNode("Specular"))
			{
				Vec4<float>specular;
				specular.x = m->constData.phong.specular.x;
				specular.y = m->constData.phong.specular.y;
				specular.z = m->constData.phong.specular.z;
				specular.w = m->constData.phong.specularFactor;
				if (ImGui::ColorPicker4("Specular", &specular.x))
				{
					m->constData.phong.specular.x = specular.x;
					m->constData.phong.specular.y = specular.y;
					m->constData.phong.specular.z = specular.z;
					m->constData.phong.specularFactor = specular.w;
					changed = true;
				}
				ImGui::TreePop();
			}
			if (ImGui::SliderFloat("Shininess", &m->constData.phong.shininess, 0.0f, 1.0f))changed = true;
			if (ImGui::SliderFloat("Reflection", &m->constData.phong.reflection, 0.0, 1.0f))changed = true;
		}
		//PBR
		else if (TYPE_IDX == PBR)
		{
			if (ImGui::ColorPicker3("BaseColor", &m->constData.pbr.baseColor.x))changed = true;
			if (ImGui::SliderFloat("Specular", &m->constData.pbr.specular, 0.0f, 1.0f))changed = true;
			if (ImGui::SliderFloat("Metalness", &m->constData.pbr.metalness, 0.0f, 1.0f))changed = true;
			if (ImGui::SliderFloat("Roughness", &m->constData.pbr.roughness, 0.0f, 1.0f))changed = true;
		}

		if (changed)
		{
			m->Mapping();
		}
	}

	ImGui::End();
}

int KuroEngine::ImguiApp::WrappedCombo(const std::string arg_label, const std::string* arg_nameArray, const size_t arg_arraySize, int arg_selectedIdx)
{
	assert(0 <= arg_selectedIdx && arg_selectedIdx < arg_arraySize);

	std::string currentStr = arg_nameArray[arg_selectedIdx];
	if (ImGui::BeginCombo(arg_label.c_str(), currentStr.c_str()))
	{
		for (int i = 0; i < static_cast<int>(arg_arraySize); ++i)
		{
			bool isSelected = arg_selectedIdx == i;
			if (ImGui::Selectable(arg_nameArray[i].c_str(), isSelected))arg_selectedIdx = i;
			if (isSelected)ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	return arg_selectedIdx;
}
