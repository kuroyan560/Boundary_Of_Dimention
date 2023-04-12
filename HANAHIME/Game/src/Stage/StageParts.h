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

//�n�`���
class StageParts
{
public:
	enum STAGE_PARTS_TYPE { TERRIAN, START_POINT, GOAL_POINT, MOVE_SCAFFOLD, NUM, NONE };
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

	virtual void OnInit() {};

public:
	StageParts(STAGE_PARTS_TYPE arg_type, std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:m_type(arg_type), m_model(arg_model), m_initializedTransform(arg_initTransform), m_transform(arg_initTransform) {}
	virtual ~StageParts() {}

	void Init();
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//�n�`����ʃQ�b�^
	const STAGE_PARTS_TYPE& GetType()const { return m_type; }

	//�g�����X�t�H�[���Q�b�^
	const KuroEngine::Transform& GetTransform()const { return m_transform; }
};

//�M�~�b�N�Ƃ͕ʂ̒ʏ�̒n�`
class Terrian : public StageParts
{
	friend class Player;

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
	Terrian(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(TERRIAN, arg_model, arg_initTransform) 
	{
		BuilCollisionMesh();
	}
};

//�X�^�[�g�n�_
class StartPoint : public StageParts
{
public:
	StartPoint(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(START_POINT, arg_model, arg_initTransform) {}
};

//�S�[���n�_
class GoalPoint : public StageParts
{
public:
	GoalPoint(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(GOAL_POINT, arg_model, arg_initTransform) {}
};

//��������
class MoveScaffold : public StageParts
{
public:
	//����
	class KeyTransform
	{
	public:
		KuroEngine::Quaternion m_rotate;
		KuroEngine::Vec3<float>m_translation;
		KuroEngine::Vec3<float>m_scaling;
	};

private:
	//�g�����X�t�H�[���̔z��i0����X�^�[�g�A����Terrian�ȊO�̃p�[�c�ɓ����邩�Ō�܂œ��B������܂�Ԃ��j
	std::vector<KeyTransform>m_transformArray;

	//KeyTransform����Transform���\�z
	KuroEngine::Transform GetTransformWithKey(const KeyTransform& arg_key);

public:
	MoveScaffold(std::weak_ptr<KuroEngine::Model>arg_model, std::vector<KeyTransform>arg_transformArray)
		:StageParts(MOVE_SCAFFOLD, arg_model, GetTransformWithKey(arg_transformArray[0])) {}
};
