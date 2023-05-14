#include "PlayerMoveParticle.h"
#include "FrameWork/Importer.h"
#include "../Graphics/BasicDraw.h"
#include "../../../../src/engine/KuroEngine.h"

PlayerMoveParticle::PlayerMoveParticle()
{

	m_model = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "ParticleSphere.glb");

}

void PlayerMoveParticle::Init()
{

	for (auto& index : m_particle) {

		index.m_isAlive = false;
		index.m_exitTimer.Reset();

	}

}

void PlayerMoveParticle::Update()
{

	for (auto& index : m_particle) {

		if (!index.m_isAlive) continue;

		//ノイズにより移動量を求める。
		KuroEngine::Vec3<float> move = CurlNoise3D(index.m_st, index.m_transform.GetPos());

		//移動させる。
		index.m_transform.SetPos(index.m_transform.GetPos() + move);

		//パーティクルの状態を変化させるタイマーを更新。
		index.m_exitTimer.UpdateTimer();

		//現在のタイマーの割合。
		float timerRate = index.m_exitTimer.GetTimeRate();

		//ステータスごとに更新。
		switch (index.m_particleStatus)
		{
		case STATUS::APPEAR:
		{

			//スケールを更新。
			index.m_transform.SetScale(KuroEngine::Math::Ease(KuroEngine::In, KuroEngine::Sine, timerRate, 0.0f, 1.0f) * PARTICLE_SCALE);

			//タイマーが終わったら次のステータスへ。
			if (index.m_exitTimer.IsTimeUpOnTrigger()) {
				//次へ
				index.m_particleStatus = STATUS::NORMAL;
				//タイマーも初期化。
				index.m_exitTimer.Reset(PARTICLE_STATUS_TIMER[index.m_particleStatus]);
			}
		}
		break;
		case STATUS::NORMAL:
		{
			//タイマーが終わったら次のステータスへ。
			if (index.m_exitTimer.IsTimeUpOnTrigger()) {
				//次へ
				index.m_particleStatus = STATUS::EXIT;
				//タイマーも初期化。
				index.m_exitTimer.Reset(PARTICLE_STATUS_TIMER[index.m_particleStatus]);
			}
		}
		break;
		case STATUS::EXIT:
		{

			index.m_transform.SetScale(PARTICLE_SCALE - KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Sine, timerRate, 0.0f, 1.0f) * PARTICLE_SCALE);

			//タイマーが終わったら次のステータスへ。
			if (index.m_exitTimer.IsTimeUpOnTrigger()) {
				//終わり
				index.m_isAlive = false;
			}
		}
		break;
		default:
			break;
		}


	}



}

void PlayerMoveParticle::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{

	for (auto& index : m_particle) {

		if (!index.m_isAlive) continue;

		IndividualDrawParameter edgeColor = IndividualDrawParameter::GetDefault();

		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			m_model,
			index.m_transform,
			edgeColor,
			KuroEngine::AlphaBlendMode_None);

	}

}

void PlayerMoveParticle::Generate(const KuroEngine::Vec3<float>& arg_playerPos, const KuroEngine::Vec3<float>& arg_scatter)
{

	for (auto& index : m_particle) {

		if (index.m_isAlive) continue;

		//プレイヤーのスケール分散らした位置にパーティクルを生成。
		KuroEngine::Vec3<float> scatter = KuroEngine::GetRand(-arg_scatter, arg_scatter);

		index.m_transform.SetPos(arg_playerPos + scatter);
		index.m_transform.SetScale(0.0f);
		index.m_isAlive = true;
		index.m_exitTimer.Reset(PARTICLE_STATUS_TIMER[APPEAR]);
		index.m_particleStatus = STATUS::APPEAR;

		//パーティクルをいい感じに散らばらせるために、それぞれに乱数を持たせる。
		const float ST = 256;
		index.m_st = KuroEngine::GetRand(KuroEngine::Vec3<float>(ST, ST, ST));

		break;

	}

}

KuroEngine::Vec3<float> PlayerMoveParticle::CurlNoise3D(const KuroEngine::Vec3<float>& arg_st, const KuroEngine::Vec3<float>& arg_pos)
{

	const float epsilon = 0.01f;

	int octaves = 4; // オクターブ数
	float persistence = 0.5; // 持続度
	float lacunarity = 1.0; // ラクナリティ

	//ノイズの中心
	float noiseCenter = PerlinNoise(arg_st, octaves, persistence, lacunarity, arg_pos);

	//各軸にちょっとだけずらした値を求める。x + h
	float noiseX = PerlinNoise(arg_st, octaves, persistence, lacunarity, arg_pos + KuroEngine::Vec3<float>(epsilon, 0, 0));
	float noiseY = PerlinNoise(arg_st, octaves, persistence, lacunarity, arg_pos + KuroEngine::Vec3<float>(0, epsilon, 0));
	float noiseZ = PerlinNoise(arg_st, octaves, persistence, lacunarity, arg_pos + KuroEngine::Vec3<float>(0, 0, epsilon));

	//微分を求める。 f(x + h) - f(x) / h
	float dNoiseX = (noiseX - noiseCenter) / epsilon;
	float dNoiseY = (noiseY - noiseCenter) / epsilon;
	float dNoiseZ = (noiseZ - noiseCenter) / epsilon;

	//ベクトルを回転させる。
	KuroEngine::Vec3<float> vel;
	vel.x = dNoiseY - dNoiseZ;
	vel.y = dNoiseZ - dNoiseX;
	vel.z = dNoiseX - dNoiseY;

	return vel;

}

