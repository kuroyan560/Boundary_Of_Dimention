#include "OperationConfig.h"
#include"FrameWork/UsersInput.h"
#include"FrameWork/WinApp.h"

OperationConfig::OperationConfig()
	:KuroEngine::Debugger("OperationConfig", true)
{
	//各デバイスでの設定値をカスタムパラメータに追加
	for (int i = 0; i < INPUT_DEVICE::NUM; ++i)
	{
		auto deviceName = m_inputDeviceNames[i];
		AddCustomParameter("CameraSensitivity", { deviceName,"CameraSensitivity" },
			PARAM_TYPE::FLOAT, &m_params[i].m_camSensitivity, deviceName);
	}

	LoadParameterLog();
}

void OperationConfig::OnImguiItems()
{
	using namespace KuroEngine;

	//最後の入力のデバイス
	ImGui::Text("LatestDevice : %s", m_inputDeviceNames[m_nowInputDevice].c_str());

	//マウス入力表示
	auto mouseMove = UsersInput::Instance()->GetMouseMove();
	ImGui::BeginChild(ImGui::GetID((void *)0), ImVec2(250, 150));
	ImGui::Text("inputX : %d", mouseMove.m_inputX);
	ImGui::Text("inputY : %d", mouseMove.m_inputY);
	ImGui::Text("inputZ : %d", mouseMove.m_inputZ);
	ImGui::EndChild();

}

KuroEngine::Vec3<float> OperationConfig::GetMoveVec(KuroEngine::Quaternion arg_rotate)
{
	using namespace KuroEngine;

	if (!m_isActive)return { 0,0,0 };

	Vec3<float>result;

	if (UsersInput::Instance()->KeyInput(DIK_W))result.z += 1.0f;
	if (UsersInput::Instance()->KeyInput(DIK_S))result.z -= 1.0f;
	if (UsersInput::Instance()->KeyInput(DIK_D))result.x += 1.0f;
	if (UsersInput::Instance()->KeyInput(DIK_A))result.x -= 1.0f;
	if (!result.IsZero())
	{
		RegisterLatestDevice(INPUT_DEVICE::KEY_BOARD_MOUSE);
		return KuroEngine::Math::TransformVec3(result.GetNormal(), arg_rotate);
	}

	//左スティックの入力を変換
	auto input = UsersInput::Instance()->GetLeftStickVec(0);
	if (!input.IsZero())RegisterLatestDevice(INPUT_DEVICE::CONTROLLER);
	result = Vec3<float>(input.x, 0.0f, -input.y);
	return KuroEngine::Math::TransformVec3(result.GetNormal(), arg_rotate);
}

KuroEngine::Vec3<float> OperationConfig::GetMoveVecFuna(KuroEngine::Quaternion arg_rotate)
{
	using namespace KuroEngine;

	if (!m_isActive)return { 0,0,0 };

	Vec3<float>result;

	if (UsersInput::Instance()->KeyInput(DIK_W))result.z += 1.0f;
	if (UsersInput::Instance()->KeyInput(DIK_S))result.z -= 1.0f;
	if (UsersInput::Instance()->KeyInput(DIK_D))result.x += 1.0f;
	if (UsersInput::Instance()->KeyInput(DIK_A))result.x -= 1.0f;
	if (!result.IsZero())
	{
		RegisterLatestDevice(INPUT_DEVICE::KEY_BOARD_MOUSE);
		return KuroEngine::Math::TransformVec3(result.GetNormal(), arg_rotate);
	}

	//左スティックの入力を変換
	auto input = UsersInput::Instance()->GetLeftStickVecFuna(0);
	if (!input.IsZero())RegisterLatestDevice(INPUT_DEVICE::CONTROLLER);
	result = Vec3<float>(input.x, 0.0f, -input.y);
	return KuroEngine::Math::TransformVec3(result.GetNormal(), arg_rotate);
}

KuroEngine::Vec3<float> OperationConfig::GetScopeMove()
{
	using namespace KuroEngine;

	if (!m_isActive)return { 0,0,0 };

	float sensitivity = m_params[m_nowInputDevice].m_camSensitivity;
	Vec3<float>result;

	//マウス入力
	auto mouseMove = UsersInput::Instance()->GetMouseMove();
	//ウィンドウサイズによって相対的なスケールに合わせる
	const auto scale = Vec2<float>(1.0f, 1.0f) / WinApp::Instance()->GetExpandWinSize();
	result = { mouseMove.m_inputX * scale.x * sensitivity,  mouseMove.m_inputY * scale.y * sensitivity,0.0f };
	if (!result.IsZero())
	{
		RegisterLatestDevice(INPUT_DEVICE::KEY_BOARD_MOUSE);
		return result;
	}

	//右スティックの入力を変換
	auto input = UsersInput::Instance()->GetRightStickVec(0);
	result = { input.x * sensitivity, -input.y * sensitivity, 0.0f };
	if (!input.IsZero())RegisterLatestDevice(INPUT_DEVICE::CONTROLLER);
	return result;
}

bool OperationConfig::InputDone()
{
	bool doneFlag = KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_SPACE) || KuroEngine::UsersInput::Instance()->ControllerOnTrigger(0, KuroEngine::XBOX_BUTTON::A);
	return doneFlag;
}

bool OperationConfig::InputCancel()
{
	bool cancelFlag = KuroEngine::UsersInput::Instance()->KeyOnTrigger(DIK_ESCAPE) || KuroEngine::UsersInput::Instance()->ControllerOnTrigger(0, KuroEngine::XBOX_BUTTON::B);
	return cancelFlag;
}
