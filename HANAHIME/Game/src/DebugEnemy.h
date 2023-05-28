#pragma once
#include"Common/Singleton.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"
#include"Graphics/BasicDraw.h"
#include"FrameWork/Importer.h"
#include"AI/EnemySearch.h"

enum EnemyType
{
	ENEMY_MINIBUG,
	ENEMY_DOSSUN_NORMAL,
	ENEMY_DOSSUN_ALLWAYS,
	ENEMY_BATTERY_FIXED,
	ENEMY_BATTERY_ROCKON,
	ENEMY_MAX
};

class DebugEnemy : public KuroEngine::DesignPattern::Singleton<DebugEnemy>, public KuroEngine::Debugger
{
public:
	DebugEnemy() :m_stageIndex(0), m_index(0), KuroEngine::Debugger("EnemyDebug", true, true)
	{
		m_enemyHitBox.resize(ENEMY_MAX);
		AddCustomParameter("EnemyHitBox", { "enemy", "common" }, PARAM_TYPE::BOOL, &m_isVisualizeEnemyHitBoxFlag, "Common");
		AddCustomParameter("Sight", { "enemy", "common" }, PARAM_TYPE::BOOL, &m_isVisualizeEnemySightFlag, "Common");
		AddCustomParameter("ENEMY_MINIBUG_HITBOX", { "enemy", "hitBox" }, PARAM_TYPE::FLOAT, &m_enemyHitBox[ENEMY_MINIBUG], "EnemyHitBox");
		AddCustomParameter("ENEMY_DOSSUN_NORMAL_HITBOX", { "enemy", "hitBox" }, PARAM_TYPE::FLOAT, &m_enemyHitBox[ENEMY_DOSSUN_NORMAL], "EnemyHitBox");
		AddCustomParameter("ENEMY_DOSSUN_ALLWAYS_HITBOX", { "enemy", "hitBox" }, PARAM_TYPE::FLOAT, &m_enemyHitBox[ENEMY_DOSSUN_ALLWAYS], "EnemyHitBox");
		AddCustomParameter("ENEMY_BATTERY_FIXED_HITBOX", { "enemy", "hitBox" }, PARAM_TYPE::FLOAT, &m_enemyHitBox[ENEMY_BATTERY_FIXED], "EnemyHitBox");
		AddCustomParameter("ENEMY_BATTERY_ROCKON_HITBOX", { "enemy", "hitBox" }, PARAM_TYPE::FLOAT, &m_enemyHitBox[ENEMY_BATTERY_ROCKON], "EnemyHitBox");

		//LoadParameterLog();
		m_enemyHitBox[ENEMY_MINIBUG] = 1.5f;
		m_enemyHitBox[ENEMY_DOSSUN_NORMAL] = 1.0f;
		m_enemyHitBox[ENEMY_DOSSUN_ALLWAYS] = 1.0f;
		m_enemyHitBox[ENEMY_BATTERY_FIXED] = 1.0f;
		m_enemyHitBox[ENEMY_BATTERY_ROCKON] = 1.0f;
	}

	~DebugEnemy()
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


	//デバックに敵に反映するもの達---------------------------------------

	float HitBox(EnemyType type)
	{
		return m_enemyHitBox[type];
	}

	bool VisualizeEnemyHitBox()
	{
		return m_isVisualizeEnemyHitBoxFlag;
	}
	bool VisualizeEnemySight()
	{
		return m_isVisualizeEnemySightFlag;
	}
	//デバックに敵に反映するもの達---------------------------------------


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

	bool m_isVisualizeEnemyHitBoxFlag, m_isVisualizeEnemySightFlag;
	std::vector<int>m_enemyHP;
	std::vector<float>m_enemyHitBox;

};



//当たり判定可視化
class EnemyHitBox
{
public:
	EnemyHitBox(Sphere &transform)
	{
		m_hitBox = transform;
		m_hitBoxModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Shpere.glb");
	}
	void Draw(KuroEngine::Camera &camera, KuroEngine::LightManager &light)
	{
		IndividualDrawParameter edgeColor = IndividualDrawParameter::GetDefault();
		edgeColor.m_edgeColor = KuroEngine::Color(0.54f, 0.14f, 0.33f, 0.3f);
		KuroEngine::Transform m_transform;
		m_transform.SetPos(*(m_hitBox.m_centerPos));
		m_transform.SetScale(*(m_hitBox.m_radius));
		BasicDraw::Instance()->Draw_NoGrass
		(
			camera,
			light,
			m_hitBoxModel,
			m_transform,
			edgeColor
		);
	}

private:
	Sphere m_hitBox;
	std::shared_ptr<KuroEngine::Model>m_hitBoxModel;


};