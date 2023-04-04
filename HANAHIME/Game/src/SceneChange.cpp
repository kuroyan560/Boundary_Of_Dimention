#include "SceneChange.h"
#include"../../../src/engine/ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"../../../src/engine/DirectX12/D3D12App.h"

SceneChange::SceneChange() :
	m_time(60),
	m_startFlag(false), m_blackOutFlag(false),
	m_countTimeUpNum(0),
	m_blackTexBuff(KuroEngine::D3D12App::Instance()->GenerateTextureBuffer(KuroEngine::Color(255, 255, 255, 255))),
	m_alpha(0.0f)
{
}

void SceneChange::Update()
{
	if (!m_startFlag)
	{
		return;
	}

	//�Ó]��
	if (m_countTimeUpNum == 0)
	{
		m_alpha = 255.0f * m_time.GetTimeRate();
		m_blackOutFlag = m_time.IsTimeUp();
	}
	//���]��
	else
	{
		m_blackOutFlag = false;
		m_alpha = 255.0f * m_time.GetInverseTimeRate();
	}
	m_alpha = std::clamp(m_alpha, 0.0f, 255.0f);

	//�Ó]�����u��
	if (m_blackOutFlag)
	{
		++m_countTimeUpNum;
		m_time.Reset();
	}
	//���]�����u��
	if (1 <= m_countTimeUpNum && m_time.IsTimeUp())
	{
		m_startFlag = false;
		m_countTimeUpNum = 0;
		m_time.Reset();
	}


	m_time.UpdateTimer();
}

void SceneChange::Draw()
{
	if (!m_startFlag)
	{
		//return;
	}
	KuroEngine::DrawFunc2D::DrawGraph({0.0f,0.0f}, m_blackTexBuff, 1.0f);
}

void SceneChange::Start()
{
	m_startFlag = true;
}

bool SceneChange::IsHide()
{
	return m_blackOutFlag;
}
