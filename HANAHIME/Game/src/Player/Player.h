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
	class LightManager;
}

class Stage;
class Terrian;

class Player : public KuroEngine::Debugger
{
	//�v���C���[�̃��f��
	std::shared_ptr<KuroEngine::Model>m_model;
	std::shared_ptr<KuroEngine::Model>m_axisModel;

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

	float m_moveScalar = 0.5f;

	//Imgui�f�o�b�O�֐��I�[�o�[���C�h
	void OnImguiItems()override;

	bool HitCheck(const KuroEngine::Vec3<float>arg_from, const KuroEngine::Vec3<float> arg_to, const std::vector<Terrian>& arg_terrianArray, KuroEngine::Vec3<float>* arg_terrianNormal = nullptr);

public:
	Player();
	void Init(KuroEngine::Transform arg_initTransform);
	void Update(const std::weak_ptr<Stage>arg_nowStage);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, bool arg_cameraDraw = false);
	void Finalize();

	std::weak_ptr<KuroEngine::Camera>GetCamera() { return m_cam; }

	//�J�����R���g���[���[�̃f�o�b�K�|�C���^�擾
	KuroEngine::Debugger* GetCameraControllerDebugger() { return &m_camController; }
};

