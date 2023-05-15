#pragma once
#include "../../../../src/engine/Common/Transform.h"
#include "../../../../src/engine/ForUser/Timer.h"
#include "../../../../src/engine/Render/RenderObject/LightManager.h"
#include "PlayerMoveParticleOrb.h"
#include "PlayerMoveParticleIdle.h"
#include "PlayerMoveParticleSmoke.h"
#include <array>
#include <ForUser/Object/Model.h>

//�v���C���[���ړ�����Ƃ��̃p�[�e�B�N��
class PlayerMoveParticle {

private:

	PlayerMoveParticleOrb m_orb;		//�ړ����̃p�[�e�B�N��
	PlayerMoveParticleIdle m_idleOrb;	//�ҋ@���̃p�[�e�B�N��
	PlayerMoveParticleSmoke m_smoke;	//�ړ����̃p�[�e�B�N��

public:

	PlayerMoveParticle();

	//����������
	void Init();

	//�X�V����
	void Update();

	//�`�揈��
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//��������
	void GenerateOrb(const KuroEngine::Vec3<float>& arg_playerPos, const KuroEngine::Vec3<float>& arg_scatter, const KuroEngine::Vec3<float>& arg_vel = {});
	void GenerateIdle(const KuroEngine::Vec3<float>& arg_playerPos, const KuroEngine::Vec3<float>& arg_scatter);
	void GenerateSmoke(const KuroEngine::Vec3<float>& arg_playerPos, const KuroEngine::Vec3<float>& arg_scatter);


};