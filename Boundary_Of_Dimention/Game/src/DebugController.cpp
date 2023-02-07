#include "DebugController.h"
#include"FrameWork/UsersInput.h"
#include"FrameWork/WinApp.h"

DebugController::DebugController()
{
	POINT pos = {};
	GetCursorPos(&pos);
}

void DebugController::Update()
{
	using namespace KuroEngine;

	//デバッグモード切り替え
	if (UsersInput::Instance()->KeyOnTrigger(DIK_ESCAPE))
	{
		m_active = !m_active;

		auto hwnd = WinApp::Instance()->GetHwnd();

		if (!m_active)
		{
			//マウス非表示
			ShowCursor(false);
			//マウスの閉じ込め
			RECT winRect;
			GetWindowRect(hwnd, &winRect);
			ClipCursor(&winRect);
		}
		else
		{
			//マウス表示
			ShowCursor(true);
			//マウス座標をウィンドウ中央にセット
			auto winCenter = WinApp::Instance()->GetExpandWinCenter().Int();
			POINT screenWinCenterPos = { winCenter.x,winCenter.y };
			ClientToScreen(hwnd, &screenWinCenterPos);
			SetCursorPos(screenWinCenterPos.x, screenWinCenterPos.y);
			//マウスの閉じ込め解除
			ClipCursor(nullptr);
		}
	}

	//以下デバッグ処理
	if (!m_active)return;
}