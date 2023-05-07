#pragma once
#include"../StageParts.h"
#include"../../AI/EnemyPatrol.h"
#include"../../AI/EnemySearch.h"
#include"../Grass.h"

class MiniBug :public StageParts
{
public:
	MiniBug(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, StageParts *arg_parent, std::vector<KuroEngine::Vec3<float>>posArray)
		:StageParts(MINI_BUG, arg_model, arg_initTransform, arg_parent), m_patrol(posArray, 0)
	{
		m_nowStatus = SERACH;
		m_prevStatus = NONE;
		m_sightArea.Init(&m_transform);
		track.Init(0.01f);
		m_posArray = posArray;
		m_limitIndex = 0;
	}
	void Update(Player &arg_player)override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	void DebugDraw(KuroEngine::Camera &camera);

private:
	std::shared_ptr<KuroEngine::Model>m_enemyModel;

	enum Status
	{
		NONE,
		SERACH,//zŠÂ’†
		ATTACK,//ƒvƒŒƒCƒ„[‚ğ’ÇÕ
		NOTICE,//ƒvƒŒƒCƒ„[‚ç‚µ‚«‚à‚Ì‚ğŒ©‚Â‚¯‚½
		RETURN,//zŠÂƒGƒŠƒA‚É–ß‚é
	};

	Status m_nowStatus;
	Status m_prevStatus;

	KuroEngine::Vec3<float>m_pos,m_prevPos;
	KuroEngine::Vec3<float>m_dir;
	PatrolBasedOnControlPoint m_patrol;
	SightSearch m_sightArea;
	int m_limitIndex;


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
			if(m_back.IsArrive(pos))
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


	//UŒ‚ˆ—---------------------------------------

	//—\”õ“®ì
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

	//UŒ‚ˆ—---------------------------------------

	//ˆÚ“®ˆ—---------------------------------------
	KuroEngine::Vec3<float>m_larpPos;
	KuroEngine::Quaternion m_larpRotation;
	//ˆÚ“®ˆ—---------------------------------------


	//vlˆ—
	KuroEngine::Timer m_thinkTimer;
	bool m_decisionFlag, m_prevDecisionFlag;

	//§Œä“_
	std::vector<KuroEngine::Vec3<float>>m_posArray;


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

class DossunRing : public StageParts
{
public:
	enum Status
	{
		NORMAL,//ƒvƒŒƒCƒ„[‚ğŒ©‚Â‚¯‚½‚çUŒ‚ŠJn
		ALWAYS //í‚Éˆê’èŠÔŠu‚ÅUŒ‚‚µ‚Ä‚¢‚é
	};

	DossunRing(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, StageParts *arg_parent, Status status)
		:StageParts(DOSSUN_RING, arg_model, arg_initTransform, arg_parent)
	{
		m_hitBoxRadiusMax = 10.0f;
		m_hitBoxRadius = 0.0f;
		m_findPlayerFlag = false;
		m_nowStatus = status;

		//‹ŠE‚Ì”»’è---------------------------------------
		m_sightHitBox.m_centerPos = &m_transform.GetPos();
		m_sightHitBox.m_radius = &m_hitBoxRadiusMax;
		m_sightArea.Init(m_sightHitBox);
		//‹ŠE‚Ì”»’è---------------------------------------

		m_maxAttackIntervalTime = 60 * 2;
		m_maxAttackTime = 60 * 5;

		m_attackInterval.Reset(m_maxAttackIntervalTime);
		m_attackTimer.Reset(m_maxAttackTime);
	}
	void Update(Player &arg_player)override;

	void DebugDraw(KuroEngine::Camera &camera);

private:

	Status m_nowStatus;

	//UŒ‚ƒtƒF[ƒY---------------
	int m_maxAttackIntervalTime;//—\”õ“®ìŠÔ
	int m_maxAttackTime;		//UŒ‚ŠÔ

	KuroEngine::Timer m_attackInterval;	//—\”õ“®ì
	bool m_attackFlag;					//UŒ‚’†
	KuroEngine::Timer m_attackTimer;	//UŒ‚‚ÌL‚ª‚è

	float m_hitBoxRadius;		//UŒ‚‚Ì“–‚½‚è”»’è(‰~)
	float m_hitBoxRadiusMax;	//UŒ‚‚Ì“–‚½‚è”»’è(Å‘å’l)
	//UŒ‚ƒtƒF[ƒY---------------

	Sphere m_sightHitBox;
	CircleSearch m_sightArea;

	bool m_findPlayerFlag;

};