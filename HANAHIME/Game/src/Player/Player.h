#pragma once
#include"KuroEngine.h"
#include"Common/Transform.h"
#include"ForUser/Debugger.h"

#include<memory>
namespace KuroEngine
{
	class Camera;
	class Model;
}

class Player : public KuroEngine::Debugger
{
	//���f��
	std::shared_ptr<KuroEngine::Model>m_model;

	//�g�����X�t�H�[��
	KuroEngine::Transform m_transform;

	//�J�����C���X�^���X
	std::shared_ptr<KuroEngine::Camera>m_cam;
	
	//�J�����ʒu�I�t�Z�b�g�i�f�t�H���g�l�j
	KuroEngine::Vec3<float>m_camPosOffsetDefault = { 0.0f,9.0f,-11.0f };
	//�J�����ʒu�I�t�Z�b�g
	KuroEngine::Vec3<float>m_camPosOffset = m_camPosOffsetDefault;

	//�J�������x
	float m_camSensitivity = 1.0f;

	//Imgui�f�o�b�O�֐��I�[�o�[���C�h
	void OnImguiItems()override;

public:
	Player();
	void Init(KuroEngine::Transform arg_initTransform);
	void Update();
	void Draw(KuroEngine::Camera& arg_cam);
	void Finalize();

	std::weak_ptr<KuroEngine::Camera>GetCamera() { return m_cam; }
};

