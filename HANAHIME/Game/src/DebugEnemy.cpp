#include"DebugEnemy.h"

KuroEngine::Transform DebugEnemy::GetTransform(int enmeyIndex)
{
	return m_transformArray[m_stageIndex][enmeyIndex].m_transform;
}

KuroEngine::Transform DebugEnemy::GetTransform()
{
	KuroEngine::Transform getTransform(m_transformArray[m_stageIndex][m_index].m_transform);
	Increment();
	return getTransform;
}

void DebugEnemy::Stack(const KuroEngine::Transform &transform, EnemyType type)
{
	m_transformArray.back().emplace_back(transform, type);
}

void DebugEnemy::StackStage()
{
	m_transformArray.emplace_back();
}


KuroEngine::Transform DebugEnemy::GetSpecificTransform(EnemyType type)
{
	FindEnemy(type);
	return m_transformArray[m_stageIndex][m_index].m_transform;
}