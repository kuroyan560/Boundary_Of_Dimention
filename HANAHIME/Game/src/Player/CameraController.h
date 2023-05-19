#pragma once
#include"Common/Vec.h"
#include"Common/Angle.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"
#include"../Stage/Stage.h"
#include"CollisionDetectionOfRayAndMesh.h"

#include<memory>
namespace KuroEngine
{
	class Camera;
}

class CameraController : public KuroEngine::Debugger
{
	void OnImguiItems()override;

	//�Ώۂƒ����_�̑��ΓI�ȍ��W�I�t�Z�b�g
	KuroEngine::Vec3<float>m_gazePointOffset = { 0,0.5f,0 };
	//�Ǐ]�ΏۂƂ̑��ΓI�ȍ��W�I�t�Z�b�g�̍ŏ��ƍő�
	float m_posOffsetDepthMin = -10.0f;
	float m_posOffsetDepthMax = -0.1f;
	//X���p�x�i�����X���j�̍ŏ��ƍő�
	KuroEngine::Angle m_xAxisAngleMin = KuroEngine::Angle(-40);
	KuroEngine::Angle m_xAxisAngleMax = KuroEngine::Angle(40);
	//�J�����̊p�x �V��ɂ���Ƃ��o�[�W����
	KuroEngine::Angle m_xAxisAngleMinCeiling = KuroEngine::Angle(-40);
	KuroEngine::Angle m_xAxisAngleMaxCeiling = KuroEngine::Angle(40);

	//�J������Z�����ɉ�]������ʁB
	float m_rotateZ;

	struct Parameter
	{
		//�ΏۂƂ̑��ΓI��Z�I�t�Z�b�g
		float m_posOffsetZ = -10.0f;
		//X���p�x�i�����X���j
		KuroEngine::Angle m_xAxisAngle = KuroEngine::Angle(20);

		//Y���p�x
		KuroEngine::Angle m_yAxisAngle = KuroEngine::Angle(0);
	};
	//�p�����[�^�̏������l�i�f�t�H���g�l�j
	Parameter m_initializedParam;
	//���݂̃p�����[�^
	Parameter m_nowParam;

	//�n�`�ɓ������Ă��邩
	bool m_isHitTerrian;

	//�����̒n�`�ɓ������Ă��邩�B�������Ă����璍���_�����炷������B
	bool m_isHitUnderGroundTerrian;

	//�v���C���[��Y����]��ۑ����Ă����ϐ��B�v���C���[�����̕ǂɋ���Ƃ��͒����_�̈ړ���Y����]�ōs���̂ŁA�����_�ړ����I������瓮�������ʂ�߂����߁B
	float m_playerRotYStorage;
	float m_playerRotYLerp;
	const float PLAYER_TARGET_MOVE_SIDE = 0.8f;		//�v���C���[�̉��ʂ̒����_�ړ��̂Ƃ��̓���������E�B


	float m_rotateYLerpAmount;
	float m_cameraXAngleLerpAmount;

	//���삷��J�����̃|�C���^
	std::weak_ptr<KuroEngine::Camera>m_attachedCam;

	KuroEngine::Vec3<float> m_oldCameraWorldPos;
	KuroEngine::Transform m_cameraLocalTransform;	//�J�����̃��[�J���ł̉�]�ƈړ����v�Z����p�B
	KuroEngine::Transform m_camParentTransform;		//�v���C���[�̍��W�Ɖ�]��K��������p�B

	//�J�����̑O�������W�ړ���Lerp�l
	float m_camForwardPosLerpRate = 0.8f;

	//�J�����̍��W�Ǐ]��Lerp�l
	float m_camFollowLerpRate = 0.8f;

	//���݂̏㉺�����ő��삵�Ă������
	enum VERTICAL_MOVE { ANGLE, DIST }m_verticalControl = ANGLE;


public:
	//�R���X�g���N�^
	CameraController();

	void AttachCamera(std::shared_ptr<KuroEngine::Camera>arg_cam);

	void Init();
	void Update(KuroEngine::Vec3<float>arg_scopeMove, KuroEngine::Transform arg_targetPos, float& arg_playerRotY, float arg_cameraZ, const std::weak_ptr<Stage>arg_nowStage, bool arg_isCameraUpInverse, bool arg_isCameraDefaultPos, bool& arg_isHitUnderGround, bool arg_isMovePlayer, bool arg_isPlayerJump);

	const KuroEngine::Quaternion& GetPosRotate() {
		return m_camParentTransform.GetRotate();
	}

	std::weak_ptr<KuroEngine::Camera> GetCamera() { return m_attachedCam; }

private:

	//3�����x�N�g����2�����Ɏˉe����֐�
	inline KuroEngine::Vec2<float> Project3Dto2D(KuroEngine::Vec3<float> arg_vector3D, KuroEngine::Vec3<float> arg_basis1, KuroEngine::Vec3<float> arg_basis2) {
		
		//���x�N�g���𐳋K��
		arg_basis1.Normalize();
		arg_basis2.Normalize();

		//3�����x�N�g����2�����x�N�g���Ɏˉe
		float x = arg_vector3D.Dot(arg_basis1);
		float y = arg_vector3D.Dot(arg_basis2);

		return KuroEngine::Vec2<float>(x, y);
	}

	//�������̉����߂�����
	void PushBackGround(const CollisionDetectionOfRayAndMesh::MeshCollisionOutput& arg_output, const KuroEngine::Vec3<float> arg_pushBackPos, const KuroEngine::Transform& arg_targetPos, float& arg_playerRotY, bool arg_isCameraUpInverse);

};