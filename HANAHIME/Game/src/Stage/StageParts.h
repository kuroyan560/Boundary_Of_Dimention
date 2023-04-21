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
class Switch;

//�����蔻��p�|���S��
struct TerrianHitPolygon
{
	bool m_isActive;					//���̃|���S�����L��������Ă��邩�̃t���O
	KuroEngine::ModelMesh::Vertex m_p0;	//���_0
	KuroEngine::ModelMesh::Vertex m_p1;	//���_1
	KuroEngine::ModelMesh::Vertex m_p2;	//���_2
};

class TerrianMeshCollider
{
public:
	//�����蔻��p�|���S���R���e�i
	std::vector<std::vector<TerrianHitPolygon>> m_collisionMesh;

public:
	//�����蔻��p���b�V�����쐬�B
	void BuilCollisionMesh(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_transform);
	//�����蔻��p�|���S���R���e�i�Q�b�^
	const std::vector<std::vector<TerrianHitPolygon>>& GetCollisionMesh()const { return m_collisionMesh; }
};

//�n�`���
class StageParts
{
public:
	enum STAGE_PARTS_TYPE { TERRIAN, START_POINT, GOAL_POINT, APPEARANCE, MOVE_SCAFFOLD, LEVER, IVY_ZIP_LINE, IVY_BLOCK, NUM, NONE };
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
	virtual void Update(Player& arg_player) = 0;
	virtual void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//�n�`����ʃQ�b�^
	const STAGE_PARTS_TYPE& GetType()const { return m_type; }

	//�g�����X�t�H�[���Q�b�^
	const KuroEngine::Transform& GetTransform()const { return m_transform; }
	void SetTransform(const KuroEngine::Transform& transform)
	{
		m_transform = transform;
	}

	//�X�e�[�W���̃Q�b�^
	const std::weak_ptr<KuroEngine::Model>& GetModel()const { return m_model; }
};

