#pragma once
#pragma once
#include<array>
#include<memory>
#include<optional>
#include"Common/Transform.h"
#include"Render/RenderObject/ModelInfo/ModelMesh.h"
#include"../Graphics/BasicDrawParameters.h"
#include"ForUser/Timer.h"
#include"../HUD/CheckPointUI.h"

namespace KuroEngine
{
	class Model;
	class Camera;
	class LightManager;
	class ModelAnimator;
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
struct AABB {
	KuroEngine::Vec3<float> m_min;
	KuroEngine::Vec3<float> m_max;
	struct CollisionInfo {
		KuroEngine::Vec3<float> m_pushBack;
	};
	//AABB�̓����蔻��
	std::optional<CollisionInfo> CheckAABBCollision(const AABB& arg_aabb1);
};

class TerrianMeshCollider
{
public:
	//�����蔻��p�|���S���R���e�i
	std::vector<std::vector<TerrianHitPolygon>> m_collisionMesh;
	//���b�V���ƃ��b�V���̓����蔻��Ɏg�p����A�|���S��������ꂽ�����̃R���e�i
	std::vector<std::vector<AABB>> m_aabb;
	const float CUBE_Z = 5.0f;

public:
	//�����蔻��p���b�V�����쐬�B
	void BuilCollisionMesh(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_transform);
	//�����蔻��p�|���S���R���e�i�Q�b�^
	const std::vector<std::vector<TerrianHitPolygon>>& GetCollisionMesh()const { return m_collisionMesh; }
private:
	//3���_���痧���̂𐶐�����B
	AABB CreateCubeFromPolygon(const KuroEngine::Vec3<float>& arg_v1, const KuroEngine::Vec3<float>& arg_v2, const KuroEngine::Vec3<float>& arg_v3, const KuroEngine::Vec3<float>& arg_normal);
};

//�n�`���
class StageParts
{
public:
	enum STAGE_PARTS_TYPE
	{
		//�n�`�ȂǓ��I�I�u�W�F�N�g
		TERRIAN,
		START_POINT,
		GOAL_POINT,
		APPEARANCE,
		MOVE_SCAFFOLD,
		LEVER,
		IVY_ZIP_LINE,
		IVY_BLOCK,
		SPLATOON_FENCE,
		GATE,
		CHECK_POINT,
		STAR_COIN,
		BACKGROUND,

		//�G�ȂǐÓI�I�u�W�F�N�g
		MINI_BUG,	//�`�r��
		DOSSUN_RING,	//�h�b�X�������O
		BATTERY,	//�C��

		NUM, NONE
	};
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
	//���݂̃t���[���ňړ������ʁB
	KuroEngine::Vec3<float> m_moveAmount;

	virtual void OnInit() {};
public:
	StageParts(STAGE_PARTS_TYPE arg_type, std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:m_type(arg_type), m_model(arg_model), m_initializedTransform(arg_initTransform), m_transform(arg_initTransform)
	{
	}
	virtual ~StageParts() {}

	void Init();
	virtual void Update(Player& arg_player) = 0;
	virtual void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//�n�`����ʃQ�b�^
	const STAGE_PARTS_TYPE& GetType()const { return m_type; }

	//�g�����X�t�H�[���Q�b�^
	KuroEngine::Transform& GetTransform() { return m_transform; }
	const KuroEngine::Transform& GetInitTransform() { return m_initializedTransform; }

	void SetTransform(const KuroEngine::Transform& transform)
	{
		m_transform = transform;
	}
	KuroEngine::Vec3<float> GetMoveAmount() { return m_moveAmount; }

	//�X�e�[�W���̃Q�b�^
	const std::weak_ptr<KuroEngine::Model>& GetModel()const { return m_model; }
};

//�M�~�b�N�Ƃ͕ʂ̒ʏ�̒n�`
class Terrian : public StageParts
{
	TerrianMeshCollider m_collider;

public:
	Terrian(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, std::weak_ptr<KuroEngine::Model>arg_collisionModel)
		:StageParts(TERRIAN, arg_model, arg_initTransform)
	{
		m_collider.BuilCollisionMesh(arg_collisionModel, m_initializedTransform);
	}
	void Update(Player& arg_player)override {}
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;
	const std::vector<std::vector<TerrianHitPolygon>>& GetCollisionMesh()const { return m_collider.GetCollisionMesh(); }
};

//�X�^�[�g�n�_
class StartPoint : public StageParts
{
public:
	StartPoint(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(START_POINT, arg_model, arg_initTransform) {}
	void Update(Player& arg_player)override {}
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override {};
};

//�S�[���n�_
class GoalPoint : public StageParts
{
	bool m_hitPlayer = false;

public:
	GoalPoint(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(GOAL_POINT, arg_model, arg_initTransform) {
	}
	void OnInit()override
	{
		m_offset.SetScale({ 0.0f, 0.0f, 0.0f });
	}
	void Update(Player& arg_player)override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	const bool& HitPlayer()const { return m_hitPlayer; }

	//�`�掞�̃I�t�Z�b�g
	KuroEngine::Transform m_offset;
};

//�����������̃I�u�W�F�N�g�i�`�悾���ŉ������Ȃ��j
class Appearance : public StageParts
{
	static std::map<std::string, std::weak_ptr<KuroEngine::Model>>s_models;

