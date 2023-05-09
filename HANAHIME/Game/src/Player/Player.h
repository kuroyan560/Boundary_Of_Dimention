#pragma once
#include"KuroEngine.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"
#include"CameraController.h"
#include"Render/RenderObject/ModelInfo/ModelMesh.h"
#include"../Stage/StageParts.h"
#include"Render/RenderObject/Light.h"
#include"../Plant/GrowPlantLight.h"
#include"PlayerCollision.h"
#include"ForUser/Timer.h"
#include"../AI/EnemySearch.h"
#include<memory>
#include"../Item/GameItem.h"

namespace KuroEngine
{
	class Camera;
	class Model;
	class LightManager;
}

class Stage;

class Player : public KuroEngine::Debugger
{

	friend PlayerCollision;

	KuroEngine::Vec3<float> m_debug;

	//�v���C���[�̃��f��
	std::shared_ptr<KuroEngine::Model>m_model;
	std::shared_ptr<KuroEngine::Model>m_axisModel;

	//�J�����̃��f���i�f�o�b�O�p�j
	std::shared_ptr<KuroEngine::Model>m_camModel;

	//�_����
	KuroEngine::Light::Point m_ptLig;

	//�g�����X�t�H�[��
	KuroEngine::Transform m_prevTransform;
	KuroEngine::Transform m_transform;		//�����蔻���ړ��̍ۂ̕����̔���Ɏg�p����g�����X�t�H�[��
	KuroEngine::Transform m_drawTransform;	//�`��Ɏg�p����g�����X�t�H�[�� �v���C���[�𒾂܂�����A�J�����̕��������Ȃ��悤�ɂ��邽�߂Ɏg�p����B
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
	const float MAX_INFLUENCE_RANGE = 8.0f;
	const float MIN_INFLUENCE_RANGE = 1.0f;
	const float ATTACK_INFLUENCE_RANGE = 3.0f;
	const float SUB_INFLUENCE_RANGE = 0.05f;
	const float ADD_INFLUENCE_RANGE = 0.3f;

	//�X�e�[�W�̎Q��
	std::weak_ptr<Stage> m_stage;

	//���𐶂₷�ۂ̎U�炵�ʁB
	KuroEngine::Vec2<float> m_grassPosScatter = KuroEngine::Vec2<float>(2.0f, 2.0f);

	//Y����]��
	float m_cameraRotY;	//�J������Y����]�ʁB���̊p�x�����ƂɃv���C���[�̈ړ������𔻒f����B
	float m_cameraRotYStorage;	//�J������Y����]�ʁB
	float m_cameraRotMove;		//�J�����������b�Ƃ����Ȃ����߂Ɏg�p����ϐ�
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
	float m_deathEffectCameraZ;	//���S���o���̃J����
	const float DEATH_EFFECT_CAMERA_Z = -15.0f;
	const float DEATH_EFFECT_TIMER_SCALE = 0.1f;

	//�U������
	int m_attackTimer;
	const int ATTACK_TIMER = 30;

	//����֘A
	bool m_isInputUnderGround;			//���ލۂ̓��͂�����Ă��邩�𔻒f����p�B
	bool m_isUnderGround;				//�n���̏�Ԃ��ۂ�
	bool m_canUnderGroundRelease;		//�����Ԃ������ł��邩�B�����Ă���Ƃ��ɏ�Ƀt�F���X������Ɛ��肪�����ł��Ȃ��B
	bool m_canOldUnderGroundRelease;	//�����Ԃ������ł��邩�B�����Ă���Ƃ��ɏ�Ƀt�F���X������Ɛ��肪�����ł��Ȃ��B
	const float UNDERGROUND_Y = 6.5f;	//���ޗʁB
	float m_underGroundEaseTimer;		//���ނƂ��╂�シ��Ƃ��Ɏg�p����C�[�W���O�̃^�C�}�[
	const float ADD_UNDERGROUND_EASE_TIMER = 0.04f;

