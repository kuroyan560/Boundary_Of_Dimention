#pragma once
#include"KuroEngine.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"
#include"CameraController.h"
#include"Render/RenderObject/ModelInfo/ModelMesh.h"
#include"../Stage/StageParts.h"
#include"Render/RenderObject/Light.h"
#include"../Plant/GrowPlantLight.h"

#include<memory>
namespace KuroEngine
{
	class Camera;
	class Model;
	class LightManager;
}

class Stage;

class Player : public KuroEngine::Debugger
{
	//�v���C���[�̃��f��
	std::shared_ptr<KuroEngine::Model>m_model;
	std::shared_ptr<KuroEngine::Model>m_axisModel;

	//�J�����̃��f���i�f�o�b�O�p�j
	std::shared_ptr<KuroEngine::Model>m_camModel;

	//�_����
	KuroEngine::Light::Point m_ptLig;

	//�g�����X�t�H�[��
	KuroEngine::Transform m_prevTransform;
	KuroEngine::Transform m_transform;
	KuroEngine::Transform m_initTransform;

	//�ړ���
	KuroEngine::Vec3<float> m_rowMoveVec;

	//�J�����C���X�^���X
	std::shared_ptr<KuroEngine::Camera>m_cam;

	//�J�����̃R���g���[���[
	CameraController m_camController;

	//�J�������x
	float m_camSensitivity = 1.0f;
	int m_cameraMode;
	std::array<const float, 3> CAMERA_MODE = { -20.0f,-40.0f,-70.0f };

	//�A����ɐB������_����
	GrowPlantLight_Point m_growPlantPtLig;

	//���𐶂₷�ۂ̎U�炵�ʁB
	KuroEngine::Vec2<float> m_grassPosScatter = KuroEngine::Vec2<float>(2.0f, 2.0f);

	//Y����]��
	float m_cameraRotY;	//�J������Y����]�ʁB���̊p�x�����ƂɃv���C���[�̈ړ������𔻒f����B
	float m_cameraRotYStorage;	//�J������Y����]�ʁB
	float m_cameraJumpLerpStorage;	//�v���C���[���W�����v���ɉ�]���Ԃ���ۂ̕�Ԍ��B
	float m_cameraJumpLerpAmount;	//�v���C���[���W�����v���ɉ�]����ۂɕ�Ԃ���ʁB
	float m_playerRotY;	//�v���C���[��Y����]�ʁB����͌����ڏ�ړ������Ɍ������邽�߁B���ʃx�N�g���Ƃ��ɉe�����o�Ȃ��悤�ɂ��Ȃ���΂Ȃ�Ȃ��B
	XMVECTOR m_cameraQ;	//�v���C���[��Y����]�ʂ𔲂����A�J���������݂̂��������ꍇ�̉�]
	XMVECTOR m_moveQ;	//�v���C���[��Y����]�ʂ𔲂����A�v���C���[�̈ړ������݂̂��������ꍇ�̉�]
	XMVECTOR m_normalSpinQ;	//�f�t�H���g�̏�x�N�g���ƌ��݂���n�`�̉�]�ʂ�����킵���N�H�[�^�j�I��

	//�ړ��x�N�g���̃X�J���[
	KuroEngine::Vec3<float> m_moveSpeed;			//�ړ����x
	float m_moveAccel = 0.05f;
	float m_maxSpeed = 0.5f;
	float m_brake = 0.07f;

	//�M�~�b�N�̈ړ���
	KuroEngine::Vec3<float> m_gimmickVel;

	//�ǈړ��̋���
	const float WALL_JUMP_LENGTH = 3.0f;

	//�ڒn�t���O
	bool m_isFirstOnGround;	//�J�n���ɋ󒆂���n�܂�̂ŁA���n�ς݂��Ƃ������Ƃ𔻒f����p�ϐ��B
	bool m_onGround;		//�ڒn�t���O
	bool m_prevOnGround;	//1F�O�̐ڒn�t���O
	bool m_prevOnGimmick;	//�M�~�b�N�̏�ɂ��邩�ǂ���
	bool m_onGimmick;		//�M�~�b�N�̏�ɂ��邩�ǂ���
	bool m_isDeath;			//�v���C���[�����񂾂��ǂ��� ���񂾂烊�g���C�����
	int m_deathTimer;		//���̃t���[���̊Ԏ���ł����玀�񂾔���ɂ���B
	const int DEATH_TIMER = 1;

	//Imgui�f�o�b�O�֐��I�[�o�[���C�h
	void OnImguiItems()override;

	//�v���C���[�̓����̃X�e�[�^�X
	enum class PLAYER_MOVE_STATUS {
		NONE,
		MOVE,	//�ʏ�ړ���
		JUMP,	//�W�����v��(��Ԓ�)
		ZIP,	//�W�b�v���C���ړ���
	}m_playerMoveStatus;

	//�M�~�b�N�ɂ�葀��s�\�̂Ƃ��̃X�e�[�^�X
	enum class GIMMICK_STATUS {
		APPEAR,
		NORMAL,
		EXIT
	}m_gimmickStatus;

