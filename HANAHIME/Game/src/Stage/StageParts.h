#pragma once
#include<array>
#include<memory>
#include"Common/Transform.h"
#include"../../../../src/engine/Render/RenderObject/ModelInfo/ModelMesh.h"

namespace KuroEngine
{
	class Model;
	class Camera;
	class LightManager;
}

class Player;

//�n�`���
class StageParts
{
public:
	enum STAGE_PARTS_TYPE { TERRIAN, START_POINT, GOAL_POINT, APPEARANCE, MOVE_SCAFFOLD, NUM, NONE };
	static const std::string& GetTypeKeyOnJson(STAGE_PARTS_TYPE arg_type);

private:
	static std::array<std::string, STAGE_PARTS_TYPE::NUM>s_typeKeyOnJson;

protected:
	//�n�`�����
	const STAGE_PARTS_TYPE m_type = NONE;
	//���f���|�C���^
	std::weak_ptr<KuroEngine::Model>m_model;
	//�f�t�H���g�i���f�[�^�j�̃g�����X�t�H�[��
	const KuroEngine::Transform m_initializedTransform;
	//�g�����X�t�H�[��
	KuroEngine::Transform m_transform;
	//�L�����t���O
	bool m_isActive;

	virtual void OnInit() {};
	virtual void OnDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr) {};

	//�����蔻��p�|���S��
	struct Polygon {
		bool m_isActive;					//���̃|���S�����L��������Ă��邩�̃t���O
		KuroEngine::ModelMesh::Vertex m_p0;	//���_0
		KuroEngine::ModelMesh::Vertex m_p1;	//���_1
		KuroEngine::ModelMesh::Vertex m_p2;	//���_2
	};
	//�����蔻��p�|���S���R���e�i��
	std::vector<std::vector<Polygon>> m_collisionMesh;

	//�����蔻��p���b�V�����쐬�B
	void BuilCollisionMesh();

public:
	StageParts(STAGE_PARTS_TYPE arg_type, std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:m_type(arg_type), m_model(arg_model), m_initializedTransform(arg_initTransform), m_transform(arg_initTransform) {}
	virtual ~StageParts() {}

	void Init();
	virtual void Update(Player& arg_player) = 0;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//�L���� ������
	void Activate() { m_isActive = true; }
	void Deactivate() { m_isActive = false; }
	bool GetIsActive() { return m_isActive; }

	//�n�`����ʃQ�b�^
	const STAGE_PARTS_TYPE& GetType()const { return m_type; }

	//�g�����X�t�H�[���Q�b�^
	const KuroEngine::Transform& GetTransform()const { return m_transform; }
	void SetTransform(const KuroEngine::Transform &transform)
	{
		m_transform = transform;
	}

	//�X�e�[�W���̃Q�b�^
	const std::weak_ptr<KuroEngine::Model>& GetModel()const { return m_model; }
	const std::vector<std::vector<Polygon>>& GetCollisionMesh()const { return m_collisionMesh; }
};

//�M�~�b�N�Ƃ͕ʂ̒ʏ�̒n�`
class Terrian : public StageParts
{
	friend class Player;

public:
	Terrian(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(TERRIAN, arg_model, arg_initTransform)
	{
		BuilCollisionMesh();
	}
	void Update(Player& arg_player)override {}
};

//�X�^�[�g�n�_
class StartPoint : public StageParts
{
public:
	StartPoint(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(START_POINT, arg_model, arg_initTransform) {}
	void Update(Player& arg_player)override {}
};

//�S�[���n�_
class GoalPoint : public StageParts
{
public:
	GoalPoint(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(GOAL_POINT, arg_model, arg_initTransform) {}
	void Update(Player& arg_player)override {}
};

//�����������̃I�u�W�F�N�g�i�`�悾���ŉ������Ȃ��j
class Appearance : public StageParts
{
public:
	Appearance(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(APPEARANCE, arg_model, arg_initTransform)
	{
	}
	void Update(Player& arg_player)override {}
};

//��������
class MoveScaffold : public StageParts
{
private:
	//�g�����X�t�H�[���̔z��i0����X�^�[�g�A����Terrian�ȊO�̃p�[�c�ɓ����邩�Ō�܂œ��B������܂�Ԃ��j
	std::vector<KuroEngine::Vec3<float>>m_translationArray;

	int m_maxTranslation;		//�ړ�����n�_ - 1�̐�
	int m_nowTranslationIndex;	//���݂̈ړ�����n�_��Index
	int m_nextTranslationIndex;	//���̈ړ�����n�_��Index
	float m_moveLength;			//���̒n�_�܂ňړ������
	float m_nowMoveLength;		//�ړ����Ă��錻�ݗ�
	KuroEngine::Vec3<float> m_moveDir;	//�ړ�����
	bool m_isOder;				//���ԂɈړ�

	const float MOVE_SPEED = 0.1f;

public:
	MoveScaffold(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, std::vector<KuroEngine::Vec3<float>>arg_translationArray)
		:StageParts(MOVE_SCAFFOLD, arg_model, arg_initTransform), m_translationArray(arg_translationArray)
	{
		BuilCollisionMesh();
		m_maxTranslation = static_cast<int>(arg_translationArray.size()) - 1;
	}

	void OnInit()override;
	void OnDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	void Update(Player& arg_player)override;
};
