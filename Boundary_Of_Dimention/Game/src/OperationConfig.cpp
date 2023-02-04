#include "OperationConfig.h"
#include"FrameWork/UsersInput.h"
#include"FrameWork/WinApp.h"

OperationConfig::OperationConfig()
{
}

KuroEngine::Vec3<float> OperationConfig::GetMove(float arg_moveScalar)
{
    using namespace KuroEngine;

    if (m_controller)
    {
        //���X�e�B�b�N�̓��͂�3���ɕϊ�
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