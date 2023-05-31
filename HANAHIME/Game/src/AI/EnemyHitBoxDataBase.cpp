#include"EnemyHitBoxDataBase.h"


EnemyHitBoxDataBase::EnemyHitBoxDataBase()
{
}

void EnemyHitBoxDataBase::Stack(Sphere *sphere)
{
	m_enemyHitBox.back().emplace_back(sphere);
}

void EnemyHitBoxDataBase::StackStage()
{
	m_enemyHitBox.emplace_back();
}

KuroEngine::Vec3<float> EnemyHitBoxDataBase::Update(const Sphere &enemyHitBox)
{
	KuroEngine::Vec3<float>vel = {};
	int stageIndex = StageManager::Instance()->GetNowStageIdx();
	for (int i = 0; i < m_enemyHitBox[stageIndex].size(); ++i)
	{
		vel += Collision::Instance()->PushBackEnemy(*m_enemyHitBox[stageIndex][i], enemyHitBox);
	}
	return vel;
}