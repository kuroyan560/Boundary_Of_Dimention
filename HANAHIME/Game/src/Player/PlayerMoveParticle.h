#pragma once
#pragma once
#include "../../../../src/engine/Common/Transform.h"
#include "../../../../src/engine/ForUser/Timer.h"
#include "../../../../src/engine/Render/RenderObject/LightManager.h"
#include <array>
#include <ForUser/Object/Model.h>

//プレイヤーが移動するときのパーティクル
class PlayerMoveParticle {

private:

	static const int EXIT_TIMER = 120;
	struct ParticleData
	{

		KuroEngine::Vec3<float> m_st;
		KuroEngine::Transform m_transform;
		KuroEngine::Timer m_exitTimer;	 //消えるまでのタイマー
		bool m_isAlive;

		ParticleData() : m_isAlive(false), m_exitTimer(120) {};

	};

	static const int PARTICLE_COUNT = 256;
	std::array<ParticleData, 256> m_particle;	//パーティクル本体


	std::shared_ptr<KuroEngine::Model> m_model;


public:

	PlayerMoveParticle();

	//初期化処理
	void Init();

	//更新処理
	void Update();

	//描画処理
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//生成処理
	void Generate(const KuroEngine::Vec3<float>& arg_playerPos, const KuroEngine::Vec3<float>& arg_scatter);


private:

	/// <summary>
	/// カールノイズを求める。
	/// </summary>
	/// <param name="arg_pos"></param>
	/// <returns> 移動量 </returns>
	KuroEngine::Vec3<float> CurlNoise3D(const KuroEngine::Vec3<float>& arg_st, const KuroEngine::Vec3<float>& arg_pos);

	//hlslのfrac関数 modfを用いて小数点を取得するようにしている。
	float Frac(float arg_x);

	//3Dのランダムハッシュ関数 hlslで使用している奴をそのまま持ってきたため、乱数生成がfrac
	KuroEngine::Vec3<float> Random3D(KuroEngine::Vec3<float> arg_st);

	//ラープ関数 処理上tが1を超えてしまうため、新しく処理を追加しました。
	inline float Lerp(float arg_a, float arg_b, float arg_t) {
		return arg_a * (1 - arg_t) + arg_b * arg_t;
	}

	//格子のノイズを調べ、補間する。
	float Noise(KuroEngine::Vec3<float> arg_st);

	//3Dパーリンノイズ関数
	float PerlinNoise(KuroEngine::Vec3<float> arg_st, int arg_octaves, float arg_persistence, float arg_lacunarity, KuroEngine::Vec3<float> arg_pos);

};