#include "OperationConfig.h"
#include"FrameWork/UsersInput.h"
#include"FrameWork/WinApp.h"

OperationConfig::OperationConfig()
    :KuroEngine::Debugger("OperationConfig", true)
{
}

void OperationConfig::OnImguiItems()
{
    using namespace KuroEngine;

    static std::array<std::string, INPUT_DEVICE::NUM>s_inputDeviceNames = { "KEY_BOARD_MOUSE","CONTROLLER" };
    int deviceIdx = m_nowInputDevice;
    m_nowInputDevice = (INPUT_DEVICE)ImguiApp::WrappedCombo("InputDevice", s_inputDeviceNames.data(), INPUT_DEVICE::NUM, deviceIdx);

    auto mouseMove = UsersInput::Instance()->GetMouseMove();
    ImGui::BeginChild(ImGui::GetID((void*)0), ImVec2(250, 150));
    ImGui::Text("inputX : %d", mouseMove.m_inputX);
    ImGui::Text("inputY : %d", mouseMove.m_inputY);
    ImGui::Text("inputZ : %d", mouseMove.m_inputZ);
    ImGui::EndChild();

}

KuroEngine::Vec3<float> OperationConfig::GetMoveVec(KuroEngine::Quaternion arg_rotate)
{
    using namespace KuroEngine;

    Vec3<float>result;

    switch (m_nowInputDevice)
    {
    case KEY_BOARD_MOUSE:
        if (UsersInput::Instance()->KeyInput(DIK_W))result.z += 1.0f;
        if (UsersInput::Instance()->KeyInput(DIK_S))result.z -= 1.0f;
        if (UsersInput::Instance()->KeyInput(DIK_D))result.x += 1.0f;
        if (UsersInput::Instance()->KeyInput(DIK_A))result.x -= 1.0f;
        break;

    case CONTROLLER:
        //���X�e�B�b�N�̓��͂�ϊ�
        auto input = UsersInput::Instance()->GetLeftStickVec(0);
        result = Vec3<float>(input.x, 0.0f, -input.y);
        break;
    }

    //���͂���������
    if (!result.IsZero())
    {
        //�ꉞ���K��
        result.Normalize();
        //��]��K�p
        result = KuroEngine::Math::TransformVec3(result, arg_rotate);
    }
    return result;
}

KuroEngine::Vec3<KuroEngine::Angle> OperationConfig::GetScopeMove()
{
    using namespace KuroEngine;

    float sensitivity = m_params[m_nowInputDevice].m_camSensitivity;

    switch (m_nowInputDevice)
    {
        case KEY_BOARD_MOUSE:
        {
            //�}�E�X����
            auto mouseMove = UsersInput::Instance()->GetMouseMove();
            //�E�B���h�E�T�C�Y�ɂ���đ��ΓI�ȃX�P�[���ɍ��킹��
            const auto scale = Vec2<float>(1.0f, 1.0f) / WinApp::Instance()->GetExpandWinSize();
            return Vec3<Angle>(mouseMove.m_inputY * scale.x * sensitivity,
                mouseMove.m_inputX * scale.y * sensitivity,
                0.0f);
        }

        case CONTROLLER:
        {
            //�E�X�e�B�b�N�̓��͂�ϊ�
            auto input = UsersInput::Instance()->GetRightStickVec(0);
            return Vec3<Angle>(input.x * sensitivity, -input.y * sensitivity, 0.0f);
        }
    }

    AppearMessageBox("OperationConfig : GetScopeMove()���s", "���̓f�o�C�X�����������݂���");
    exit(EXIT_FAILURE);
}