	//�v���C���[�̃W�����v�Ɋւ���ϐ�
	KuroEngine::Vec3<float> m_jumpStartPos;			//�W�����v�J�n�ʒu
	KuroEngine::Vec3<float> m_jumpEndPos;			//�W�����v�I���ʒu
	KuroEngine::Vec3<float> m_bezierCurveControlPos;//�W�����v�x�W�F�Ȑ��̐���_	
	XMVECTOR m_jumpStartQ;							//�W�����v�J�n���̃N�H�[�^�j�I��
	XMVECTOR m_jumpEndQ;							//�W�����v�I�����̃N�I�[�^�j�I��
	float m_jumpTimer;								//�W�����v�̌v�����Ԃ�}��^�C�}�[
	const float JUMP_TIMER = 0.08f;
	bool m_canJump;									//�W�����v���ł��邩�̃t���O
	int m_canJumpDelayTimer;						//�W�����v���ł���悤�ɂȂ�܂ł̈����|����
	const int CAN_JUMP_DELAY = 20;
	const int CAN_JUMP_DELAY_FAST = 1;

	//�W�b�v���C���֌W�̃X�e�[�^�X
	const int ZIP_LINE_MOVE_TIMER_START = 30;
	const int ZIP_LINE_MOVE_TIMER_END = 30;
	int m_ziplineMoveTimer;							//�W�b�v���C���ɓ�������o���肷��Ƃ��Ɏg���^�C�}�[
	KuroEngine::Vec3<float> m_zipInOutPos;			//�W�b�v���C���ɏo����������肷��Ƃ��̏ꏊ�B�C�[�W���O�Ɏg�p����B
	std::weak_ptr<IvyZipLine> m_refZipline;

	struct HitCheckResult
	{
		KuroEngine::Vec3<float>m_terrianNormal;
	};
	bool HitCheckAndPushBack(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo = nullptr);

	//�v���C���[�̑傫���i���a�j
	float GetPlayersRadius()
	{
		return m_transform.GetScale().x;
	}

public:
	Player();
	void Init(KuroEngine::Transform arg_initTransform);
	void Update(const std::weak_ptr<Stage>arg_nowStage);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, bool arg_cameraDraw = false);
	void Finalize();

	std::weak_ptr<KuroEngine::Camera>GetCamera() { return m_cam; }

	//�J�����R���g���[���[�̃f�o�b�K�|�C���^�擾
	KuroEngine::Debugger* GetCameraControllerDebugger() { return &m_camController; }
	//KuroEngine::Transform& GetCameraControllerParentTransform() { return m_camController.GetParentTransform(); }

	KuroEngine::Transform& GetTransform() { return m_transform; }
	KuroEngine::Vec2<float>& GetGrassPosScatter() { return m_grassPosScatter; }

	void SetGimmickVel(const KuroEngine::Vec3<float>& arg_gimmickVel) { m_gimmickVel = arg_gimmickVel; }

	//�_�����Q�b�^
	KuroEngine::Light::Point* GetPointLig() { return &m_ptLig; }
	bool GetOnGround() { return m_onGround; }
	bool GetOnGimmick() { return m_onGimmick; }
	bool GetIsStatusMove() { return m_playerMoveStatus == PLAYER_MOVE_STATUS::MOVE; }

	//�W�����v�I���n�_
	KuroEngine::Vec3<float> GetJumpEndPos() { return m_jumpEndPos; }
	void SetJumpEndPos(KuroEngine::Vec3<float> arg_jumpEndPos) { m_jumpEndPos = arg_jumpEndPos; }
	bool GetIsJump() { return m_playerMoveStatus == PLAYER_MOVE_STATUS::JUMP; }

	//���S�t���O���擾�B
	bool GetIsDeath() { return m_isDeath; }