//�M�~�b�N�Ƃ͕ʂ̒ʏ�̒n�`
class Terrian : public StageParts
{
	TerrianMeshCollider m_collider;

public:
	Terrian(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(TERRIAN, arg_model, arg_initTransform)
	{
		m_collider.BuilCollisionMesh(arg_model, arg_initTransform);
	}
	void Update(Player& arg_player)override {}
	const std::vector<std::vector<TerrianHitPolygon>>& GetCollisionMesh()const { return m_collider.GetCollisionMesh(); }
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
	bool m_hitPlayer = false;
public:
	GoalPoint(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(GOAL_POINT, arg_model, arg_initTransform) {}
	void Update(Player& arg_player)override;

	const bool& HitPlayer()const { return m_hitPlayer; }
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
	TerrianMeshCollider m_collider;

	//�g�����X�t�H�[���̔z��i0����X�^�[�g�A����Terrian�ȊO�̃p�[�c�ɓ����邩�Ō�܂œ��B������܂�Ԃ��j
	std::vector<KuroEngine::Vec3<float>>m_translationArray;

	int m_maxTranslation;		//�ړ�����n�_ - 1�̐�
	int m_nowTranslationIndex;	//���݂̈ړ�����n�_��Index
	int m_nextTranslationIndex;	//���̈ړ�����n�_��Index
	float m_moveLength;			//���̒n�_�܂ňړ������
	float m_nowMoveLength;		//�ړ����Ă��錻�ݗ�
	KuroEngine::Vec3<float> m_moveDir;	//�ړ�����
	bool m_isOder;				//���ԂɈړ�

	//�L�����t���O
	bool m_isActive;

	//���W�֘A
	KuroEngine::Vec3<float> m_nowPos;
	KuroEngine::Vec3<float> m_oldPos;

	const float MOVE_SPEED = 0.1f;

public:
	MoveScaffold(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, std::vector<KuroEngine::Vec3<float>>arg_translationArray)
		:StageParts(MOVE_SCAFFOLD, arg_model, arg_initTransform), m_translationArray(arg_translationArray)
	{
		m_maxTranslation = static_cast<int>(arg_translationArray.size()) - 1;
	}

	void OnInit()override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	void Update(Player& arg_player)override;

	const std::vector<std::vector<TerrianHitPolygon>>& GetCollisionMesh()const { return m_collider.GetCollisionMesh(); }

	//�L���� ������
	void Activate() { m_isActive = true; }
	void Deactivate() { m_isActive = false; }
	bool GetIsActive() { return m_isActive; }

	//�ړ�������
	KuroEngine::Vec3<float> GetNowPos() { return m_nowPos; }
	KuroEngine::Vec3<float> GetOldPos() { return m_oldPos; }
};

//���o�[
class Lever :public StageParts
{
public:
	static const int INVALID_ID = -1;

private:
	friend class Stage;

	//���ʔԍ�
	int m_id = INVALID_ID;

	//���삷��X�C�b�`�̃|�C���^
	Switch* m_parentSwitch = nullptr;

	//���R���C�_�[�̏��
	struct BoxCollider
	{
		//���S���W
		KuroEngine::Vec3<float>m_center = { 0,0,0 };
		//�傫��
		KuroEngine::Vec3<float>m_size = { 1,1,1 };
	}m_boxCollider;

	//���������̃t���O
	const bool m_initFlg = false;

	//�Փ˔���g���K�[�p�ϐ�
	bool m_isHit;
	bool m_isOldHit;

	//�I���I�t
	bool m_flg = false;

public:
	Lever(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, int arg_id, bool arg_initFlg = false)
		:StageParts(LEVER, arg_model, arg_initTransform), m_id(arg_id), m_initFlg(arg_initFlg)
	{
		m_boxCollider.m_center = arg_initTransform.GetPosWorld();
		m_boxCollider.m_size = arg_initTransform.GetScale();
	}

	void OnInit()override
	{
		m_flg = m_initFlg;
		m_isHit = false;
		m_isOldHit = false;
	}
	void Update(Player& arg_player)override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	const bool& GetLeverFlg()const { return m_flg; }

};

//�W�b�v���C����
class IvyZipLine : public StageParts
{
	//�g�����X�t�H�[���̔z��i0����X�^�[�g�A�Ō�܂œ��B������܂�Ԃ��j
	std::vector<KuroEngine::Vec3<float>>m_translationArray;

	bool m_isActive;
	bool m_isHitStartPos;
	bool m_isReadyPlayer;	//�v���C���[���œ������������ł������B

	int m_maxTranslation;		//�ړ�����n�_ - 1�̐�
	int m_nowTranslationIndex;	//���݂̈ړ�����n�_��Index
	int m_nextTranslationIndex;	//���̈ړ�����n�_��Index
	float m_moveLength;			//���̒n�_�܂ňړ������
	float m_nowMoveLength;		//�ړ����Ă��錻�ݗ�
	KuroEngine::Vec3<float> m_moveDir;	//�ړ�����

public:

	//�ӂɏ�邱�Ƃ��ł���͈�
	const float JUMP_SCALE = 2.0f;
	const float ZIPLINE_SPEED = 1.0f;

public:
	IvyZipLine(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, std::vector<KuroEngine::Vec3<float>>arg_translationArray)
		:StageParts(IVY_ZIP_LINE, arg_model, arg_initTransform), m_translationArray(arg_translationArray) {

		//�f�o�b�O�p�Ő擪�̃W�b�v���C����Y���W��������B
		m_translationArray.front().x -= 5.0f;
		m_translationArray.front().y -= 6.0f;
		m_translationArray.front().z += 15.0f;

		//�W�b�v���C����ǉ�
		m_translationArray.emplace_back(m_translationArray.back());
		m_translationArray.back().x -= 0.0f;
		m_translationArray.back().z += 30.0f;

		//�W�b�v���C����ǉ�
		m_translationArray.emplace_back(m_translationArray.back());
		m_translationArray.back().x -= 6.0f;
		m_translationArray.back().y += 2.0f;

		//�W�b�v���C���ړ��ɕK�v�ȕϐ���������
		m_maxTranslation = static_cast<int>(m_translationArray.size()) - 1;
		m_nowTranslationIndex = 0;
		m_nextTranslationIndex = 0;
		m_moveLength = 0;
		m_nowMoveLength = 0;

		m_isActive = false;
		m_isReadyPlayer = false;

	}

	void Update(Player& arg_player)override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	//�n�_���I�_�ɓ���������
	void CheckHit(bool arg_isHitStartPos);

	//�W�b�v���C���̐���_�̐�
	int GetTranslationArraySize() { return static_cast<int>(m_translationArray.size()); }

	//�v���C���[�͓�����B
	void CanMovePlayer() { m_isReadyPlayer = true; }

	//�n�_�ƏI�_���擾
	KuroEngine::Vec3<float> GetStartPoint() { return m_translationArray.front(); }
	KuroEngine::Vec3<float> GetEndPoint() { return m_translationArray.back(); }


	//�C�[�W���O�̎n�_�ƏI�_���擾����֐��B
	KuroEngine::Vec3<float> GetPoint(bool arg_isEaseStart);

};

//���u���b�N�i��������o�������肷��u���b�N�j
class IvyBlock : public StageParts
{
	//�u���b�N�̍����O���W
	KuroEngine::Vec3<float>m_leftTopFront;
	//�u���b�N�̉E�������W
	KuroEngine::Vec3<float>m_rightBottomBack;

public:
	IvyBlock(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, KuroEngine::Vec3<float>arg_leftTopFront, KuroEngine::Vec3<float>arg_rightBottomBack)
		:StageParts(IVY_BLOCK, arg_model, arg_initTransform), m_leftTopFront(arg_leftTopFront), m_rightBottomBack(arg_rightBottomBack) {
	}

	void Update(Player& arg_player)override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;
};
