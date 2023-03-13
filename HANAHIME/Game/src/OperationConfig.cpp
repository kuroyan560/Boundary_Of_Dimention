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
    auto mouseMove = UsersInput::Instance()->GetMouseMove();
    ImGui::BeginChild(ImGui::GetID((void*)0), ImVec2(250, 150));
    ImGui::Text("inputX : %d", mouseMove.m_inputX);
    ImGui::Text("inputY : %d", mouseMove.m_inputY);
    ImGui::Text("inputZ : %d", mouseMove.m_inputZ);
    ImGui::EndChild();

}

KuroEngine::Vec3<float> OperationConfig::GetMove(float arg_moveScalar)
{
    using namespace KuroEngine;

    if (m_controller)
    {
        //���X�e�B�b�N�̓��͂�ϊ�
        auto input = UsersInput::Instance()->GetLeftStickVec(0);
        return Vec3<float>(input.x, 0.0f, input.y) * arg_moveScalar;
    }

    //WASD����
    Vec3<float>result = { 0,0,0 };
    if (UsersInput::Instance()->KeyInput(DIK_W))result.z += arg_moveScalar;
    if (UsersInput::Instance()->KeyInput(DIK_S))result.z -= arg_moveScalar;
    if (UsersInput::Instance()->KeyInput(DIK_D))result.x += arg_moveScalar;
    if (UsersInput::Instance()->KeyInput(DIK_A))result.x -= arg_moveScalar;
    return result;
}

KuroEngine::Vec3<KuroEngine::Angle> OperationConfig::GetScopeMove(float arg_sensitivity)
{
    using namespace KuroEngine;

    if (m_controller)
    {
        //�E�X�e�B�b�N�̓��͂�ϊ�
        auto input = UsersInput::Instance()->GetRightStickVec(0);
        return Vec3<Angle>(input.x * arg_sensitivity, -input.y * arg_sensitivity, 0.0f);
    }

    //�}�E�X����
    auto mouseMove = UsersInput::Instance()->GetMouseMove();
    //�E�B���h�E�T�C�Y�ɂ���đ��ΓI�ȃX�P�[���ɍ��킹��
    const auto scale = Vec2<float>(1.0f, 1.0f) / WinApp::Instance()->GetExpandWinSize();
    return Vec3<Angle>(mouseMove.m_inputY * scale.x * arg_sensitivity,
        mouseMove.m_inputX * scale.y * arg_sensitivity,
        0.0f);
}