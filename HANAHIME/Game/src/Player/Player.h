#pragma once
#include"KuroEngine.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"
#include"CameraController.h"
#include"../../../../src/engine/Render/RenderObject/ModelInfo/ModelMesh.h"
#include"../Stage/Stage.h"
#include"Render/RenderObject/Light.h"

#include<memory>
namespace KuroEngine
{
	class Camera;
	class Model;
	class LightManager;
}

class Stage;
struct Terrian;

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
	KuroEngine::Transform m_transform;

	//�ړ���
	KuroEngine::Vec3<float> m_rowMoveVec;

	//�J�����C���X�^���X
	std::shared_ptr<KuroEngine::Camera>m_cam;

	//�J�����̃R���g���[���[
	CameraController m_camController;

	//�J�������x
	float m_camSensitivity = 1.0f;

	//���𐶂₷�ۂ̎U�炵�ʁB
	KuroEngine::Vec2<float> m_grassPosScatter = KuroEngine::Vec2<float>(2.0f, 2.0f);

	//Y����]��
	float m_cameraRotY;	//�J������Y����]�ʁB���̊p�x�����ƂɃv���C���[�̈ړ������𔻒f����B
	float m_cameraRotYStorage;	//�J������Y����]�ʁB
	float m_playerRotY;	//�v���C���[��Y����]�ʁB����͌����ڏ�ړ������Ɍ������邽�߁B���ʃx�N�g���Ƃ��ɉe�����o�Ȃ��悤�ɂ��Ȃ���΂Ȃ�Ȃ��B
	XMVECTOR m_cameraQ;	//�v���C���[��Y����]�ʂ𔲂����A�J���������݂̂��������ꍇ�̉�]
	XMVECTOR m_moveQ;	//�v���C���[��Y����]�ʂ𔲂����A�v���C���[�̈ړ������݂̂��������ꍇ�̉�]
	XMVECTOR m_normalSpinQ;	//�f�t�H���g�̏�x�N�g���ƌ��݂���n�`�̉�]�ʂ�����킵���N�H�[�^�j�I��
	bool m_isNoLerpCamera;	//�J�������Ԃ��邩�ǂ���

	//�ړ��x�N�g���̃X�J���[
	KuroEngine::Vec3<float> m_moveSpeed;			//�ړ����x
	float m_moveAccel = 0.05f;
	float m_maxSpeed = 0.5f;
	float m_brake = 0.07f;

	//�ǈړ��̋���
	const float WALL_JUMP_LENGTH = 6.0f;

	//�ڒn�t���O
	bool m_isFirstOnGround;	//�J�n���ɋ󒆂���n�܂�̂ŁA���n�ς݂��Ƃ������Ƃ𔻒f����p�ϐ��B
	bool m_onGround;		//�ڒn�t���O
	bool m_prevOnGround;	//1F�O�̐ڒn�t���O

	//Imgui�f�o�b�O�֐��I�[�o�[���C�h
	void OnImguiItems()override;

	//�v���C���[�̓����̃X�e�[�^�X
	enum class PLAYER_MOVE_STATUS {
		NONE,
		MOVE,	//�ʏ�ړ���
		JUMP,	//�W�����v��(��Ԓ�)
	}m_playerMoveStatus;

	//�v���C���[�̃W�����v�Ɋւ���ϐ�
	KuroEngine::Vec3<float> m_jumpStartPos;
	KuroEngine::Vec3<float> m_jumpEndPos;
	KuroEngine::Vec3<float> m_bezierCurveControlPos;
	XMVECTOR m_jumpStartQ;
	XMVECTOR m_jumpEndQ;
	float m_jumpTimer;
	const float JUMP_TIMER = 0.07f;


	struct HitCheckResult
	{
		KuroEngine::Vec3<float>m_terrianNormal;
	};
	bool HitCheckAndPushBack(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, const std::vector<Terrian>& arg_terrianArray, HitCheckResult* arg_hitInfo = nullptr);

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

	KuroEngine::Transform& GetTransform() { return m_transform; }
	KuroEngine::Vec2<float>& GetGrassPosScatter() { return m_grassPosScatter; }

	//�_�����Q�b�^
	KuroEngine::Light::Point* GetPointLig() { return &m_ptLig; }
	bool GetOnGround() { return m_onGround; }
	bool GetIsStatusMove() { return m_playerMoveStatus == PLAYER_MOVE_STATUS::MOVE; }

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

		CHECK_CLIFF,	//�R���ǂ������`�F�b�N����p
		GROUND,	//�n��������Ĕ�΂����C�B�ݒu����Ŏg�p����B
		AROUND,	//���͂Ɍ������Ĕ�΂����C�B�ǂ̂ڂ蔻��Ŏg�p����B

	};
	/// <summary>
	/// ���C�ƃ��b�V���̓����蔻��
	/// </summary>
	/// <param name="arg_rayPos"> ���C�̎ˏo�n�_ </param>
	/// <param name="arg_rayDir"> ���C�̎ˏo���� </param>
	/// <param name="arg_targetMesh"> ������s���Ώۂ̃��b�V�� </param>
	/// <param name="arg_targetTransform"> ������s���Ώۂ̃g�����X�t�H�[�� </param>
	/// <returns> �����蔻�茋�� </returns>
	MeshCollisionOutput MeshCollision(const KuroEngine::Vec3<float>& arg_rayPos, const KuroEngine::Vec3<float>& arg_rayDir, std::vector<Terrian::Polygon>& arg_targetMesh, KuroEngine::Transform arg_targetTransform);

	/// <summary>
	/// �d�S���W�����߂�B
	/// </summary>
	KuroEngine::Vec3<float> CalBary(const KuroEngine::Vec3<float>& PosA, const KuroEngine::Vec3<float>& PosB, const KuroEngine::Vec3<float>& PosC, const KuroEngine::Vec3<float>& TargetPos);

	//�Փ˓_�p�\����
	struct ImpactPointData {
		KuroEngine::Vec3<float> m_impactPos;
		KuroEngine::Vec3<float> m_normal;
		ImpactPointData(KuroEngine::Vec3<float> arg_impactPos, KuroEngine::Vec3<float> arg_normal) : m_impactPos(arg_impactPos), m_normal(arg_normal) {};
	};

	//CastRay�ɓn���f�[�^�p�\����
	struct CastRayArgument {
		std::vector<Terrian::Polygon> m_mesh;		//������s���Ώۂ̃��b�V��
		KuroEngine::Transform m_targetTransform;	//������s���Ώۂ̃g�����X�t�H�[��
		std::vector<ImpactPointData> m_impactPoint;	//�O�㍶�E�̃��C�̓��������n�_�B
		bool m_onGround;							//�ڒn�t���O
		KuroEngine::Vec3<float> m_bottomTerrianNormal;
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
	bool CastRay(KuroEngine::Vec3<float>& arg_charaPos, const KuroEngine::Vec3<float>& arg_rayCastPos, const KuroEngine::Vec3<float>& arg_rayDir, float arg_rayLength, CastRayArgument& arg_collisionData, RAY_ID arg_rayID);

	//�ړ�������B
	void Move(KuroEngine::Vec3<float>& arg_newPos);

	//�����蔻��
	void CheckHit(KuroEngine::Vec3<float>& arg_frompos, KuroEngine::Vec3<float>& arg_nowpos, const std::weak_ptr<Stage>arg_nowStage);

	//�x�W�G�Ȑ������߂�B
	KuroEngine::Vec3<float> CalculateBezierPoint(float arg_time, KuroEngine::Vec3<float> arg_startPoint, KuroEngine::Vec3<float> arg_endPoint, KuroEngine::Vec3<float> arg_controlPoint);

	//�ړ������𐳂��������邽�߂̏���
	void AdjustCaneraRotY(const KuroEngine::Vec3<float>& arg_nowUp, const KuroEngine::Vec3<float>& arg_nextUp);

};