private:
	//���C�ƃ��b�V���̓����蔻��o�͗p�\����
	struct MeshCollisionOutput {
		bool m_isHit;						// ���C�����b�V���ɓ����������ǂ��� �����������b�V���܂ł̋����͍l������Ă��炸�A���������������ǂ����𔻒f����p�B
		float m_distance;					// ���C�����b�V���ɓ��������ꍇ�A�Փ˒n�_�܂ł̋���������B���̃p�����[�^�[��m_isHit��g�ݍ��킹�Đ������Փ˔�����s���B
		KuroEngine::Vec3<float> m_pos;		// ���C�̏Փ˒n�_�̍��W
		KuroEngine::Vec3<float> m_normal;	// ���C�̏Փ˒n�_�̖@��
		KuroEngine::Vec2<float> m_uv;		// ���C�̏Փ˒n�_��UV
	};
	//���˂��郌�C��ID
	enum class RAY_ID {

		GROUND,	//�n��������Ĕ�΂����C�B�ݒu����Ŏg�p����B
		AROUND,	//���͂Ɍ������Ĕ�΂����C�B�ǂ̂ڂ蔻��Ŏg�p����B
		CHECK_DEATH,	//���S�m�F�p
		CHECK_CLIFF,	//�R�`�F�b�N�p

	};
	//���C�̕���ID
	enum class RAY_DIR_ID {
		RIGHT,
		LEFT,
		FRONT,
		BEHIND,
		TOP,
		BOTTOM,
	};

	/// <summary>
	/// ���C�ƃ��b�V���̓����蔻��
	/// </summary>
	/// <param name="arg_rayPos"> ���C�̎ˏo�n�_ </param>
	/// <param name="arg_rayDir"> ���C�̎ˏo���� </param>
	/// <param name="arg_targetMesh"> ������s���Ώۂ̃��b�V�� </param>
	/// <returns> �����蔻�茋�� </returns>
	MeshCollisionOutput MeshCollision(const KuroEngine::Vec3<float>& arg_rayPos, const KuroEngine::Vec3<float>& arg_rayDir, std::vector<TerrianHitPolygon>& arg_targetMesh);

	/// <summary>
	/// �d�S���W�����߂�B
	/// </summary>
	KuroEngine::Vec3<float> CalBary(const KuroEngine::Vec3<float>& PosA, const KuroEngine::Vec3<float>& PosB, const KuroEngine::Vec3<float>& PosC, const KuroEngine::Vec3<float>& TargetPos);

	//�Փ˓_�p�\����
	struct ImpactPointData {
		KuroEngine::Vec3<float> m_impactPos;
		KuroEngine::Vec3<float> m_normal;
		bool m_isActive;
		bool m_isFastJump;	//�Ԃ����Ă���b�����ăW�����v����̂ł͂Ȃ��A�����ɔ�я��t���O�B�����ɏ�肽���I�u�W�F�N�g������ꍇ�A�����true�ɂ���B
		ImpactPointData(KuroEngine::Vec3<float> arg_impactPos, KuroEngine::Vec3<float> arg_normal) : m_impactPos(arg_impactPos), m_normal(arg_normal), m_isActive(true), m_isFastJump(false) {};
	};

	//CastRay�ɓn���f�[�^�p�\����
	struct CastRayArgument {
		std::weak_ptr<StageParts> m_stage;			//�X�e�[�W
		std::vector<TerrianHitPolygon> m_mesh;		//������s���Ώۂ̃��b�V��
		std::vector<ImpactPointData> m_impactPoint;	//�O�㍶�E�̃��C�̓��������n�_�B
		KuroEngine::Vec3<float> m_bottomTerrianNormal;
		StageParts::STAGE_PARTS_TYPE m_stageType;
		std::array<bool, 6> m_checkDeathCounter;
	};

	/// <summary>
	/// ���C�𔭎˂��A���̌�̈�A�̏������܂Ƃ߂��֐�
	/// </summary>
	/// <param name="arg_rayCastPos"> �L�����N�^�[�̍��W </param>
	/// <param name="arg_rayCastPos"> ���C�̎ˏo�n�_ </param>
	/// <param name="arg_rayDir"> ���C�̎ˏo���� </param>
	/// <param name="arg_rayLength"> ���C�̒��� </param>
	/// <param name="arg_collisionData"> �������܂Ƃ߂��\���� </param>
	/// <param name="arg_rayID"> ���C�̎�� </param>
	bool CastRay(KuroEngine::Vec3<float>& arg_charaPos, const KuroEngine::Vec3<float>& arg_rayCastPos, const KuroEngine::Vec3<float>& arg_rayDir, float arg_rayLength, CastRayArgument& arg_collisionData, RAY_ID arg_rayID, RAY_DIR_ID arg_rayDirID = RAY_DIR_ID::RIGHT);

	//�ړ�������B
	void Move(KuroEngine::Vec3<float>& arg_newPos);

	//�����蔻��
	void CheckHit(KuroEngine::Vec3<float>& arg_frompos, KuroEngine::Vec3<float>& arg_nowpos, std::weak_ptr<Stage>arg_nowStage);

	//�x�W�G�Ȑ������߂�B
	KuroEngine::Vec3<float> CalculateBezierPoint(float arg_time, KuroEngine::Vec3<float> arg_startPoint, KuroEngine::Vec3<float> arg_endPoint, KuroEngine::Vec3<float> arg_controlPoint);

	//�ړ������𐳂��������邽�߂̏���
	void AdjustCaneraRotY(const KuroEngine::Vec3<float>& arg_nowUp, const KuroEngine::Vec3<float>& arg_nextUp);

	//���͗p�̓����蔻��
	void CheckDeath(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, Player::CastRayArgument& arg_castRayArgment);
	void CheckHitAround(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, Player::CastRayArgument& arg_castRayArgment);
	void CheckHitGround(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, Player::CastRayArgument& arg_castRayArgment);
	void CheckCliff(Player::ImpactPointData& arg_impactPointData, std::weak_ptr<Stage> arg_nowStage);
	void CheckCanJump(Player::ImpactPointData& arg_impactPointData, std::weak_ptr<Stage> arg_nowStage);

	//�W�b�v���C���Ƃ̓����蔻��
	void CheckZipline(const KuroEngine::Vec3<float> arg_newPos, std::weak_ptr<Stage> arg_nowStage);

	//�W�b�v���C�����̍X�V����
	void UpdateZipline();

};

