#include "Switch.h"
#include"StageParts.h"

bool Switch::IsBooting() const
{
    for (auto& lever : m_leverArray)
    {
        //1つでもレバーがオフなら起動していない
        if (!lever.lock()->GetLeverFlg())return false;
    }
    return true;
}