	TerrianMeshCollider m_collider;
public:
	static void ModelsUpdate();

	Appearance(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, std::weak_ptr<KuroEngine::Model>arg_collisionModel);
	void Update(Player& arg_player)override {}
	const std::vector<std::vector<TerrianHitPolygon>>& GetCollisionMesh()const { return m_collider.GetCollisionMesh(); }
};

//��������
class MoveScaffold : public StageParts
{
public:
	TerrianMeshCollider m_collider;
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

	//�v���C���[������Ă��瓮���o���܂ł̃^�C�}�[
	KuroEngine::Timer m_moveStartTimer;
	const float MOVE_START_TIMER = 30.0f;

	//�L�����t���O
	bool m_isActive;
	bool m_isOldActive;
	bool m_prevOnPlayer;
	bool m_onPlayer;	//�v���C���[������Ă��邩
	bool m_isStop;		//�ʂ̓�������ƂԂ����Ĉꎞ��~�������B
	bool m_isOldStop;		//�ʂ̓�������ƂԂ����Ĉꎞ��~�������B

	//���W�֘A
	KuroEngine::Vec3<float> m_nowPos;
	KuroEngine::Vec3<float> m_oldPos;

	const float MOVE_SPEED = 0.4f;

	std::weak_ptr<KuroEngine::Model>m_collisionModel;

public:
	MoveScaffold(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, std::vector<KuroEngine::Vec3<float>>arg_translationArray, std::weak_ptr<KuroEngine::Model>arg_collisionModel)
		:StageParts(MOVE_SCAFFOLD, arg_model, arg_initTransform), m_translationArray(arg_translationArray)
	{
		m_maxTranslation = static_cast<int>(arg_translationArray.size()) - 1;
		m_isOldActive = false;
		m_collisionModel = arg_collisionModel;
	}

	void OnInit()override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	void Update(Player& arg_player)override;

	const std::vector<std::vector<TerrianHitPolygon>>& GetCollisionMesh()const { return m_collider.GetCollisionMesh(); }

	//�L���� ������
	void Activate() { m_isActive = true; }
	void Deactivate() { m_isActive = false; }
	bool GetIsActive() { return m_isActive; }
	void Stop() { m_isStop = true; }	//�n�`���m���Ԃ������Ƃ��Ɉꎞ��~���鏈���B

	//�v���C���[����ɏ���Ă��邩�̃t���O��؂�ւ���B
	void SetOnPlayer(bool arg_onPlayer) { m_onPlayer = arg_onPlayer; }
	void OnPlayer();

	//�����蔻��č\�z
	void ReBuildCollisionMesh();

	//�����߂��B
	void PushBack(KuroEngine::Vec3<float> arg_pushBack);

	//�ړ�������
	KuroEngine::Vec3<float> GetNowPos() { return m_nowPos; }
	KuroEngine::Vec3<float> GetOldPos() { return m_oldPos; }
};

//���o�[
class Lever :public StageParts
{
public:
	static const int INVALID_ID = -1;
	static const std::string TURN_ON_ANIM_NAME;
	static const std::string TURN_OFF_ANIM_NAME;

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

	std::shared_ptr<KuroEngine::ModelAnimator>m_modelAnimator;

public:
	Lever(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, int arg_id, bool arg_initFlg = false);

	void OnInit()override;
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
		m_translationArray.back().y += 1.0f;

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
	//�������Ă���Ԃɕ`�悷�郂�f��
	std::shared_ptr<KuroEngine::Model>m_nonExistModel;
	//�������Ă���Ԃɕ`�悷�郂�f���̃}�e���A��
	std::shared_ptr<KuroEngine::Material>m_nonExistMaterial;
	IndividualDrawParameter m_nonExistDrawParam;


	//�u���b�N�̍����O���W
	KuroEngine::Vec3<float>m_leftTopFront;
	//�u���b�N�̉E�������W
	KuroEngine::Vec3<float>m_rightBottomBack;
	//�����蔻��p�R���W����
	TerrianMeshCollider m_collider;
	//�o������
	bool m_isAppear;
	bool m_prevOnPlayer;
	bool m_onPlayer;	//�v���C���[������Ă��邩

	//�C�[�W���O�^�C�}�[�n
	float m_easingTimer;
	const float EASING_TIMER = 0.03f;

	const float HIT_SCALE_MIN = 7.5f * 2.0f;
	const float HIT_SCALE_MAX = 15.5f * 2.0f;
	const float SCALE_DEF = 15.0f;

	std::weak_ptr<KuroEngine::Model>m_collisionModel;

