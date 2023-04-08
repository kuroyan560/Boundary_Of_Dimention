#pragma once
#include"Common/Vec.h"
#include"Common/Angle.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"

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
	KuroEngine::Angle m_xAxisAngleMin = KuroEngine::Angle(10);
	KuroEngine::Angle m_xAxisAngleMax = KuroEngine::Angle(20);

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

	//���삷��J�����̃|�C���^
	std::weak_ptr<KuroEngine::Camera>m_attachedCam;

	//�v���C���[�̃g�����X�t�H�[�����Ԃ��Ȃ���R�s�[
	KuroEngine::Transform m_copyPlayerTransform;
	//�v���C���[���W�R�s�[��lerp��
	float m_playerPosLerpRate = 0.2f;
	//�v���C���[�N�H�[�^�j�I���R�s�[��lerp��
	float m_playerQuaternionLerpRate = 0.2f;

	//�J�����R���g���[���[�̃g�����X�t�H�[��
	KuroEngine::Transform m_controllerTransform;

	//���݂̏㉺�����ő��삵�Ă������
	enum VERTICAL_MOVE { ANGLE, DIST }m_verticalControl = ANGLE;

public:
	//�R���X�g���N�^
	CameraController();

	void AttachCamera(std::shared_ptr<KuroEngine::Camera>arg_cam);

	void Init(const KuroEngine::Vec3<float>& arg_playerPos, const KuroEngine::Quaternion& arg_playerRotate);
	void Update(KuroEngine::Vec3<float>arg_scopeMove, const KuroEngine::Vec3<float>& arg_playerPos, const KuroEngine::Quaternion& arg_playerRotate, float arg_cameraY, bool arg_isNoLerp);

	const KuroEngine::Quaternion& GetPosRotate() {
		return m_controllerTransform.GetRotate();
	}
};