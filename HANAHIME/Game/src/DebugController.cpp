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

	//�f�o�b�O���[�h�؂�ւ�
	if (UsersInput::Instance()->KeyOnTrigger(DIK_ESCAPE))
	{
		m_active = !m_active;

		auto hwnd = WinApp::Instance()->GetHwnd();

		if (!m_active)
		{
			//�}�E�X��\��
			ShowCursor(false);
			//�}�E�X�̕�����
			RECT winRect;
			GetWindowRect(hwnd, &winRect);
			ClipCursor(&winRect);
		}
		else
		{
			//�}�E�X�\��
			ShowCursor(true);
			//�}�E�X���W���E�B���h�E�����ɃZ�b�g
			auto winCenter = WinApp::Instance()->GetExpandWinCenter().Int();
			POINT screenWinCenterPos = { winCenter.x,winCenter.y };
			ClientToScreen(hwnd, &screenWinCenterPos);
			SetCursorPos(screenWinCenterPos.x, screenWinCenterPos.y);
			//�}�E�X�̕����߉���
			ClipCursor(nullptr);
		}
	}

	//�ȉ��f�o�b�O����
	if (!m_active)return;
}