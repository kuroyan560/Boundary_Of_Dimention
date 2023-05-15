#include "OperationConfig.h"
#include"FrameWork/UsersInput.h"
#include"FrameWork/WinApp.h"

OperationConfig::OperationConfig()
	:KuroEngine::Debugger("OperationConfig", true)
{
	//�e�f�o�C�X�ł̐ݒ�l���J�X�^���p�����[�^�ɒǉ�
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

	//���X�e�B�b�N�̓��͂�ϊ�
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

	//���X�e�B�b�N�̓��͂�ϊ�
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