	//Imgui�f�o�b�O�֐��I�[�o�[���C�h
	void OnImguiItems()override;

	//�v���C���[�̓����̃X�e�[�^�X
	enum class PLAYER_MOVE_STATUS {
		NONE,
		MOVE,	//�ʏ�ړ���
		JUMP,	//�W�����v��(��Ԓ�)
		ZIP,	//�W�b�v���C���ړ���
		DEATH,	//���S���B
	}m_playerMoveStatus;
	//�P�t���[���O�̓����̃X�e�[�^�X
	PLAYER_MOVE_STATUS m_beforePlayerMoveStatus;

	//�M�~�b�N�ɂ�葀��s�\�̂Ƃ��̃X�e�[�^�X
	enum class GIMMICK_STATUS {
		APPEAR,
		NORMAL,
		EXIT
	}m_gimmickStatus;

	//���S���o�̃X�e�[�^�X
	enum class DEATH_STATUS {
		APPROACH,
		STAY,
		LEAVE,
	}m_deathStatus;
	int m_deathEffectTimer;
	const int DEATH_EFFECT_APPROACH_TIMER = 30;
	const int DEATH_EFFECT_STAY_TIMER = 30;
	const int DEATH_EFFECT_FINISH_TIMER = 150;
	float m_deathShakeAmount;
	KuroEngine::Vec3<float> m_deathShake;
	const float DEATH_SHAKE_AMOUNT = 3.0f;
	const float SUB_DEATH_SHAKE_AMOUNT = 0.2f;
	bool m_isCameraInvX;

	//�v���C���[�̃W�����v�Ɋւ���ϐ�
	KuroEngine::Vec3<float> m_jumpStartPos;			//�W�����v�J�n�ʒu
	KuroEngine::Vec3<float> m_jumpEndPos;			//�W�����v�I���ʒu
	KuroEngine::Vec3<float> m_bezierCurveControlPos;//�W�����v�x�W�F�Ȑ��̐���_	
	XMVECTOR m_jumpStartQ;							//�W�����v�J�n���̃N�H�[�^�j�I��
	XMVECTOR m_jumpEndQ;							//�W�����v�I�����̃N�I�[�^�j�I��
	float m_jumpTimer;								//�W�����v�̌v�����Ԃ�}��^�C�}�[
	const float JUMP_TIMER = 0.05f;
	bool m_canJump;									//�W�����v���ł��邩�̃t���O
	int m_canJumpDelayTimer;						//�W�����v���ł���悤�ɂȂ�܂ł̈����|����
	const int CAN_JUMP_DELAY = 10;
	const int CAN_JUMP_DELAY_FAST = 1;

	//�W�b�v���C���֌W�̃X�e�[�^�X
	const int ZIP_LINE_MOVE_TIMER_START = 30;
	const int ZIP_LINE_MOVE_TIMER_END = 30;
	int m_ziplineMoveTimer;							//�W�b�v���C���ɓ�������o���肷��Ƃ��Ɏg���^�C�}�[
	KuroEngine::Vec3<float> m_zipInOutPos;			//�W�b�v���C���ɏo����������肷��Ƃ��̏ꏊ�B�C�[�W���O�Ɏg�p����B
	std::weak_ptr<IvyZipLine> m_refZipline;
	bool m_canZip;
	std::vector<KuroEngine::Vec3<float>> m_gimmickExitPos;
	std::vector<KuroEngine::Vec3<float>> m_gimmickExitNormal;

	//�����蔻��p�X�e�[�W�̎Q��
	std::weak_ptr<Stage> m_nowStage;

	//���S�X�v���C�g�֘A�̕ϐ�
	KuroEngine::Timer m_deathSpriteAnimTimer;		//���S���o�̃A�j���[�V�����̃^�C�}�[
	const float DEATH_SPRITE_TIMER = 4;			//�A�j���[�V������؂�ւ���^�C�}�[
	int m_deathSpriteAnimNumber;					//�A�j���[�V�����̘A�Ԕԍ�
	static const int DEATH_SPRITE_ANIM_COUNT = 10;	//�A�j���[�V�����̐�
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, DEATH_SPRITE_ANIM_COUNT> m_deathAnimSprite;
	bool m_isFinishDeathAnimation;					//���S���̃A�j���[�V�������I��������B

