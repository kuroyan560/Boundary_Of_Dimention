#include"GuideInsect.h"

GuideInsect::GuideInsect(std::shared_ptr<KuroEngine::RWStructuredBuffer>particle_buffer) :m_stageIndex(0), m_index(0), m_guideInsect(particle_buffer), m_goToCheckPointFlag(false)
{
	for (auto &stage : m_checkPointArray)
	{
		stage->m_isHitFlag = false;
	}

	m_tiemr.Reset(60 * 10);
}

void GuideInsect::Init()
{
	m_index = 0;
	m_goToCheckPointFlag = false;
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
		//GoToCheckPoint(m_checkPointArray[m_index]->m_pos);
	}
	m_guideInsect.Update();

	if (m_tiemr.UpdateTimer())
	{
		m_goToCheckPointFlag = false;
	}

	if (m_guideInsect.GetAlphaRate() <= 0.0f)
	{
		m_goToCheckPointFlag = false;
	}
}

void GuideInsect::Draw()
{
}

std::shared_ptr<GuideInsect::CheckPointData> GuideInsect::Stack()
{
	m_checkPointArray.emplace_back(std::make_shared<GuideInsect::CheckPointData>());
	return m_checkPointArray.back();
}


struct IndexData
{
	int index;
	float distance;
};

int int_cmpr(const void *a, const void *b)
{
	IndexData* lAHandle = (IndexData*)a, *lBHandle = (IndexData *)b;

	if (lAHandle->distance < lBHandle->distance)
	{
		return 1;
	}
	else if (lBHandle->distance < lAHandle->distance)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

void GuideInsect::GoToCheckPoint(const KuroEngine::Vec3<float> &pos)
{
	if (!m_goToCheckPointFlag)
	{
		m_tiemr.Reset(60 * 10);
		m_guideInsect.Init(pos);

		//接触していない&&最も近いチェックポイントに飛ばすようにする
		std::vector<IndexData>distanceArray;
		for (int i = 0; i < m_checkPointArray.size(); ++i)
		{
			if (m_checkPointArray[i]->m_isHitFlag)
			{
				continue;
			}

			distanceArray.emplace_back();
			distanceArray.back().distance = pos.Distance(m_checkPointArray[i]->m_pos);
			distanceArray.back().index = i;
		}
		std::qsort(distanceArray.data(), distanceArray.size(), sizeof(IndexData), int_cmpr);

		//接近命令
		m_guideInsect.GoThisPos(pos, m_checkPointArray[distanceArray.back().index]->m_pos);
		m_goToCheckPointFlag = true;
	}
	else
	{
		m_guideInsect.Finish();
	}
}