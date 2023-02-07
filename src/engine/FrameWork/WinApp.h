#pragma once
#include<windows.h>
#include<string>
#include"Common/Vec.h"

namespace KuroEngine
{
	class WinApp
	{
	private:
		static WinApp* s_instance;
	public:
		static WinApp* Instance()
		{
			if (s_instance == nullptr)
			{
				printf("WinAppのインスタンスを呼び出そうとしましたがnullptrでした\n");
				assert(0);
			}
			return s_instance;
		}

	private:
		Vec2<int>m_winSize;
		Vec2<float>m_winDifferRate = { 1.0f,1.0f };	//フルスクリーンにした際のウィンドウサイズ倍率
		WNDCLASSEX m_wc{};
		HWND m_hwnd;

		void Initialize(const std::string& arg_winName, const Vec2<int>arg_winSize, const bool& arg_fullScreen, const wchar_t* arg_iconPath);

	public:
		WinApp(const std::string& arg_winName, const Vec2<int>arg_winSize, const bool& arg_fullScreen, const wchar_t* arg_iconPath = nullptr)
		{
			if (s_instance != nullptr)
			{
				printf("既にインスタンスがあります。WinAppは１つのインスタンスしか持てません\n");
				assert(0);
			}
			s_instance = this;
			Initialize(arg_winName, arg_winSize, arg_fullScreen, arg_iconPath);
		}
		const HWND& GetHwnd() { return m_hwnd; }
		const WNDCLASSEX& GetWinClass() { return m_wc; }
		const Vec2<int>& GetWinSize() { return m_winSize; }
		const Vec2<int>& GetWinCenter() { return m_winSize / 2; }
		Vec2<float>GetExpandWinSize() { return m_winSize.Float() * m_winDifferRate; }	//ウィンドウサイズ倍率がかかったサイズ
		Vec2<float>GetExpandWinCenter() { return GetExpandWinSize() / 2.0f; }
		const Vec2<float>& GetWinDifferRate() { return m_winDifferRate; }
		float GetAspect()
		{
			auto size = GetExpandWinSize();
			return size.x / size.y;
		}
	};
}