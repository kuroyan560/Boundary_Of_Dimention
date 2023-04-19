#include "Switch.h"
#include"StageParts.h"

bool Switch::IsBooting() const
{
    for (auto& lever : m_leverArray)
    {
        //1�ł����o�[���I�t�Ȃ�N�����Ă��Ȃ�
        if (!lever.lock()->GetLeverFlg())return false;
    }
    return true;
}