float PlayerMoveParticle::Frac(float arg_x)
{
	float intpart;
	float fracpart = std::modf(arg_x, &intpart);
	return fracpart;
}

KuroEngine::Vec3<float> PlayerMoveParticle::Random3D(KuroEngine::Vec3<float> arg_st)
{
	KuroEngine::Vec3<float> seed =
		KuroEngine::Vec3<float>(arg_st.Dot(KuroEngine::Vec3<float>(127.1f, 311.7f, 523.3f)),
			arg_st.Dot(KuroEngine::Vec3<float>(269.5f, 183.3f, 497.5f)),
			arg_st.Dot(KuroEngine::Vec3<float>(419.2f, 371.9f, 251.6f)));
	return KuroEngine::Vec3<float>(-1.0f + 2.0f * Frac(sinf(seed.x) * 43758.5453123f), -1.0f + 2.0f * Frac(sinf(seed.y) * 43758.5453123f), -1.0f + 2.0f * Frac(sinf(seed.z) * 43758.5453123f));
}

float PlayerMoveParticle::Noise(KuroEngine::Vec3<float> arg_st)
{
	KuroEngine::Vec3<float> intValue = { std::floor(arg_st.x) ,std::floor(arg_st.y) ,std::floor(arg_st.z) };
	KuroEngine::Vec3<float> floatValue = { Frac(arg_st.x) ,Frac(arg_st.y) ,Frac(arg_st.z) };

	//八つの隣接点の座標を求める。
	KuroEngine::Vec3<float> u;
	u.x = floatValue.x * floatValue.x * (3.0f - 2.0f * floatValue.x);
	u.y = floatValue.y * floatValue.y * (3.0f - 2.0f * floatValue.y);
	u.z = floatValue.z * floatValue.z * (3.0f - 2.0f * floatValue.z);

	//各隣接点でのノイズを求める。
	float center = Random3D(intValue).Dot(floatValue - KuroEngine::Vec3<float>(0, 0, 0));
	float right = Random3D(intValue + KuroEngine::Vec3<float>(1, 0, 0)).Dot(floatValue - KuroEngine::Vec3<float>(1, 0, 0));
	float top = Random3D(intValue + KuroEngine::Vec3<float>(0, 1, 0)).Dot(floatValue - KuroEngine::Vec3<float>(0, 1, 0));
	float rightTop = Random3D(intValue + KuroEngine::Vec3<float>(1, 1, 0)).Dot(floatValue - KuroEngine::Vec3<float>(1, 1, 0));
	float front = Random3D(intValue + KuroEngine::Vec3<float>(0, 0, 1)).Dot(floatValue - KuroEngine::Vec3<float>(0, 0, 1));
	float rightFront = Random3D(intValue + KuroEngine::Vec3<float>(1, 0, 1)).Dot(floatValue - KuroEngine::Vec3<float>(1, 0, 1));
	float topFront = Random3D(intValue + KuroEngine::Vec3<float>(0, 1, 1)).Dot(floatValue - KuroEngine::Vec3<float>(0, 1, 1));
	float rightTopFront = Random3D(intValue + KuroEngine::Vec3<float>(1, 1, 1)).Dot(floatValue - KuroEngine::Vec3<float>(1, 1, 1));

	//ノイズ値を補間する。
	float x1 = Lerp(center, right, u.x);
	float x2 = Lerp(top, rightTop, u.x);
	float y1 = Lerp(front, rightFront, u.x);
	float y2 = Lerp(topFront, rightTopFront, u.x);

	float xy1 = Lerp(x1, x2, u.y);
	float xy2 = Lerp(y1, y2, u.y);

	return Lerp(xy1, xy2, u.z);
}

float PlayerMoveParticle::PerlinNoise(KuroEngine::Vec3<float> arg_st, int arg_octaves, float arg_persistence, float arg_lacunarity, KuroEngine::Vec3<float> arg_pos)
{

	float amplitude = 1.0;

	//プレイヤーのワールド座標に基づくノイズ生成
	KuroEngine::Vec3<float> worldSpaceCoords = arg_st + arg_pos / 100.0f;

	float noiseValue = 0;

	float frequency = 2.0f;
	float localAmplitude = amplitude;
	float sum = 0.0;
	float maxValue = 0.0;

	for (int i = 0; i < arg_octaves; ++i)
	{
		sum += localAmplitude * Noise(worldSpaceCoords * frequency);
		maxValue += localAmplitude;

		localAmplitude *= arg_persistence;
		frequency *= arg_lacunarity;
	}

	noiseValue = (sum / maxValue + 1.0) * 0.5; //ノイズ値を0.0から1.0の範囲に再マッピング


	return noiseValue;

}
