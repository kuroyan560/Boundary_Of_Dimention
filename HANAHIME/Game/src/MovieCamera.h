#pragma once
#include"Common/Transform.h"
#include"Player/CameraController.h"

struct MovieCameraData
{
	KuroEngine::Transform transform;	//���̏ꏊ�Ɍ��������W
	bool skipInterpolationFlag;			//���W�Ɗp�x�̕�Ԃ��X�L�b�v���邩�ǂ���
	int interpolationTimer;				//
	int stopTimer;						//�J��������ʒu�ɂ��Ăǂꂭ�炢�~�܂邩
};

class MovieCamera
{
public:
	MovieCamera();
	void Update();

	std::weak_ptr<KuroEngine::Camera>GetCamera()
	{
		return m_cam;
	}
private:
	std::shared_ptr<KuroEngine::Camera>m_cam;
};

