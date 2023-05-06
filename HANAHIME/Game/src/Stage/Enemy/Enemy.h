#pragma once
#include"../StageParts.h"
#include"../../AI/EnemyPatrol.h"
#include"../../AI/EnemySearch.h"

class MiniBug :public StageParts
{
public:
	MiniBug(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, StageParts *arg_parent, std::vector<KuroEngine::Vec3<float>>posArray)
		:StageParts(MINI_BUG, arg_model, arg_initTransform, arg_parent), m_patrol(posArray, 0)
	{
		m_nowStatus = SERACH;
		m_prevStatus = NONE;

		float nearScale = 1.0f;
		float farScale = 6.0f;
		float length = 15.0f;
		float nearHeight = 1.0f;
		float farHeight = 5.0f;
		m_sightArea.Init(nearScale, farScale, length, nearHeight, farHeight);

		m_posArray = posArray;
	}
	void Update(Player& arg_player)override;

	void DebugDraw(KuroEngine::Camera &camera);

private:
	std::shared_ptr<KuroEngine::Model>m_model;

	enum Status
	{
		NONE,
		SERACH,//�z��
		ATTACK,//�v���C���[��ǐ�
		NOTICE,//�v���C���[�炵�����̂�������
		RETURN,//�z�G���A�ɖ߂�
	};

	Status m_nowStatus;
	Status m_prevStatus;

	KuroEngine::Vec3<float>m_pos;
	PatrolBasedOnControlPoint m_patrol;
	SightSearch m_sightArea;

	HeadNextPoint m_trackPlayer;
	KuroEngine::Vec3<float>m_aPointPos;
	KuroEngine::Vec3<float>m_bPointPos;
	KuroEngine::Timer m_trackTimer;

	//�v�l����
	KuroEngine::Timer m_thinkTimer;
	bool m_decisionFlag,m_prevDecisionFlag;

	//����_
	std::vector<KuroEngine::Vec3<float>>m_posArray;

	bool m_attackFlag;
};

class DossunRing : public StageParts
{
public:
	enum Status
	{
		NORMAL,//�v���C���[����������U���J�n
		ALWAYS //��Ɉ��Ԋu�ōU�����Ă���
	};

	DossunRing(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, StageParts *arg_parent, Status status)
		:StageParts(DOSSUN_RING, arg_model, arg_initTransform, arg_parent)
	{
		m_hitBoxRadiusMax = 10.0f;
		m_hitBoxRadius = 0.0f;
		m_findPlayerFlag = false;
		m_nowStatus = status;

		//���E�̔���---------------------------------------
		m_sightHitBox.m_centerPos = &m_transform.GetPos();
		m_sightHitBox.m_radius = &m_hitBoxRadiusMax;
		m_sightArea.Init(m_sightHitBox);
		//���E�̔���---------------------------------------

		m_maxAttackIntervalTime = 60 * 2;
		m_maxAttackTime = 60 * 5;

		m_attackInterval.Reset(m_maxAttackIntervalTime);
		m_attackTimer.Reset(m_maxAttackTime);
	}
	void Update(Player& arg_player)override;

	void DebugDraw(KuroEngine::Camera &camera);

private:

	Status m_nowStatus;

	//�U���t�F�[�Y---------------
	int m_maxAttackIntervalTime;//�\�����쎞��
	int m_maxAttackTime;		//�U������

	KuroEngine::Timer m_attackInterval;	//�\������
	bool m_attackFlag;					//�U����
	KuroEngine::Timer m_attackTimer;	//�U���̍L����

	float m_hitBoxRadius;		//�U���̓����蔻��(�~)
	float m_hitBoxRadiusMax;	//�U���̓����蔻��(�ő�l)
	//�U���t�F�[�Y---------------

	Sphere m_sightHitBox;
	CircleSearch m_sightArea;

	bool m_findPlayerFlag;

};