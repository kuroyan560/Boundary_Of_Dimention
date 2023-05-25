#include"GuideInsect.h"

GuideInsect::GuideInsect(std::shared_ptr<KuroEngine::RWStructuredBuffer>particle_buffer) :m_stageIndex(0), m_index(0), m_guideInsect(particle_buffer)
{
	for (auto &stage : m_checkPointArray)
	{
		stage->m_isHitFlag = false;
	}
}

void GuideInsect::Init()
{
	m_index = 0;

	for (auto &stage : m_checkPointArray)
	{
		stage->m_isHitFlag = false;
	}
}

void GuideInsect::Update()
{
	if (m_checkPointArray.size() == 0 ||
		m_checkPointArray.size() <= m_index)
	{
		return;
	}
	//チェックポイントに触れた
	if (m_checkPointArray[m_index]->m_isHitFlag)
	{
		++m_index;
	}
	m_guideInsect.Update();
}

void GuideInsect::Draw()
{
}

std::shared_ptr<GuideInsect::CheckPointData> GuideInsect::Stack()
{
	m_checkPointArray.emplace_back(std::make_shared<GuideInsect::CheckPointData>());
	m_guideInsect.GoThisPos(m_checkPointArray[0]->m_pos);
	return m_checkPointArray.back();
}

void GuideInsect::GoToCheckPoint(const KuroEngine::Vec3<float> &pos)
{
	m_guideInsect.GoThisPos(pos);
}