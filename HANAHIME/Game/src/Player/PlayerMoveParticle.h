#pragma once
#include "../../../../src/engine/Common/Transform.h"
#include "../../../../src/engine/ForUser/Timer.h"
#include "../../../../src/engine/Render/RenderObject/LightManager.h"
#include "PlayerMoveParticleOrb.h"
#include "PlayerMoveParticleIdle.h"
#include "PlayerMoveParticleSmoke.h"
#include <array>
#include <ForUser/Object/Model.h>

//プレイヤーが移動するときのパーティクル
class PlayerMoveParticle {

private:

	PlayerMoveParticleOrb m_orb;		//移動時のパーティクル
	PlayerMoveParticleIdle m_idleOrb;	//待機中のパーティクル
	PlayerMoveParticleSmoke m_smoke;	//移動時のパーティクル

public:

	PlayerMoveParticle();

	//初期化処理
	void Init();

	//更新処理
	void Update();

	//描画処理
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//生成処理
	void GenerateOrb(const KuroEngine::Vec3<float>& arg_playerPos, const KuroEngine::Vec3<float>& arg_scatter, const KuroEngine::Vec3<float>& arg_vel = {});
	void GenerateIdle(const KuroEngine::Vec3<float>& arg_playerPos, const KuroEngine::Vec3<float>& arg_scatter);
	void GenerateSmoke(const KuroEngine::Vec3<float>& arg_playerPos, const KuroEngine::Vec3<float>& arg_scatter);


};