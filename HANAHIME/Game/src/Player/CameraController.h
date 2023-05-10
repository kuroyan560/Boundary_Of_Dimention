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
	KuroEngine::Angle m_xAxisAngleMin = KuroEngine::Angle(10);
	KuroEngine::Angle m_xAxisAngleMax = KuroEngine::Angle(20);

	//�J������X����]
	float m_oldAngleX;
	float m_angleX;
	float m_distanceZ;

	//���z�I�ȃJ�����̃g�����X�t�H�[�� �J�����͂��̒l�Ɍ������ĕ�Ԃ���B
	KuroEngine::Transform m_baseTransform;

	//���삷��J�����̃|�C���^
	KuroEngine::Vec3<float> m_oldPos;
	std::weak_ptr<KuroEngine::Camera>m_attachedCam;

	KuroEngine::Transform m_cameraMoveTransform;
	KuroEngine::Transform m_playerTransform;

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
	void Update(KuroEngine::Vec3<float> arg_scopeMove, KuroEngine::Transform arg_playerTransform, float arg_cameraZ, const std::weak_ptr<Stage> arg_nowStage);

	std::weak_ptr<KuroEngine::Camera> GetCamera() { return m_attachedCam; }

private:

	//�J�������ʒu�ɃZ�b�g�B
	void SetCameraPos();

};