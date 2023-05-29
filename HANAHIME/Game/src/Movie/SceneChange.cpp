#include"SceneChange.h"
#include"ForUser/DrawFunc/2D/DrawFunc2D.h"
#include"DirectX12/D3D12App.h"
#include"FrameWork/WinApp.h"
#include"../OperationConfig.h"

SceneChange::SceneChange() :
	SCENE_CHANGE_TIME(60),
	m_time(SCENE_CHANGE_TIME),
	m_startFlag(false), m_blackOutFlag(false),
	m_countTimeUpNum(0),
	m_blackTexBuff(KuroEngine::D3D12App::Instance()->GenerateTextureBuffer(KuroEngine::Color(0, 0, 0, 255))),
	m_alpha(0.0f),
	m_size(KuroEngine::WinApp::Instance()->GetWinSize().Float())
{
}

void SceneChange::Update()
{
	if (!m_startFlag)
	{
		return;
	}

	//à√ì]íÜ
	if (m_countTimeUpNum == 0)
	{
		m_alpha = m_time.GetTimeRate();
		m_blackOutFlag = m_time.IsTimeUp();
	}
	//ñæì]íÜ
	else
	{
		m_blackOutFlag = false;
		m_alpha = m_time.GetInverseTimeRate();
	}
	m_alpha = std::clamp(m_alpha, 0.0f, 255.0f);

	//à√ì]ÇµÇΩèuä‘
	if (m_blackOutFlag)
	{
		++m_countTimeUpNum;
		m_alphaOffset = 0.0f;
		m_time.Reset();
	}
	//ñæì]ÇµÇΩèuä‘
	if (1 <= m_countTimeUpNum && m_time.IsTimeUp())
	{
		m_startFlag = false;
		m_countTimeUpNum = 0;
		m_time.Reset();

		//ì¸óÕÇéÛÇØïtÇØÇÈÇÊÇ§Ç…
		OperationConfig::Instance()->SetAllInputActive(true);
	}


	m_time.UpdateTimer();
}

void SceneChange::Draw()
{
	if (!m_startFlag)
	{
		return;
	}
	KuroEngine::DrawFunc2D::DrawRotaGraph2D({ m_size.x / 2.0f,m_size.y / 2.0f }, m_size, 0.0f, m_blackTexBuff, m_alpha + m_alphaOffset);
}

void SceneChange::Start()
{
	if (m_startFlag)
	{
		//m_countTimeUpNum = 0;
		//m_alphaOffset = m_time.GetInverseTimeRate();
		//m_time.Reset(m_time.GetLeftTime());
	}
	else
	{
		m_startFlag = true;
		m_alphaOffset = 0.0f;
		m_time.Reset(SCENE_CHANGE_TIME);
	}

	//ì¸óÕÇéÛÇØïtÇØÇ»Ç¢ÇÊÇ§Ç…
	OperationConfig::Instance()->SetAllInputActive(false);
}

bool SceneChange::IsHide()
{
	return m_blackOutFlag;
}
