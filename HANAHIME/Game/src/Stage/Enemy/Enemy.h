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

//�G�̍U���p�^�[��
enum ENEMY_ATTACK_PATTERN
{
	ENEMY_ATTACK_PATTERN_NORMAL,//�v���C���[����������U���J�n
	ENEMY_ATTACK_PATTERN_ALWAYS, //��Ɉ��Ԋu�ōU�����Ă���
	ENEMY_ATTACK_PATTERN_INVALID
};

//�G�̎ˌ��p�^�[��
enum ENEMY_BARREL_PATTERN
{
	ENEMY_BARREL_PATTERN_FIXED,		//�����Œ�
	ENEMY_BARREL_PATTERN_ROCKON,	//�v���C���[�Ɍ�������
	ENEMY_BARREL_PATTERN_INVALID
};

class MiniBug :public StageParts, IEnemyAI
{
public:
	MiniBug(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, std::vector<KuroEngine::Vec3<float>>posArray, bool loopFlag)
		:StageParts(MINI_BUG, arg_model, arg_initTransform), m_deadTimer(120), m_eyeEffect(&m_transform), ENEMY_ID(ENEMY_MAX_ID)
	{
		//�ۉe�p�ɓG�̃f�[�^�̎Q�Ƃ�n���B
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
	//�f�o�b�N�p�̓G����ID
	const int ENEMY_ID;
	static int ENEMY_MAX_ID;

	enum Status
	{
		NONE,
		SEARCH,//�z��
		ATTACK,//�v���C���[��ǐ�
		NOTICE,//�v���C���[�炵�����̂�������
		RETURN,//�z�G���A�ɖ߂�
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

	//�ۉe�p
	float m_shadowInfluenceRange;
	const float SHADOW_INFLUENCE_RANGE = 6.0f;

	//�A�j���[�V����
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


	//�U������---------------------------------------

	//�\������
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

	//�U������---------------------------------------

	//�ړ�����---------------------------------------
	KuroEngine::Vec3<float>m_larpPos;
	KuroEngine::Quaternion m_larpRotation;
	//�ړ�����---------------------------------------

	//���S����------------------------------------------------------------------------------

	bool m_deadFlag, m_finalizeFlag;
	bool m_startDeadMotionFlag;
	KuroEngine::Timer m_deadTimer;

	Sphere m_hitBox;

	//���S����------------------------------------------------------------------------------



	//�v�l����
	KuroEngine::Timer m_thinkTimer;
	bool m_decisionFlag, m_prevDecisionFlag;

	//����_
	std::vector<KuroEngine::Vec3<float>>m_posArray;


	//���o���------------------------------------------------------------------------------
	DashEffect m_dashEffect;
	EnemyEyeEffect m_eyeEffect;
	int m_knockBackTime;
	EnemyKnockBack m_knockBack;
	EnemyHeadAttack m_headAttack;

private:

	std::unique_ptr<EnemyHitBox> m_debugHitBox;

	//���A�N�V�����\�L---------------------------------------

	//3�����x�N�g����2�����Ɏˉe����֐�
	inline KuroEngine::Vec2<float> Project3Dto2D(KuroEngine::Vec3<float> arg_vector3D, KuroEngine::Vec3<float> arg_basis1, KuroEngine::Vec3<float> arg_basis2) {

		//���x�N�g���𐳋K��
		arg_basis1.Normalize();
		arg_basis2.Normalize();

		//3�����x�N�g����2�����x�N�g���Ɏˉe
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


	//�U���t�F�[�Y---------------
	int m_maxAttackIntervalTime;//�\�����쎞��
	int m_maxAttackTime;		//�U������

	KuroEngine::Timer m_attackInterval;	//�\������
	bool m_attackFlag;					//�U����
	KuroEngine::Timer m_attackTimer;	//�U���̍L����

	float m_attackHitBoxRadius;		//�U���̓����蔻��(�~)
	float m_attackhitBoxRadiusMax;	//�U���̓����蔻��(�ő�l)

	std::shared_ptr<KuroEngine::Model>m_attackRingModel;
	KuroEngine::Color m_ringColor;
	//�U���t�F�[�Y---------------


	Sphere m_hitBox;
	Sphere m_enemyHitBox;
	float m_radius;
	Sphere m_sightHitBox;
	CircleSearch m_sightArea;

	bool m_findPlayerFlag, m_preFindPlayerFlag, m_intervalFlag;

	//���S����---------------------------------------
	bool m_deadFlag;
	bool m_startDeadMotionFlag;
	KuroEngine::Timer m_deadTimer;
	float m_deadScale;
	//���S����---------------------------------------

	//�\������---------------------------------------
	KuroEngine::Vec3<float> m_larpScale, m_scale;
	//�\������---------------------------------------


	//�ړ�����---------------------------------------
	KuroEngine::Vec3<float>m_larpPos;
	KuroEngine::Quaternion m_larpRotation;
	//�ړ�����---------------------------------------

	//�ۉe�p
	float m_shadowInfluenceRange;
	const float SHADOW_INFLUENCE_RANGE = 6.0f;


	std::unique_ptr<EnemyHitBox> m_debugHitBox;

	std::shared_ptr<KuroEngine::Model>m_hitBoxModel;

	void SetParam()
	{
		//���G---------------------------------------
		//����
		m_sightRange = m_initializedTransform.GetScale().Length() * DebugEnemy::Instance()->GetDossunParam().m_sightRadius;
		//�U��---------------------------------------
		//�v���C���[�ƓG�̔���
		m_radius = m_initializedTransform.GetScale().Length() * DebugEnemy::Instance()->GetDossunParam().m_hitBoxRadius;
		//�~�̍ő�̍L����
		m_attackhitBoxRadiusMax = DebugEnemy::Instance()->GetDossunParam().m_attackHitBoxRadius;
		//�~���L����؂�܂ł̎���
		m_maxAttackIntervalTime = DebugEnemy::Instance()->GetDossunParam().m_attackTime;
		//�U���̃N�[���^�C��
		m_maxAttackTime = DebugEnemy::Instance()->GetDossunParam().m_attackCoolTime;
	}

	void Attack(Player &arg_player);

	//�A�j���[�V����
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

	//�ړ�
	KuroEngine::Vec3<float>m_pos;
	std::vector<KuroEngine::Vec3<float>>m_posArray;
	std::unique_ptr<PatrolBasedOnControlPoint> m_patrol;

	//�p�x
	KuroEngine::Vec3<float>m_upVec;
	KuroEngine::Quaternion m_rotation, m_larpRotation;

	//�e
	KuroEngine::Vec3<float>m_bulletDir;
	float m_bulletScale;
	ENEMY_BARREL_PATTERN m_barrelPattern;
	BulletManager m_bulletManager;

	//����
	float m_radius;
	Sphere m_hitBox;


	//���S
	bool m_startDeadMotionFlag;
	bool m_deadFlag;

	bool m_noticeFlag;

};