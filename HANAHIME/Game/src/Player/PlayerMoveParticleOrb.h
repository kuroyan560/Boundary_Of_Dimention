#pragma once
#include "../../../../src/engine/Common/Transform.h"
#include "../../../../src/engine/ForUser/Timer.h"
#include "../../../../src/engine/Render/RenderObject/LightManager.h"
#include <array>
#include <ForUser/Object/Model.h>

class PlayerMoveParticleOrb {

private:

	enum STATUS {
		APPEAR,	//出現
		NORMAL,	//通常状態
		EXIT,	//消滅
		MAX,
	};

	//パーティクルの各状態のスケール
	const float PARTICLE_SCALE = 0.4f;
	const float RANDOM_PARTICLE_SCALE = 0.1f;//パーティクルの大きさをずらす量。
	const std::array<int, MAX> DEFAULT_PARTICLE_STATUS_TIMER = { 20, 30, 60 };	//デフォルトの各ステータスのタイマー
	const std::array<int, MAX> RANDOM_PARTICLE_STATUS_TIMER = { 5, 8, 15 };	//ランダムで各ステータスのタイマーをずらす量

	//パーティクル構造体
	struct ParticleData
	{

		KuroEngine::Vec3<float> m_st;
		KuroEngine::Transform m_transform;
		KuroEngine::Timer m_statusTimer;	 //各ステータスのタイマー
		std::array<int, MAX> m_statusTimerArray;
		STATUS m_particleStatus;
		float m_particleScale;
		bool m_isAlive;

		ParticleData() : m_isAlive(false), m_particleStatus(APPEAR) {};

	};

	static const int PARTICLE_COUNT = 1024;
	std::array<ParticleData, PARTICLE_COUNT> m_particle;	//パーティクル本体


	std::shared_ptr<KuroEngine::Model> m_model;


public:

	PlayerMoveParticleOrb();

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
	KuroEngine::Vec3<float> CurlNoise3D(const KuroEngine::Vec3<float>& arg_st, const KuroEngine::Vec3<float>& arg_pos, bool arg_isExit);

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