#include "GateManager.h"
#include"StageManager.h"
#include"Stage.h"

void GateManager::FrameEnd()
{
	m_nonTouch = (m_touchCount == 0);
	m_touchCount = 0;
}

void GateManager::SetEnter(bool arg_enter, int arg_destStageNum, int arg_destGateID)
{
	if (arg_enter)m_touchCount++;

	if (!m_nonTouch)return;
	if (!arg_enter)return;

	m_enterGate = arg_enter;
	m_destStageNum = arg_destStageNum;
	m_destGateID = arg_destGateID;
}