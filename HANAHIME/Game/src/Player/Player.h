#pragma once
#include"KuroEngine.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"
#include"CameraController.h"

#include<memory>
namespace KuroEngine
{
	class Camera;
	class Model;
}

class Player : public KuroEngine::Debugger
{
	//�v���C���[�̃��f��
	std::shared_ptr<KuroEngine::Model>m_model;

	//�J�����̃��f���i�f�o�b�O�p�j
	std::shared_ptr<KuroEngine::Model>m_camModel;

	//�g�����X�t�H�[��
	KuroEngine::Transform m_transform;

	//�J�����C���X�^���X
	std::shared_ptr<KuroEngine::Camera>m_cam;

	//�J�����̃R���g���[���[
	CameraController m_camController;
	
	//�J�������x
	float m_camSensitivity = 1.0f;

	//Imgui�f�o�b�O�֐��I�[�o�[���C�h
	void OnImguiItems()override;

public:
	Player();
	void Init(KuroEngine::Transform arg_initTransform);
	void Update();
	void Draw(KuroEngine::Camera& arg_cam, bool arg_cameraDraw = false);
	void Finalize();

	std::weak_ptr<KuroEngine::Camera>GetCamera() { return m_cam; }

	//�J�����R���g���[���[�̃f�o�b�K�|�C���^�擾
	KuroEngine::Debugger* GetCameraControllerDebugger() { return &m_camController; }
};

