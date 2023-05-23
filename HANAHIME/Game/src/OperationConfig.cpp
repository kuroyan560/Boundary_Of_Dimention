#include "OperationConfig.h"
#include"FrameWork/UsersInput.h"
#include"FrameWork/WinApp.h"

using namespace KuroEngine;

OperationConfig::OperationConfig()
	:Debugger("OperationConfig", true)
{
	//�e�f�o�C�X�ł̐ݒ�l���J�X�^���p�����[�^�ɒǉ�
	for (int i = 0; i < INPUT_DEVICE::NUM; ++i)
	{
		auto deviceName = m_inputDeviceNames[i];
		AddCustomParameter("CameraSensitivity", { deviceName,"CameraSensitivity" },
			PARAM_TYPE::FLOAT, &m_params[i].m_camSensitivity, deviceName);
	}

	LoadParameterLog();

	//����L�[���蓖��
	m_operationKeyCode =
	{
		DIK_SPACE,	//����
		DIK_ESCAPE,	//�L�����Z��
		DIK_RETURN,	//�J�����̋������[�h�؂�ւ�
		DIK_R,	//�J�������Z�b�g
		DIK_SPACE,	//�n���ɐ���
		DIK_LSHIFT,	//�W�b�v���C���ɏ��
		DIK_TAB,	//�|�[�Y��ʂ�
	};

	//����{�^�����蓖��
	m_operationButton =
	{
		A,	//����
		B,	//�L�����Z��
		X,	//�J�����̋������[�h�؂�ւ�
		LT,	//�J�������Z�b�g
		RT,	//�n���ɐ���
		A,	//�W�b�v���C���ɏ��
		START,	//�|�[�Y��ʂ�
	};
}

void OperationConfig::OnImguiItems()
{
	using namespace KuroEngine;

	//�Ō�̓��͂̃f�o�C�X
	ImGui::Text("LatestDevice : %s", m_inputDeviceNames[m_nowInputDevice].c_str());

	//�}�E�X���͕\��
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

	//���X�e�B�b�N�̓��͂�ϊ�
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

	//���X�e�B�b�N�̓��͂�ϊ�
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

	//�}�E�X����
	auto mouseMove = UsersInput::Instance()->GetMouseMove();
	//�E�B���h�E�T�C�Y�ɂ���đ��ΓI�ȃX�P�[���ɍ��킹��
	const auto scale = Vec2<float>(1.0f, 1.0f) / WinApp::Instance()->GetExpandWinSize();
	result = { mouseMove.m_inputX * scale.x * sensitivity,  mouseMove.m_inputY * scale.y * sensitivity,0.0f };
	if (!result.IsZero())
	{
		RegisterLatestDevice(INPUT_DEVICE::KEY_BOARD_MOUSE);
		return result;
	}

	//�E�X�e�B�b�N�̓��͂�ϊ�
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
	//�S�Ă̓��͂��󂯕t���Ă��Ȃ�
	if (!m_isAllInputActive)return false;

	//�C���Q�[���̑�����󂯕t���Ȃ����
	if (!m_isInGameOperationActive &&
		std::find(m_inGameOperationArray.begin(), m_inGameOperationArray.end(), arg_operation) != m_inGameOperationArray.end())return false;

	return KeyInput(arg_pattern, m_operationKeyCode[arg_operation]) || ControllerInput(arg_pattern, m_operationButton[arg_operation]);
}

bool OperationConfig::CheckAllOperationInput()
{
	//�x�N�g������
	if (!GetMoveVecFuna(XMQuaternionIdentity()).IsZero())return true;
	if (!GetScopeMove().IsZero())return true;

	//�{�^������
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