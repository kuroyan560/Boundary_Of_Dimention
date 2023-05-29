#pragma once
#include"../StageParts.h"
#include"../../AI/EnemyPatrol.h"
#include"../../AI/EnemySearch.h"
#include"../Grass.h"
#include"ForUser/DrawFunc/BillBoard/DrawFuncBillBoard.h"
#include"EnemyDataReferenceForCircleShadow.h"
#include"../../Graphics/BasicDraw.h"
#include"../../CPUParticle/DashEffect.h"
#include"../../Effect/EnemyEyeEffect.h"
#include"../../AI/EnemyAttack.h"
#include"../../AI/IEnemyAI.h"
#include"Render/RenderObject/ModelInfo/ModelAnimator.h"
#include"../../DebugEnemy.h"
#include"../../AI/EnemyStatus.h"

namespace KuroEngine
{
	class ModelAnimator;
}

//敵の攻撃パターン
enum ENEMY_ATTACK_PATTERN
{
	ENEMY_ATTACK_PATTERN_NORMAL,//プレイヤーを見つけたら攻撃開始
	ENEMY_ATTACK_PATTERN_ALWAYS, //常に一定間隔で攻撃している
	ENEMY_ATTACK_PATTERN_INVALID
};

//敵の射撃パターン
enum ENEMY_BARREL_PATTERN
{
	ENEMY_BARREL_PATTERN_FIXED,		//方向固定
	ENEMY_BARREL_PATTERN_ROCKON,	//プレイヤーに向かって
	ENEMY_BARREL_PATTERN_INVALID
};

class MiniBug :public StageParts, IEnemyAI
{
public:
	MiniBug(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, std::vector<KuroEngine::Vec3<float>>posArray, bool loopFlag)
		:StageParts(MINI_BUG, arg_model, arg_initTransform), m_deadTimer(120), m_eyeEffect(&m_transform), ENEMY_ID(ENEMY_MAX_ID)
	{
		//丸影用に敵のデータの参照を渡す。
		EnemyDataReferenceForCircleShadow::Instance()->SetData(&m_transform, &m_shadowInfluenceRange, &m_deadFlag);
		m_finalizeFlag = false;
		++ENEMY_MAX_ID;

		if (posArray.size() == 0 || posArray.size() == 1)
		{
			std::vector<KuroEngine::Vec3<float>>limitPosArray;
			limitPosArray.emplace_back(arg_initTransform.GetPos());
			m_patrol = std::make_unique<PatrolBasedOnControlPoint>(limitPosArray, 0, loopFlag);
			m_posArray = m_patrol->GetLimitPosArray();
		}
		else
		{
			m_patrol = std::make_unique<PatrolBasedOnControlPoint>(posArray, 0, loopFlag);
		}

		m_animator = std::make_shared<KuroEngine::ModelAnimator>(arg_model);
		OnInit();


		DebugEnemy::Instance()->Stack(m_initializedTransform, ENEMY_MINIBUG);

		m_debugHitBox = std::make_unique<EnemyHitBox>(m_hitBox, KuroEngine::Color(1.0f, 1.0f, 1.0f, 1.0f));
	}

	void OnInit()override;
	void Update(Player &arg_player)override;
	void Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)override;

	void DebugDraw(KuroEngine::Camera &camera);

