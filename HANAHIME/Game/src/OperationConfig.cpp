#include "OperationConfig.h"
#include"FrameWork/UsersInput.h"
#include"FrameWork/WinApp.h"

OperationConfig::OperationConfig()
    :KuroEngine::Debugger("OperationConfig", true)
{
    for (int i = 0; i < INPUT_DEVICE::NUM; ++i)
    {
        auto deviceName = m_inputDeviceNames[i];
        AddCustomParameter("CameraSensitivity", { deviceName,"CameraSensitivity" },
            PARAM_TYPE::FLOAT, &m_params[i].m_camSensitivity, deviceName);
    }
}

void OperationConfig::OnImguiItems()
{
    using namespace KuroEngine;

    int deviceIdx = m_nowInputDevice;
    m_nowInputDevice = (INPUT_DEVICE)ImguiApp::WrappedCombo("InputDevice", m_inputDeviceNames.data(), INPUT_DEVICE::NUM, deviceIdx);

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
        //左スティックの入力を変換
        auto input = UsersInput::Instance()->GetLeftStickVec(0);
        result = Vec3<float>(input.x, 0.0f, -input.y);
        break;
    }

    //入力があったら
    if (!result.IsZero())
    {
        //一応正規化
        result.Normalize();
        //回転を適用
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
            //マウス入力
            auto mouseMove = UsersInput::Instance()->GetMouseMove();
            //ウィンドウサイズによって相対的なスケールに合わせる
            const auto scale = Vec2<float>(1.0f, 1.0f) / WinApp::Instance()->GetExpandWinSize();
            return Vec3<Angle>(mouseMove.m_inputX * scale.x * sensitivity,
                mouseMove.m_inputY * scale.y * sensitivity,
                0.0f);
        }

        case CONTROLLER:
        {
            //右スティックの入力を変換
            auto input = UsersInput::Instance()->GetRightStickVec(0);
            return Vec3<Angle>(input.x * sensitivity, -input.y * sensitivity, 0.0f);
        }
    }

    AppearMessageBox("OperationConfig : GetScopeMove()失敗", "入力デバイスがおかしいみたい");
    exit(EXIT_FAILURE);
}