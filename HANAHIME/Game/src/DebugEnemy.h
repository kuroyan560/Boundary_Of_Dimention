#pragma once
#include"Common/Singleton.h"
#include"Common/Transform.h"

enum EnemyType
{
	ENEMY_MINIBUG,
	ENEMY_DOSSUN_NORMAL,
	ENEMY_DOSSUN_ALLWAYS,
	ENEMY_BATTERY_FIXED,
	ENEMY_BATTERY_ROCKON,
	ENEMY_MAX,
};

class DebugEnemy : public KuroEngine::DesignPattern::Singleton<DebugEnemy>
{
public:
	DebugEnemy() :m_stageIndex(0), m_index(0)
	{

	}


	KuroEngine::Transform GetTransform(int enmeyIndex);
	KuroEngine::Transform GetTransform();
	KuroEngine::Transform GetSpecificTransform(EnemyType type);
	void SetStageNum(int stageIndex)
	{
		m_stageIndex = stageIndex;
	}

	void StackStage();
	void Stack(const KuroEngine::Transform &transform, EnemyType type);

private:

	struct EnemyData
	{
		KuroEngine::Transform m_transform;
		EnemyType type;
	};
	std::vector<std::vector<EnemyData>>m_transformArray;

	int m_index, m_stageIndex;
	void Increment()
	{
		++m_index;
		if (m_transformArray[m_stageIndex].size() <= m_index)
		{
			m_index = 0;
		}
	};

	//指定した敵の種類までインデックスをスキップする。
	void FindEnemy(EnemyType type)
	{
		while (1)
		{
			//敵が見つかったらインデックス探索を止める
			if (type == ENEMY_MINIBUG)
			{
				break;
			}
			Increment();
		}
	}

};
