#pragma once
#include"EnemySearch.h"
#include"Common/Singleton.h"
#include"../Stage/StageManager.h"

class EnemyHitBoxDataBase : public KuroEngine::DesignPattern::Singleton<EnemyHitBoxDataBase>
{
public:
	EnemyHitBoxDataBase();

	void Stack(Sphere *sphere);
	void StackStage();
	KuroEngine::Vec3<float> Update(const Sphere &enemyHitBox);

private:
	std::vector<std::vector<Sphere *>>m_enemyHitBox;
};
