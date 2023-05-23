#include "OperationConfig.h"
#include"FrameWork/UsersInput.h"
#include"FrameWork/WinApp.h"

using namespace KuroEngine;

OperationConfig::OperationConfig()
	:Debugger("OperationConfig", true)
{
	//各デバイスでの設定値をカスタムパラメータに追加
	for (int i = 0; i < INPUT_DEVICE::NUM; ++i)
	{
		auto deviceName = m_inputDeviceNames[i];
		AddCustomParameter("CameraSensitivity", { deviceName,"CameraSensitivity" },
			PARAM_TYPE::FLOAT, &m_params[i].m_camSensitivity, deviceName);
	}

	LoadParameterLog();

	//操作キー割り当て
	m_operationKeyCode =
	{
		DIK_SPACE,	//決定
		DIK_ESCAPE,	//キャンセル
		DIK_RETURN,	//カメラの距離モード切り替え
		DIK_R,	//カメラリセット
		DIK_SPACE,	//地中に潜る
		DIK_LSHIFT,	//ジップラインに乗る
		DIK_TAB,	//ポーズ画面へ
	};

	//操作ボタン割り当て
	m_operationButton =
	{
		A,	//決定
		B,	//キャンセル
		X,	//カメラの距離モード切り替え
		LT,	//カメラリセット
		RT,	//地中に潜る
		A,	//ジップラインに乗る
		START,	//ポーズ画面へ
	};
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

bool OperationConfig::ControllerInput(INPUT_PATTERN arg_pattern, KuroEngine::XBOX_BUTTON arg_xboxButton)
{
	using namespace KuroEngine;
	switch (arg_pattern)
	{
		case HOLD:
			return UsersInput::Instance()->ControllerInput(0, arg_xboxButton);
			break;
		case ON_TRIGGER:
			return UsersInput::Instance()->ControllerOnTrigger(0, arg_xboxButton);
			break;
		case OFF_TRIGGER:
			return UsersInput::Instance()->ControllerOffTrigger(0, arg_xboxButton);
			break;
		case ON_OFF_TRIGGER:
			return UsersInput::Instance()->ControllerOnTrigger(0, arg_xboxButton) || UsersInput::Instance()->ControllerOffTrigger(0, arg_xboxButton);
			break;
		default:
			break;
	}
	return false;
}

bool OperationConfig::KeyInput(INPUT_PATTERN arg_pattern, int arg_keyCode)
{
	using namespace KuroEngine;
	switch (arg_pattern)
	{
		case HOLD:
			return UsersInput::Instance()->KeyInput(arg_keyCode);
			break;
		case ON_TRIGGER:
			return UsersInput::Instance()->KeyOnTrigger(arg_keyCode);
			break;
		case OFF_TRIGGER:
			return UsersInput::Instance()->KeyOffTrigger(arg_keyCode);
			break;
		case ON_OFF_TRIGGER:
			return UsersInput::Instance()->KeyOnTrigger(arg_keyCode) || UsersInput::Instance()->KeyOffTrigger(arg_keyCode);
			break;
		default:
			break;
	}
	return false;
}

KuroEngine::Vec3<float> OperationConfig::GetMoveVec(KuroEngine::Quaternion arg_rotate)
{
	if (!m_isAllInputActive || !m_isInGameOperationActive)return { 0,0,0 };

	Vec3<float>result;

	if (UsersInput::Instance()->KeyInput(DIK_W))result.z += 1.0f;
	if (UsersInput::Instance()->KeyInput(DIK_S))result.z -= 1.0f;
	if (UsersInput::Instance()->KeyInput(DIK_D))result.x += 1.0f;
	if (UsersInput::Instance()->KeyInput(DIK_A))result.x -= 1.0f;
	if (!result.IsZero())
	{
		RegisterLatestDevice(INPUT_DEVICE::KEY_BOARD_MOUSE);
		return Math::TransformVec3(result.GetNormal(), arg_rotate);
	}

	//左スティックの入力を変換
	auto input = UsersInput::Instance()->GetLeftStickVec(0);
	if (!input.IsZero())RegisterLatestDevice(INPUT_DEVICE::CONTROLLER);
	result = Vec3<float>(input.x, 0.0f, -input.y);
	return Math::TransformVec3(result.GetNormal(), arg_rotate);
}

KuroEngine::Vec3<float> OperationConfig::GetMoveVecFuna(KuroEngine::Quaternion arg_rotate)
{
	if (!m_isAllInputActive || !m_isInGameOperationActive)return { 0,0,0 };

	Vec3<float>result;

	if (UsersInput::Instance()->KeyInput(DIK_W))result.z += 1.0f;
	if (UsersInput::Instance()->KeyInput(DIK_S))result.z -= 1.0f;
	if (UsersInput::Instance()->KeyInput(DIK_D))result.x += 1.0f;
	if (UsersInput::Instance()->KeyInput(DIK_A))result.x -= 1.0f;
	if (!result.IsZero())
	{
		RegisterLatestDevice(INPUT_DEVICE::KEY_BOARD_MOUSE);
		return Math::TransformVec3(result.GetNormal(), arg_rotate);
	}

	//左スティックの入力を変換
	auto input = UsersInput::Instance()->GetLeftStickVecFuna(0);
	if (!input.IsZero())RegisterLatestDevice(INPUT_DEVICE::CONTROLLER);
	result = Vec3<float>(input.x, 0.0f, -input.y);
	return Math::TransformVec3(result.GetNormal(), arg_rotate);
}

KuroEngine::Vec3<float> OperationConfig::GetScopeMove()
{
	if (!m_isAllInputActive || !m_isInGameOperationActive)return { 0,0,0 };

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

bool OperationConfig::GetSelectVec(SELECT_VEC arg_vec)
{
	static const float STICK_DEAD_RANGE = 0.15f;

	switch (arg_vec)
	{
		case SELECT_VEC_UP:
			return UsersInput::Instance()->ControllerOnTrigger(0, XBOX_STICK::L_UP, STICK_DEAD_RANGE) || UsersInput::Instance()->KeyOnTrigger(DIK_W);
			break;
		case SELECT_VEC_DOWN:
			return UsersInput::Instance()->ControllerOnTrigger(0, XBOX_STICK::L_DOWN, STICK_DEAD_RANGE) || UsersInput::Instance()->KeyOnTrigger(DIK_S);
			break;
		case SELECT_VEC_LEFT:
			return UsersInput::Instance()->ControllerOnTrigger(0, XBOX_STICK::L_LEFT, STICK_DEAD_RANGE) || UsersInput::Instance()->KeyOnTrigger(DIK_A);
			break;
		case SELECT_VEC_RIGHT:
			return UsersInput::Instance()->ControllerOnTrigger(0, XBOX_STICK::L_RIGHT, STICK_DEAD_RANGE) || UsersInput::Instance()->KeyOnTrigger(DIK_D);
			break;
		default:
			break;
	}
	return false;
}

bool OperationConfig::GetOperationInput(OPERATION_TYPE arg_operation, INPUT_PATTERN arg_pattern)
{
	//全ての入力を受け付けていない
	if (!m_isAllInputActive)return false;

	//インゲームの操作を受け付けない状態
	if (!m_isInGameOperationActive &&
		std::find(m_inGameOperationArray.begin(), m_inGameOperationArray.end(), arg_operation) != m_inGameOperationArray.end())return false;

	return KeyInput(arg_pattern, m_operationKeyCode[arg_operation]) || ControllerInput(arg_pattern, m_operationButton[arg_operation]);
}

bool OperationConfig::CheckAllOperationInput()
{
	//ベクトル入力
	if (!GetMoveVecFuna(XMQuaternionIdentity()).IsZero())return true;
	if (!GetScopeMove().IsZero())return true;

	//ボタン入力
	for (int ope = 0; ope < OPERATION_TYPE_NUM; ++ope)
	{
		if (GetOperationInput((OPERATION_TYPE)ope, HOLD))
		{
			return true;
		}
	}
	return false;
}

bool OperationConfig::DebugKeyInputOnTrigger(int arg_keyCode)
{
	if (!m_isDebug)return false;
	return UsersInput::Instance()->KeyOnTrigger(arg_keyCode);
}