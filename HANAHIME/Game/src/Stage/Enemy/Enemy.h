#pragma once
#include"../StageParts.h"
#include"../../AI/EnemyPatrol.h"
#include"../../AI/EnemySearch.h"
#include"../Grass.h"
#include"ForUser/DrawFunc/BillBoard/DrawFuncBillBoard.h"
#include"EnemyDataReferenceForCircleShadow.h"
#include"../../../src/Graphics/BasicDraw.h"
#include"../../CPUParticle/DashEffect.h"
#include"../../Effect/EnemyEyeEffect.h"
#include"../../AI/EnemyAttack.h"
#include"../../AI/IEnemyAI.h"

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

class MiniBug :public StageParts
{
public:
	MiniBug(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, std::vector<KuroEngine::Vec3<float>>posArray, bool loopFlag)
		:StageParts(MINI_BUG, arg_model, arg_initTransform), m_deadTimer(120), m_eyeEffect(&m_transform), ENEMY_ID(ENEMY_MAX_ID)
	{

		m_sightArea.Init(&m_transform);
		track.Init(0.5f);
		m_posArray = posArray;

		m_nowStatus = SEARCH;
		m_prevStatus = NONE;
		m_limitIndex = 0;
		m_deadFlag = false;
		m_startDeadMotionFlag = false;
		m_deadTimer.Reset(120);

		m_tex.resize(MAX);
		m_tex[FIND] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/Find.png");
		m_tex[HIT] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/Attack.png");
		m_tex[LOOK] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/hatena.png");
		m_tex[FAR_AWAY] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/hatena.png");
		m_tex[DEAD] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/reaction/dead.png");

		m_reaction = std::make_unique<Reaction>(m_tex);

		m_hitBox.m_centerPos = &m_transform.GetPos();
		m_hitBox.m_radius = &m_hitBoxSize;

		m_hitBoxSize = 3.0f;

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

		m_shadowInfluenceRange = SHADOW_INFLUENCE_RANGE;

		//�ۉe�p�ɓG�̃f�[�^�̎Q�Ƃ�n���B
		EnemyDataReferenceForCircleShadow::Instance()->SetData(&m_transform, &m_shadowInfluenceRange, &m_deadFlag);
		m_finalizeFlag = false;
		++ENEMY_MAX_ID;
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


private:

	std::unique_ptr<Reaction> m_reaction;

	std::vector<std::shared_ptr<KuroEngine::TextureBuffer>>m_tex;

	//���A�N�V�����\�L---------------------------------------



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
	DossunRing(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, ENEMY_ATTACK_PATTERN status)
		:StageParts(DOSSUN_RING, arg_model, arg_initTransform)
	{

		m_hitBoxRadiusMax = 15.0f * arg_initTransform.GetScale().x;
		m_hitBoxRadius = 0.0f;
		m_findPlayerFlag = false;
		m_preFindPlayerFlag = false;
		m_nowStatus = status;

		m_sightRange = m_hitBoxRadiusMax;

		//���E�̔���---------------------------------------
		m_sightHitBox.m_centerPos = &m_transform.GetPos();
		m_sightHitBox.m_radius = &m_sightRange;
		m_sightArea.Init(m_sightHitBox);
		//���E�̔���---------------------------------------

		m_maxAttackIntervalTime = 60 * 2;
		m_maxAttackTime = 60 * 5;

		m_attackInterval.Reset(m_maxAttackIntervalTime);
		m_attackTimer.Reset(m_maxAttackTime);

		//���S����---------------------------------------
		m_deadFlag = false;
		m_startDeadMotionFlag = false;
		m_deadTimer.Reset(120);
		//���S����---------------------------------------

		m_hitBox.m_centerPos = &m_transform.GetPos();
		m_hitBox.m_radius = &m_hitBoxRadius;

		m_hitBoxModel =
			KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "RedSphere.glb");

		m_shadowInfluenceRange = SHADOW_INFLUENCE_RANGE;

		//�ۉe�p�ɓG�̃f�[�^�̎Q�Ƃ�n���B
		EnemyDataReferenceForCircleShadow::Instance()->SetData(&m_transform, &m_shadowInfluenceRange, &m_deadFlag);


		m_attackRingModel = 
			KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "impactWave.glb");

	}
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

	float m_hitBoxRadius;		//�U���̓����蔻��(�~)
	float m_hitBoxRadiusMax;	//�U���̓����蔻��(�ő�l)

	std::shared_ptr<KuroEngine::Model>m_attackRingModel;

	//�U���t�F�[�Y---------------


	Sphere m_hitBox;
	Sphere m_sightHitBox;
	CircleSearch m_sightArea;

	bool m_findPlayerFlag, m_preFindPlayerFlag;

	//���S����---------------------------------------
	bool m_deadFlag;
	bool m_startDeadMotionFlag;
	KuroEngine::Timer m_deadTimer;
	float m_scale;
	//���S����---------------------------------------


	//�ړ�����---------------------------------------
	KuroEngine::Vec3<float>m_larpPos;
	KuroEngine::Quaternion m_larpRotation;
	//�ړ�����---------------------------------------

	//�ۉe�p
	float m_shadowInfluenceRange;
	const float SHADOW_INFLUENCE_RANGE = 6.0f;


	std::shared_ptr<KuroEngine::Model>m_hitBoxModel;
};

class Battery : public StageParts,IEnemyAI
{
public:
	Battery(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, std::vector<KuroEngine::Vec3<float>>arg_posArray, float arg_bulletScale, ENEMY_BARREL_PATTERN arg_barrelPattern);
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