	//�A�j���[�^�[
	std::shared_ptr<KuroEngine::ModelAnimator>m_modelAnimator;
	//���f���̃A�j���[�V�����p�^�[��
	enum ANIM_PATTERN { ANIM_PATTERN_WAIT, ANIM_PATTERN_INTEREST, ANIM_PATTERN_WALK, ANIM_PATTERN_CLEAR, ANIM_PATTERN_JUMP, ANIM_PATTERN_NUM };
	//���f���̃A�j���[�V������
	const std::array<std::string, ANIM_PATTERN_NUM>m_animNames =
	{
		"Wait","Interest","Walk","Happy","Jump"
	};
	static const int ANIM_INTEREST_CYCLE = 2;
	//�ҋ@�A�j���[�V�����L�����L������������̃J�E���^�[
	int m_animInterestCycleCounter = ANIM_INTEREST_CYCLE;

	//�A�j���[�V�����w��
	void AnimationSpecification(const KuroEngine::Vec3<float>& arg_beforePos, const KuroEngine::Vec3<float>& arg_newPos);

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
	void DrawUI();
	void Finalize();

	//�����蔻��N���X
	PlayerCollision m_collision;

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
	bool GetIsFinishDeathAnimation() { return m_isFinishDeathAnimation; }

	bool GetIsUnderGround() { return m_isUnderGround; }

	//���W��Ԃ��n�B
	KuroEngine::Vec3<float> GetNowPos() { return m_transform.GetPosWorld(); }
	KuroEngine::Vec3<float> GetOldPos() { return m_prevTransform.GetPosWorld(); }

	//�M�~�b�N�ɂ��ړ����I��点��B
	void FinishGimmickMove();

	//�_���[�W��^����B�Ƃ肠�����͂��ꂪ�Ă΂ꂽ��v���C���[�͎��ʁB
	void Damage();

	//�U�������H
	bool GetIsAttack() { return 0 < m_attackTimer; }

	void DisactiveLight()
	{
		m_growPlantPtLig.Disactive();
	}
	GrowPlantLight_Point GetGrowPlantLight() { return m_growPlantPtLig; }

	//�A�E�g���C���̓_����`�悷��Ƃ��Ɏg�p����l
	KuroEngine::Vec3<float> GetOutlineStandardVec() {
		return KuroEngine::Math::TransformVec3(KuroEngine::Vec3<float>(0, 0, 1), m_normalSpinQ);
	}

	//���𐶂₷���Ƃ̓����蔻��
	bool CheckHitGrassSphere(KuroEngine::Vec3<float> arg_enemyPos, KuroEngine::Vec3<float> arg_enemyUp, float arg_enemySize);

	Sphere m_sphere;
	float m_radius;

	void GetItemEffect(std::shared_ptr<ItemOnGame::ItemData> itemData)
	{
		HealData healData;
		switch (itemData->m_itemEnum)
		{
		case ITEM_NONE:
			break;
		case ITEM_HEAL:
			healData = dynamic_cast<ItemInterface<HealData>*>(itemData->m_itemInfomation)->m_itemData;
			break;
		default:
			break;
		}
	};

private:

	//�ړ�������B
	void Move(KuroEngine::Vec3<float>& arg_newPos);

	//�x�W�G�Ȑ������߂�B
	KuroEngine::Vec3<float> CalculateBezierPoint(float arg_time, KuroEngine::Vec3<float> arg_startPoint, KuroEngine::Vec3<float> arg_endPoint, KuroEngine::Vec3<float> arg_controlPoint);

	//�W�b�v���C�����̍X�V����
	void UpdateZipline();

	//���S���̍X�V����
	void UpdateDeath();

};