	void ReuilCollisionMesh()
	{
		m_collider.BuilCollisionMesh(m_collisionModel, m_transform);
	}

public:
	IvyBlock(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, KuroEngine::Vec3<float>arg_leftTopFront, KuroEngine::Vec3<float>arg_rightBottomBack, std::weak_ptr<KuroEngine::Model>arg_collisionModel);

	void Update(Player& arg_player)override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	void OnInit()override
	{
		m_isAppear = true;
		m_onPlayer = false;
		m_prevOnPlayer = false;
		m_easingTimer = EASING_TIMER;

		ReuilCollisionMesh();

		m_nonExistDrawParam.m_alpha = 0.0f;
	}

	void Appear();
	void Disappear();

	void OnPlayer() { m_onPlayer = true; }
	void OffPlayer() { m_onPlayer = false; }
	bool GetOnPlayer() { return m_onPlayer; }
	bool GetIsAppear() { return m_isAppear; }

	//���W���擾
	KuroEngine::Vec3<float> GetPos() { return m_transform.GetPosWorld(); }

	float GetHitScaleMin() { return HIT_SCALE_MIN; }
	float GetHitScaleMax() { return HIT_SCALE_MAX; }

	const std::vector<std::vector<TerrianHitPolygon>>& GetCollisionMesh()const { return m_collider.GetCollisionMesh(); }
};

//�X�v���g�D�[�����t�F���X�i�������Ēʂ�j
class SplatoonFence : public StageParts
{
public:
	TerrianMeshCollider m_collider;
public:
	SplatoonFence(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, std::weak_ptr<KuroEngine::Model>arg_collisionModel)
		:StageParts(SPLATOON_FENCE, arg_model, arg_initTransform) {
		m_collider.BuilCollisionMesh(arg_collisionModel, m_initializedTransform);
	}
	void Update(Player& arg_player)override {};
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	const std::vector<std::vector<TerrianHitPolygon>>& GetCollisionMesh()const { return m_collider.GetCollisionMesh(); }
};

class Gate : public StageParts
{
	static const int INVALID_STAGE_NUM = -1;
	static int s_destStageNum;

	//�Q�[�g�̃e�N�X�`��
	static const int GATE_TEX_ARRAY_SIZE = 6;
	static std::array<std::shared_ptr<KuroEngine::TextureBuffer>, GATE_TEX_ARRAY_SIZE>s_texArray;
	//�e�N�X�`���A�j���[�V����
	int m_texIdx = 0;
	KuroEngine::Timer m_animTimer;

	//���o�^�C�}�[
	KuroEngine::Angle m_effectSinCurveAngle;
	float m_effectScale = 1.0f;

	int m_id;
	int m_destStageNum;
	int m_destGateId;


	//�o����p
	bool IsExit() 
	{
		return m_destStageNum == INVALID_STAGE_NUM;
	}

	void OnInit()override
	{
		m_effectSinCurveAngle = 0;
	}

public:
	Gate(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, int arg_id, int arg_destStageNum, int arg_destGateId);

	void Update(Player& arg_player)override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	bool CheckID(int arg_id);
	const int& GetDestStageNum()const { return m_destStageNum; }
	const int& GetDestGateID()const { return m_destGateId; }
};

class CheckPoint : public StageParts
{
	//UI
	static std::shared_ptr<CheckPointUI> s_ui;
	//�Ō�ɖK�ꂽ�`�F�b�N�|�C���g�̃g�����X�t�H�[��
	static KuroEngine::Transform s_latestVisitTransform;
	//�`�F�b�N�|�C���g���P�ł��K�ꂽ��
	static bool s_visit;

	int m_order;
	bool m_touched = false;

public:
	static std::weak_ptr<CheckPointUI>UI() 
	{
		if (!s_ui)s_ui = std::make_shared<CheckPointUI>();
		return s_ui; 
	}

	//�Ō�ɖK�ꂽ�`�F�b�N�|�C���g�̃g�����X�t�H�[���Q�b�^
	static KuroEngine::Transform GetLatestVistTransform(const KuroEngine::Transform& arg_alternative)
	{
		//�܂��P���K��Ă��Ȃ��Ȃ�����̃g�����X�t�H�[�������̂܂ܕԂ�
		if (!s_visit)return arg_alternative;
		return s_latestVisitTransform;
	}

	CheckPoint(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, int arg_order);

	void Update(Player& arg_player)override;
};

class StarCoin : public StageParts
{
private:
	//�݌v�̓��萔
	static int GET_SUM;
public:
	static const int& GetFlowerSum() { return GET_SUM; }

private:
	bool m_get = false;
	bool m_touched = false;

	void OnInit()override;
public:
	StarCoin(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(CHECK_POINT, arg_model, arg_initTransform) {}

	void Update(Player& arg_player)override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	const bool& IsGet()const { return m_get; }
};

class BackGround : public StageParts
{
public:
	BackGround(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(BACKGROUND, arg_model, arg_initTransform) {}

	void Update(Player& arg_player)override {};
};