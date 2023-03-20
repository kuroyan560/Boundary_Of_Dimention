#pragma once
#include"KuroEngine.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"
#include"CameraController.h"
#include"../../../../src/engine/Render/RenderObject/ModelInfo/ModelMesh.h"

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

	//Y����]��
	float m_cameraRotY;	//�J������Y����]�ʁB���̊p�x�����ƂɃv���C���[�̈ړ������𔻒f����B
	float m_playerRotY;	//�v���C���[��Y����]�ʁB����͌����ڏ�ړ������Ɍ������邽�߁B���ʃx�N�g���Ƃ��ɉe�����o�Ȃ��悤�ɂ��Ȃ���΂Ȃ�Ȃ��B
	XMVECTOR m_cameraQ;	//�v���C���[��Y����]�ʂ𔲂����A�J���������݂̂��������ꍇ�̉�]

	//�ړ��x�N�g���̃X�J���[
	float m_moveScalar = 0.5f;

	//�ڒn�t���O
	bool m_onGround;		//�ڒn�t���O
	bool m_prevOnGround;	//1F�O�̐ڒn�t���O

	//Imgui�f�o�b�O�֐��I�[�o�[���C�h
	void OnImguiItems()override;

	struct HitCheckResult
	{
		KuroEngine::Vec3<float>m_interPos;
		KuroEngine::Vec3<float>m_terrianNormal;
		KuroEngine::Vec3<float>m_bottmRayTerrianNormal;
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
		CLIFF,	//�R�Ŗ������ƂɌ������Ĕ�΂����C�B�R���~���ۂɎg�p����B
	};
	/// <summary>
	/// ���C�ƃ��b�V���̓����蔻��
	/// </summary>
	/// <param name="arg_rayPos"> ���C�̎ˏo�n�_ </param>
	/// <param name="arg_rayDir"> ���C�̎ˏo���� </param>
	/// <param name="arg_targetMesh"> ������s���Ώۂ̃��b�V�� </param>
	/// <param name="arg_targetTransform"> ������s���Ώۂ̃g�����X�t�H�[�� </param>
	/// <returns> �����蔻�茋�� </returns>
	MeshCollisionOutput MeshCollision(const KuroEngine::Vec3<float>& arg_rayPos, const KuroEngine::Vec3<float>& arg_rayDir, KuroEngine::ModelMesh arg_targetMesh, KuroEngine::Transform arg_targetTransform);

	/// <summary>
	/// �d�S���W�����߂�B
	/// </summary>
	KuroEngine::Vec3<float> CalBary(const KuroEngine::Vec3<float>& PosA, const KuroEngine::Vec3<float>& PosB, const KuroEngine::Vec3<float>& PosC, const KuroEngine::Vec3<float>& TargetPos);

	//CastRay�ɓn���f�[�^�p�\����
	struct CastRayArgument {
		KuroEngine::ModelMesh m_mesh;				//������s���Ώۂ̃��b�V��
		KuroEngine::Transform m_targetTransform;	//������s���Ώۂ̃g�����X�t�H�[��
		bool& m_onGround;							//�ڒn�t���O
		bool& m_isHitWall;							//���C���ǂɓ����������ǂ���
		HitCheckResult& m_hitResult;				//�����蔻�茋�ʃf�[�^
		CastRayArgument(bool& arg_onGround, bool& arg_isHitWall, HitCheckResult& arg_hitResult) : m_onGround(arg_onGround), m_isHitWall(arg_isHitWall), m_hitResult(arg_hitResult) {};
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
	void CastRay(KuroEngine::Vec3<float>& arg_charaPos, const KuroEngine::Vec3<float>& arg_rayCastPos, const KuroEngine::Vec3<float>& arg_rayDir, float arg_rayLength, CastRayArgument arg_collisionData, RAY_ID arg_rayID);

};

