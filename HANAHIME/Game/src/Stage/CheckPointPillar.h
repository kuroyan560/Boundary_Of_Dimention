#pragma once
#include "../../../../src/engine/KuroEngine.h"
#include "../../../../src/engine/ForUser/Object/Model.h"
#include "../../../../src/engine/ForUser/Timer.h"
#include <Render/RenderObject/LightManager.h>
#include <memory>

namespace KuroEngine
{
	class Model;
	class Camera;
	class LightManager;
}

//���̃`�F�b�N�|�C���g�̉~��
class CheckPointPillar {

private:

	std::shared_ptr<KuroEngine::Model> m_pillarModel;	//�~���̃��f��
	KuroEngine::Transform m_transform;					//�~���̕`����
	bool m_isDraw;										//�`�悷�邩�H
	float m_alpha;										//�~���̃A���t�@
	bool m_isFirstFrame;								//�ŏ��̃t���[�����ǂ����B�ŏ��̃t���[���������W��ۑ�����B

	//�e�X�e�[�^�X�̃^�C�}�[
	KuroEngine::Timer m_appearModeTimer;
	const float APPEAR_MODE_TIMER = 20.0f;
	KuroEngine::Timer m_exitModeTimer;
	const float EXIT_MODE_TIMER = 30.0f;

	enum STATUS {
		NORMAL,
		EXIT,
		APPEAR,
	}m_status;

public:

	CheckPointPillar();
	void Init();
	void Update(const KuroEngine::Vec3<float>& arg_playerPos);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, std::weak_ptr<KuroEngine::DepthStencil>arg_ds);

};