#pragma once
#include"Common/Vec.h"
#include"Common/Angle.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"
#include"../Stage/Stage.h"

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

	//�J�����̉�]�𐧌䂷��N�H�[�^�j�I��
	KuroEngine::Quaternion m_cameraQ;

	//���삷��J�����̃|�C���^
	std::weak_ptr<KuroEngine::Camera>m_attachedCam;

	KuroEngine::Vec3<float> m_oldCameraWorldPos;


public:
	//�R���X�g���N�^
	CameraController();

	void AttachCamera(std::shared_ptr<KuroEngine::Camera>arg_cam);

	void Init(const KuroEngine::Vec3<float>& arg_up, float arg_rotateY);
	void Update(KuroEngine::Vec3<float>arg_scopeMove, KuroEngine::Transform arg_targetPos, float& arg_playerRotY, float arg_cameraZ, const std::weak_ptr<Stage>arg_nowStage, bool arg_isCameraUpInverse, bool arg_isCameraDefaultPos, bool& arg_isHitUnderGround, bool arg_isMovePlayer);

	std::weak_ptr<KuroEngine::Camera> GetCamera() { return m_attachedCam; }

	KuroEngine::Quaternion GetCameraQ() { return m_cameraQ; }

private:

	//3�����x�N�g����2�����Ɏˉe����֐�
	KuroEngine::Vec2<float> Project3Dto2D(KuroEngine::Vec3<float> arg_vector3D, KuroEngine::Vec3<float> arg_basis1, KuroEngine::Vec3<float> arg_basis2) {

		//���x�N�g���𐳋K��
		arg_basis1.Normalize();
		arg_basis2.Normalize();

		//3�����x�N�g����2�����x�N�g���Ɏˉe
		float x = arg_vector3D.Dot(arg_basis1);
		float y = arg_vector3D.Dot(arg_basis2);

		return KuroEngine::Vec2<float>(x, y);
	}

};