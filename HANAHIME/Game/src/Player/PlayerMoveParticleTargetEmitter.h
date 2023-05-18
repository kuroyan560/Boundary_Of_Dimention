#pragma once
#include "PlayerMoveParticleTarget.h"

//�v���C���[���S�[���Ƃ��̒����_������Ƃ��ɏo���p�[�e�B�N���̃G�~�b�^�[
class PlayerMoveParticleTargetEmitter {

private:

	static const int EMITTER_COUNT = 10;
	const float EMITTER_SIZE = 5.0f;

	//�G�~�b�^�[�̏��
	struct EmitterInfo {
		KuroEngine::Vec3<float> m_pos;
		KuroEngine::Vec3<float> m_moveDir;
		float m_speed;
		int m_timer;
		bool m_isAlive;
	};


};