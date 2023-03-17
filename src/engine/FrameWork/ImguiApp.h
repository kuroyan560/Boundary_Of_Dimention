#pragma once
#include<wrl.h>
#include<d3d12.h>
#include"imgui.h"
#include"imgui_impl_win32.h"
#include"imgui_impl_dx12.h"
#include<stdio.h>
#include<array>
#include<string>
#include<vector>
#include<memory>

namespace KuroEngine
{
	enum IMGUI_DEBUG_MODE
	{
		REFERENCE,	//参照（見るだけ）
		REWRITE,	//書き換え
		IMGUI_DEBUG_MODE_NUM
	};

	class Material;
	class TextureBuffer;
	class RenderTarget;

	class ImguiApp
	{
	private:
		template<class T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;

	private:
		ComPtr<ID3D12DescriptorHeap>m_heap;

		const std::array<std::string, IMGUI_DEBUG_MODE_NUM>m_modeName =
		{
			"Reference",
			"Rewrite"
		};
		const float m_indent = 32.0f;

	public:
		ImguiApp(const ComPtr<ID3D12Device>& Device, const HWND& Hwnd);
		~ImguiApp();
		void BeginImgui(const ComPtr<ID3D12GraphicsCommandList>& CmdList);
		void EndImgui(const ComPtr<ID3D12GraphicsCommandList>& CmdList);

		//マテリアル情報調整
		void DebugMaterial(std::weak_ptr<Material>Material, const IMGUI_DEBUG_MODE& Mode);

		/// <summary>
		/// imguiのCombo関数のラップ関数
		/// </summary>
		/// <param name="arg_label">ラベル名</param>
		/// <param name="arg_nameArray">要素の名前配列</param>
		/// <param name="arg_arraySize">配列サイズ</param>
		/// <param name="arg_selectedIdx">現在選択されているインデックス</param>
		/// <returns>新しいインデックス</returns>
		static int WrappedCombo(const std::string arg_label, const std::string* arg_nameArray, const size_t arg_arraySize, int arg_selectedIdx);
	};
}