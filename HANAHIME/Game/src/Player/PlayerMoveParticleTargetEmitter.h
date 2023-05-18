#pragma once
#include "PlayerMoveParticleTarget.h"

//プレイヤーがゴールとかの注視点を見るときに出すパーティクルのエミッター
class PlayerMoveParticleTargetEmitter {

private:

	static const int EMITTER_COUNT = 10;
	const float EMITTER_SIZE = 5.0f;

	//エミッターの情報
	struct EmitterInfo {
		KuroEngine::Vec3<float> m_pos;
		KuroEngine::Vec3<float> m_moveDir;
		float m_speed;
		int m_timer;
		bool m_isAlive;
	};


};