private:
	std::shared_ptr<KuroEngine::Model>m_enemyModel;
	//デバック用の敵識別ID
	const int ENEMY_ID;
	static int ENEMY_MAX_ID;

	enum Status
	{
		NONE,
		SEARCH,//循環中
		ATTACK,//プレイヤーを追跡
		NOTICE,//プレイヤーらしきものを見つけた
		RETURN,//循環エリアに戻る
		KNOCK_BACK,
		HEAD_ATTACK
	};

	Status m_nowStatus;
	Status m_prevStatus;

	KuroEngine::Vec3<float>m_pos, m_prevPos;
	float m_scale;
	KuroEngine::Vec3<float>m_dir;
	std::unique_ptr<PatrolBasedOnControlPoint> m_patrol;
	SightSearch m_sightArea;
	int m_limitIndex;

	float m_hitBoxSize;

	//丸影用
	float m_shadowInfluenceRange;
	const float SHADOW_INFLUENCE_RANGE = 6.0f;

	//アニメーション
	std::shared_ptr<KuroEngine::ModelAnimator>m_animator;

	class AttackMotion
	{
		KuroEngine::Vec3<float> m_aPointPos;
		KuroEngine::Vec3<float> m_bPointPos;

		HeadNextPoint m_go;
		HeadNextPoint m_back;

		bool m_flag;
		bool m_doneFlag = true;
	public:
		void Init(const KuroEngine::Vec3<float> &aPointPos, const KuroEngine::Vec3<float> &bPointPos, float speed)
		{
			m_aPointPos = aPointPos;
			m_bPointPos = bPointPos;

			m_go.Init(m_aPointPos, m_bPointPos, speed);
			m_back.Init(m_bPointPos, m_aPointPos, speed);
			m_doneFlag = true;
			m_flag = true;
		};

		bool IsDone()
		{
			return m_doneFlag;
		}

		void Done() {
			m_doneFlag = true;
		}
		void UnDone() {
			m_doneFlag = false;
		}

		KuroEngine::Vec3<float>GetVel(const KuroEngine::Vec3<float> &pos)
		{
			if (m_go.IsArrive(pos))
			{
				m_flag = false;
			}
			if (m_back.IsArrive(pos))
			{
				m_flag = true;
				m_doneFlag = true;
			}

			if (m_flag)
			{
				return m_go.Update();
			}
			else
			{
				return m_back.Update();
			}
		};
	};


	//攻撃処理---------------------------------------

	//予備動作
	KuroEngine::Timer m_attackIntervalTimer;
	KuroEngine::Timer m_attackCoolTimer;
	KuroEngine::Timer m_readyToGoToPlayerTimer;
	KuroEngine::Vec3<float>m_attackOffsetVel;

	AttackMotion m_jumpMotion;
	AttackMotion m_attackMotion;



	bool m_attackFlag;
	HeadNextPoint m_trackPlayer;
	KuroEngine::Vec3<float>m_aPointPos;
	KuroEngine::Vec3<float>m_bPointPos;
	TrackEndPoint track;

	//攻撃処理---------------------------------------

	//移動処理---------------------------------------
	KuroEngine::Vec3<float>m_larpPos;
	KuroEngine::Quaternion m_larpRotation;
	//移動処理---------------------------------------

	//死亡処理------------------------------------------------------------------------------

	bool m_deadFlag, m_finalizeFlag;
	bool m_startDeadMotionFlag;
	KuroEngine::Timer m_deadTimer;

	Sphere m_hitBox;

	//死亡処理------------------------------------------------------------------------------



	//思考処理
	KuroEngine::Timer m_thinkTimer;
	bool m_decisionFlag, m_prevDecisionFlag;

	//制御点
	std::vector<KuroEngine::Vec3<float>>m_posArray;


	//演出回り------------------------------------------------------------------------------
	DashEffect m_dashEffect;
	EnemyEyeEffect m_eyeEffect;
	int m_knockBackTime;
	EnemyKnockBack m_knockBack;
	EnemyHeadAttack m_headAttack;

private:

	std::unique_ptr<EnemyHitBox> m_debugHitBox;

	//リアクション表記---------------------------------------

	//3次元ベクトルを2次元に射影する関数
	inline KuroEngine::Vec2<float> Project3Dto2D(KuroEngine::Vec3<float> arg_vector3D, KuroEngine::Vec3<float> arg_basis1, KuroEngine::Vec3<float> arg_basis2) {

		//基底ベクトルを正規化
		arg_basis1.Normalize();
		arg_basis2.Normalize();

		//3次元ベクトルを2次元ベクトルに射影
		float x = arg_vector3D.Dot(arg_basis1);
		float y = arg_vector3D.Dot(arg_basis2);

		return KuroEngine::Vec2<float>(x, y);
	}



	KuroEngine::Quaternion Lerp(const KuroEngine::Quaternion &a, const KuroEngine::Quaternion &b, float mul)
	{
		KuroEngine::Vec4<float> base;
		base.x = a.m128_f32[0];
		base.y = a.m128_f32[1];
		base.z = a.m128_f32[2];
		base.w = a.m128_f32[3];

		KuroEngine::Vec4<float> base2;
		base2.x = b.m128_f32[0];
		base2.y = b.m128_f32[1];
		base2.z = b.m128_f32[2];
		base2.w = b.m128_f32[3];

		KuroEngine::Vec4<float>ease = KuroEngine::Math::Lerp(base, base2, mul);

		KuroEngine::Quaternion result;
		result.m128_f32[0] = ease.x;
		result.m128_f32[1] = ease.y;
		result.m128_f32[2] = ease.z;
		result.m128_f32[3] = ease.w;

		return result;

	};
};

