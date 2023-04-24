#include "Tutorial.h"

Tutorial::Tutorial(std::shared_ptr<KuroEngine::RWStructuredBuffer> particleBuffer) :m_fireFly(particleBuffer), m_changeTimer(120.0f)
{
	m_tutorialDataArray.emplace_back(TutorialData({ -50.0f, 30.0f, 40.0f }, 120.0f, "resource/user/tex/Number.png", 10, { 10,1 }));
	m_tutorialDataArray.emplace_back(TutorialData({ -30.0f, 30.0f, 40.0f }, 120.0f, "resource/user/tex/Number.png", 10, { 10,1 }));
	//m_tutorialDataArray.emplace_back(TutorialData({ 20.0f, 0.0f, 10.0f }, 30.0f, "resource/user/tex/L_stick.png", 3, { 3,1 }));	
	m_fireFly.GoThisPos(m_tutorialDataArray[0].m_pos);

	m_texPosIndex = 0;
	m_nowIndex = 0;

	m_nowScale = { 0.0f,0.0f,0.0f };
}

void Tutorial::Update()
{
	//目標地点に到着したら吹き出しを入れ替える
	if (m_nowIndex != m_prevNowIndex)
	{
		m_changeTimer.Reset();
		m_changeUIFlag = true;
		m_fireFly.GoThisPos(m_tutorialDataArray[m_nowIndex].m_pos);
		m_prevNowIndex = m_nowIndex;
	}

	//切り替え時間が終わったらUI再度登場
	if (m_changeTimer.IsTimeUp())
	{
		m_texPosIndex = m_nowIndex;
		m_changeUIFlag = false;
	}

	//UI消滅
	if (m_changeUIFlag)
	{
		m_nowScale = KuroEngine::Math::Lerp(m_nowScale, { 0.0f,0.0f,0.0f }, 0.1f);
	}
	//UI登場
	else
	{
		m_nowScale = KuroEngine::Math::Lerp(m_nowScale, { 5.0f,5.0f,5.0f }, 0.1f);
	}


	//一定以上までいったら消滅


	m_nowTransform.SetPos(m_tutorialDataArray[m_texPosIndex].m_pos + KuroEngine::Vec3<float>(5.0f, 5.0f, 0.0f));
	m_nowTransform.SetScale(m_nowScale);
	m_changeTimer.UpdateTimer();

	m_fireFly.Update();

}

void Tutorial::Draw(KuroEngine::Camera &camera)
{
	KuroEngine::DrawFunc3D::DrawNonShadingPlane(m_tutorialDataArray[m_nowIndex].GetTex(), m_nowTransform, camera, m_fireFly.GetAlphaRate());
}

void Tutorial::Finish()
{
	m_fireFly.Finish();
}

void Tutorial::Next()
{
	++m_nowIndex;
	if (m_tutorialDataArray.size() <= m_nowIndex)
	{
		m_nowIndex = 0;
	}
}
