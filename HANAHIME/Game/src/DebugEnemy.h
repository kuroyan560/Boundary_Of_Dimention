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

struct DossunParameterData
{
	float m_hitBoxRadius;		//プレイヤーと敵の判定
	float m_sightRadius;		//プレイヤーと敵の判定
	float m_attackHitBoxRadius;	//円がどれくらい広がるか
	int m_attackCoolTime;		//攻撃した後のクールタイム
	int m_attackTime;			//円が最大サイズになるまでの時間

	DossunParameterData()
	{
		m_sightRadius = 2.0f;
		m_hitBoxRadius = 1.0f;
		m_attackHitBoxRadius = 5.0f;
		m_attackCoolTime = 60;
		m_attackTime = 60 * 2;
	}
};

class DebugEnemy : public KuroEngine::DesignPattern::Singleton<DebugEnemy>, public KuroEngine::Debugger
{
public:
	DebugEnemy() :m_stageIndex(0), m_index(0), KuroEngine::Debugger("EnemyDebug", true, true)
	{
		m_enemyHitBox.resize(ENEMY_MAX);
		AddCustomParameter("draw_HitBox", { "common", "draw_HitBox" }, PARAM_TYPE::BOOL, &m_isVisualizeEnemyHitBoxFlag, "Common");
		AddCustomParameter("draw_Sight", { "common", "draw_Sight" }, PARAM_TYPE::BOOL, &m_isVisualizeEnemySightFlag, "Common");
		//ドッスン(普通)
		AddCustomParameter("hitBoxRadius", { "dossun_normal", "status","hitBoxRadius" }, PARAM_TYPE::FLOAT, &m_dossunParam.m_hitBoxRadius, "ENEMY_DOSSUN_NORMAL");
		AddCustomParameter("sightRadius", { "dossun_normal", "status", "sightRadius" }, PARAM_TYPE::FLOAT, &m_dossunParam.m_sightRadius, "ENEMY_DOSSUN_NORMAL");
		AddCustomParameter("attackTime", { "dossun_normal", "status","attackTime" }, PARAM_TYPE::INT, &m_dossunParam.m_attackTime, "ENEMY_DOSSUN_NORMAL");
		AddCustomParameter("attackHitBoxRadius", { "dossun_normal", "status","attackHitBoxRadius" }, PARAM_TYPE::FLOAT, &m_dossunParam.m_attackHitBoxRadius, "ENEMY_DOSSUN_NORMAL");
		AddCustomParameter("attackCoolTime", { "dossun_normal", "status","attackCoolTime" }, PARAM_TYPE::INT, &m_dossunParam.m_attackCoolTime, "ENEMY_DOSSUN_NORMAL");
		//ミニバグ
		LoadParameterLog();

		m_enemyHitBox[ENEMY_MINIBUG] = 1.5f;
		m_enemyHitBox[ENEMY_DOSSUN_NORMAL] = 1.0f;
		m_enemyHitBox[ENEMY_DOSSUN_ALLWAYS] = 1.0f;
		m_enemyHitBox[ENEMY_BATTERY_FIXED] = 1.0f;
		m_enemyHitBox[ENEMY_BATTERY_ROCKON] = 1.0f;
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

	void GetEnemyStatus(EnemyType type)
	{
		switch (type)
		{
		case ENEMY_MINIBUG:
			break;
		case ENEMY_DOSSUN_NORMAL:

			break;
		case ENEMY_DOSSUN_ALLWAYS:
			break;
		case ENEMY_BATTERY_FIXED:
			break;
		case ENEMY_BATTERY_ROCKON:
			break;
		case ENEMY_MAX:
			break;
		default:
			break;
		}
	}

	DossunParameterData GetDossunParam()
	{
		return m_dossunParam;
	};


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
		int loopIndex = 0;
		int prevIndex = m_index;
		while (1)
		{
			//敵が見つかったらインデックス探索を止める
			if (type == m_transformArray[m_stageIndex][m_index].type)
			{
				break;
			}
			Increment();

			//何も無かった際の処理
			if (m_transformArray[m_stageIndex].size() <= loopIndex)
			{
				m_index = prevIndex;
				break;
			}
			++loopIndex;
		}
	}

	bool m_isVisualizeEnemyHitBoxFlag, m_isVisualizeEnemySightFlag;
	std::vector<int>m_enemyHP;
	std::vector<float>m_enemyHitBox;

	DossunParameterData m_dossunParam;

};



//当たり判定可視化
class EnemyHitBox
{
public:
	EnemyHitBox(Sphere &transform, KuroEngine::Color &color)
	{
		m_hitBox = transform;
		m_hitBoxModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Shpere.glb");

		m_color = color;
		m_color.m_a = 0.5f;
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
			edgeColor,
			m_color
		);
	}

private:
	Sphere m_hitBox;
	std::shared_ptr<KuroEngine::Model>m_hitBoxModel;
	KuroEngine::Color m_color;

};