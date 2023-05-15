#include "GateManager.h"
#include"StageManager.h"
#include"Stage.h"

void GateManager::SetEnter(bool arg_enter, int arg_destStageNum, int arg_destGateID)
{
	if (!arg_enter)return;

	m_enterGate = arg_enter;
	m_destStageNum = arg_destStageNum;
	m_destGateID = arg_destGateID;
}