class DossunRing : public StageParts, IEnemyAI
{
public:
	DossunRing(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, ENEMY_ATTACK_PATTERN status);

	void OnInit()override;
	void Update(Player &arg_player)override;
	void Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)override;

	void DebugDraw(KuroEngine::Camera &camera);

private:

	ENEMY_ATTACK_PATTERN m_nowStatus;
	float m_sightRange;


	//攻撃フェーズ---------------
	int m_maxAttackIntervalTime;//予備動作時間
	int m_maxAttackTime;		//攻撃時間

	KuroEngine::Timer m_attackInterval;	//予備動作
	bool m_attackFlag;					//攻撃中
	KuroEngine::Timer m_attackTimer;	//攻撃の広がり

	float m_attackHitBoxRadius;		//攻撃の当たり判定(円)
	float m_attackhitBoxRadiusMax;	//攻撃の当たり判定(最大値)

	std::shared_ptr<KuroEngine::Model>m_attackRingModel;
	KuroEngine::Color m_ringColor;
	//攻撃フェーズ---------------


	Sphere m_hitBox;
	Sphere m_enemyHitBox;
	float m_radius;
	Sphere m_sightHitBox;
	CircleSearch m_sightArea;

	bool m_findPlayerFlag, m_preFindPlayerFlag, m_intervalFlag;

	//死亡処理---------------------------------------
	bool m_deadFlag;
	bool m_startDeadMotionFlag;
	KuroEngine::Timer m_deadTimer;
	float m_deadScale;
	//死亡処理---------------------------------------

	//予備動作---------------------------------------
	KuroEngine::Vec3<float> m_larpScale, m_scale;
	//予備動作---------------------------------------


	//移動処理---------------------------------------
	KuroEngine::Vec3<float>m_larpPos;
	KuroEngine::Quaternion m_larpRotation;
	//移動処理---------------------------------------

	//丸影用
	float m_shadowInfluenceRange;
	const float SHADOW_INFLUENCE_RANGE = 6.0f;


	std::unique_ptr<EnemyHitBox> m_debugHitBox;

	std::shared_ptr<KuroEngine::Model>m_hitBoxModel;

	void SetParam()
	{
		//索敵---------------------------------------
		//視野
		m_sightRange = m_initializedTransform.GetScale().Length() * DebugEnemy::Instance()->GetDossunParam().m_sightRadius;
		//攻撃---------------------------------------
		//プレイヤーと敵の判定
		m_radius = m_initializedTransform.GetScale().Length() * DebugEnemy::Instance()->GetDossunParam().m_hitBoxRadius;
		//円の最大の広がり
		m_attackhitBoxRadiusMax = DebugEnemy::Instance()->GetDossunParam().m_attackHitBoxRadius;
		//円が広がり切るまでの時間
		m_maxAttackIntervalTime = DebugEnemy::Instance()->GetDossunParam().m_attackTime;
		//攻撃のクールタイム
		m_maxAttackTime = DebugEnemy::Instance()->GetDossunParam().m_attackCoolTime;
	}

	void Attack(Player &arg_player);

	//アニメーション
	std::shared_ptr<KuroEngine::ModelAnimator>m_modelAnimator;
};

class Battery : public StageParts, IEnemyAI
{
public:
	Battery(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, std::vector<KuroEngine::Vec3<float>>arg_posArray, float arg_bulletScale, ENEMY_BARREL_PATTERN arg_barrelPattern);
	void OnInit()override;
	void Update(Player &arg_player)override;
	void Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)override;

private:
	KuroEngine::Transform m_initTransform;

	//移動
	KuroEngine::Vec3<float>m_pos;
	std::vector<KuroEngine::Vec3<float>>m_posArray;
	std::unique_ptr<PatrolBasedOnControlPoint> m_patrol;

	//角度
	KuroEngine::Vec3<float>m_upVec;
	KuroEngine::Quaternion m_rotation, m_larpRotation;

	//弾
	KuroEngine::Vec3<float>m_bulletDir;
	float m_bulletScale;
	ENEMY_BARREL_PATTERN m_barrelPattern;
	BulletManager m_bulletManager;

	//判定
	float m_radius;
	Sphere m_hitBox;


	//死亡
	bool m_startDeadMotionFlag;
	bool m_deadFlag;

	bool m_noticeFlag;

};