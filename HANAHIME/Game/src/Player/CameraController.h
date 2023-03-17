#pragma once
#include"Common/Vec.h"
#include"Common/Angle.h"
#include"ForUser/Debugger.h"

#include<memory>
namespace KuroEngine
{
	class Camera;
}

class CameraController : public KuroEngine::Debugger
{
	void OnImguiItems()override;

public:
	struct Parameter
	{
		//�Ǐ]����Ώۂ���J�����܂ł̋���
		float m_distToTarget = 4.0f;
		//�����_�̍���
		float m_gazePointHeight = 0.0f;
		//�����_����J�����܂ł̋���
		float m_gazePointDist = 10.0f;

		//X���p�x�i�����X���j
		KuroEngine::Angle m_xAngle;
	};
	//�p�����[�^�̏������l�i�f�t�H���g�l�j
	Parameter m_initializedParam;
	//���݂̃p�����[�^
	Parameter m_nowParam;

	//�J�������W�ړ���Lerp�l
	float m_camPosLerpRate = 0.8f;
	//�J���������_�ړ���Lerp�l
	float m_camGazePointLerpRate = 0.8f;

	//�R���X�g���N�^
	CameraController();

	void Init();
	void Update(std::shared_ptr<KuroEngine::Camera>arg_cam, KuroEngine::Vec3<float>arg_targetPos, KuroEngine::Vec3<float>arg_frontVec);
};