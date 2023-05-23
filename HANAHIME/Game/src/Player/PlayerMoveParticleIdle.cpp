#include "PlayerMoveParticleIdle.h"
#include "FrameWork/Importer.h"
#include "../Graphics/BasicDraw.h"
#include "../../../../src/engine/KuroEngine.h"
#include "../../../../src/engine/Render/RenderObject/Camera.h"
#include "../../../../src/engine/ForUser/DrawFunc/3D/DrawFunc3D.h"
#include "../TimeScaleMgr.h"

PlayerMoveParticleIdle::PlayerMoveParticleIdle()
{

	m_model = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "MoveParticle.glb");

}

void PlayerMoveParticleIdle::Init()
{

	for (auto& index : m_particle) {

		index.m_isAlive = false;
		index.m_statusTimer.Reset();

	}

}

void PlayerMoveParticleIdle::Update()
{

	for (auto& index : m_particle) {

		if (!index.m_isAlive) continue;

		//�m�C�Y�ɂ��ړ��ʂ����߂�B
		KuroEngine::Vec3<float> move = CurlNoise3D(index.m_st, index.m_transform.GetPos(), index.m_particleStatus == EXIT) + index.m_vel;

		//�ړ�������B
		index.m_transform.SetPos(index.m_transform.GetPos() + move * TimeScaleMgr::s_inGame.GetTimeScale());

		//�n���ꂽ�ړ��ʂ����炷�B
		if (0.0f < index.m_vel.Length()) {
			index.m_vel = KuroEngine::Math::Lerp(index.m_vel, KuroEngine::Vec3<float>(0, 0, 0), 0.06f);
		}

		//�p�[�e�B�N���̏�Ԃ�ω�������^�C�}�[���X�V�B
		index.m_statusTimer.UpdateTimer(TimeScaleMgr::s_inGame.GetTimeScale());

		//���݂̃^�C�}�[�̊����B
		float timerRate = index.m_statusTimer.GetTimeRate();

		//�X�e�[�^�X���ƂɍX�V�B
		switch (index.m_particleStatus)
		{
		case STATUS::APPEAR:
		{

			//�X�P�[�����X�V�B
			index.m_transform.SetScale(KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Cubic, timerRate, 0.0f, 1.0f) * index.m_particleScale);

			//�^�C�}�[���I������玟�̃X�e�[�^�X�ցB
			if (index.m_statusTimer.IsTimeUpOnTrigger()) {
				//����
				index.m_particleStatus = STATUS::NORMAL;
				//�^�C�}�[���������B
				index.m_statusTimer.Reset(index.m_statusTimerArray[index.m_particleStatus]);
			}
		}
		break;
		case STATUS::NORMAL:
		{
			//�^�C�}�[���I������玟�̃X�e�[�^�X�ցB
			if (index.m_statusTimer.IsTimeUpOnTrigger()) {
				//����
				index.m_particleStatus = STATUS::EXIT;
				//�^�C�}�[���������B
				index.m_statusTimer.Reset(index.m_statusTimerArray[index.m_particleStatus]);
			}
		}
		break;
		case STATUS::EXIT:
		{

			index.m_transform.SetScale(index.m_particleScale - KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Sine, timerRate, 0.0f, 1.0f) * index.m_particleScale);

			//�^�C�}�[���I������玟�̃X�e�[�^�X�ցB
			if (index.m_statusTimer.IsTimeUpOnTrigger()) {
				//�I���
				index.m_isAlive = false;
			}
		}
		break;
		default:
			break;
		}


	}



}

void PlayerMoveParticleIdle::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	std::vector<KuroEngine::Matrix> mat;

	for (auto& index : m_particle) {

		if (!index.m_isAlive) continue;

		//�r���{�[�h�ɂ���B
		KuroEngine::Vec3<float> cameraDir = (arg_cam.GetTransform().GetPos() - index.m_transform.GetPos()).GetNormal();

		//�f�t�H���g�̌����Ƃ̍�������]������B
		KuroEngine::Vec3<float> defDir = { 0,1,0 };
		KuroEngine::Vec3<float> axis = defDir.Cross(cameraDir);
		float angle = std::acos(defDir.Dot(cameraDir));

		//��]�����߂�B
		if (0 < axis.Length()) {

			auto q = DirectX::XMQuaternionRotationAxis(axis, angle);
			index.m_transform.SetRotate(q);

		}

		mat.emplace_back(index.m_transform.GetMatWorld());

	}

	if (0 < static_cast<int>(mat.size())) {
		BasicDraw::Instance()->InstancingDraw_NoOutline(arg_cam, arg_ligMgr, m_model, mat, IndividualDrawParameter::GetDefault(), true, KuroEngine::AlphaBlendMode::AlphaBlendMode_Add);
	}

}

void PlayerMoveParticleIdle::Generate(const KuroEngine::Vec3<float>& arg_playerPos, const KuroEngine::Vec3<float>& arg_scatter)
{

	for (auto& index : m_particle) {

		if (index.m_isAlive) continue;

		index.m_transform.SetPos(arg_playerPos + arg_scatter);
		index.m_transform.SetScale(0.0f);
		index.m_isAlive = true;
		//index.m_vel = arg_vel;

		//�p�[�e�B�N���̊e�X�e�[�^�X�̃^�C�}�[��ݒ�B
		index.m_statusTimerArray = DEFAULT_PARTICLE_STATUS_TIMER;
		index.m_statusTimerArray[0] += KuroEngine::GetRand(-RANDOM_PARTICLE_STATUS_TIMER[0], RANDOM_PARTICLE_STATUS_TIMER[0]);
		index.m_statusTimerArray[1] += KuroEngine::GetRand(-RANDOM_PARTICLE_STATUS_TIMER[1], RANDOM_PARTICLE_STATUS_TIMER[1]);
		index.m_statusTimerArray[2] += KuroEngine::GetRand(-RANDOM_PARTICLE_STATUS_TIMER[2], RANDOM_PARTICLE_STATUS_TIMER[2]);

		index.m_statusTimer.Reset(index.m_statusTimerArray[APPEAR]);
		index.m_particleStatus = STATUS::APPEAR;
		index.m_particleScale = PARTICLE_SCALE + KuroEngine::GetRand(-RANDOM_PARTICLE_SCALE, RANDOM_PARTICLE_SCALE);

		//�p�[�e�B�N�������������ɎU��΂点�邽�߂ɁA���ꂼ��ɗ�������������B
		const float ST = 1024;
		index.m_st = KuroEngine::GetRand(KuroEngine::Vec3<float>(ST, ST, ST));

		break;

	}

}

KuroEngine::Vec3<float> PlayerMoveParticleIdle::CurlNoise3D(const KuroEngine::Vec3<float>& arg_st, const KuroEngine::Vec3<float>& arg_pos, bool arg_isExit)
{

	const float epsilon = 0.01f;

	int octaves = 4; //�I�N�^�[�u��
	float persistence = 0.5; //�����x
	float lacunarity = 2.0f; //���N�i���e�B

	//�m�C�Y�̒��S
	float noiseCenter = PerlinNoise(arg_st, octaves, persistence, lacunarity, arg_pos);

	//�e���ɂ�����Ƃ������炵���l�����߂�Bx + h
	float noiseX = PerlinNoise(arg_st, octaves, persistence, lacunarity, arg_pos + KuroEngine::Vec3<float>(epsilon, 0, 0));
	float noiseY = PerlinNoise(arg_st, octaves, persistence, lacunarity, arg_pos + KuroEngine::Vec3<float>(0, epsilon, 0));
	float noiseZ = PerlinNoise(arg_st, octaves, persistence, lacunarity, arg_pos + KuroEngine::Vec3<float>(0, 0, epsilon));

	//���������߂�B f(x + h) - f(x) / h
	float dNoiseX = (noiseX - noiseCenter) / epsilon;
	float dNoiseY = (noiseY - noiseCenter) / epsilon;
	float dNoiseZ = (noiseZ - noiseCenter) / epsilon;

	//�x�N�g������]������B
	KuroEngine::Vec3<float> vel;
	vel.x = dNoiseY - dNoiseZ;
	vel.y = dNoiseZ - dNoiseX;
	vel.z = dNoiseX - dNoiseY;

	return vel * (1.0f + (arg_isExit ? 1.0f : 0.0f));

}

float PlayerMoveParticleIdle::Frac(float arg_x)
{
	float intpart;
	float fracpart = std::modf(arg_x, &intpart);
	return fracpart;
}

KuroEngine::Vec3<float> PlayerMoveParticleIdle::Random3D(KuroEngine::Vec3<float> arg_st)
{
	KuroEngine::Vec3<float> seed =
		KuroEngine::Vec3<float>(arg_st.Dot(KuroEngine::Vec3<float>(127.1f, 311.7f, 523.3f)),
			arg_st.Dot(KuroEngine::Vec3<float>(269.5f, 183.3f, 497.5f)),
			arg_st.Dot(KuroEngine::Vec3<float>(419.2f, 371.9f, 251.6f)));
	return KuroEngine::Vec3<float>(-1.0f + 2.0f * Frac(sinf(seed.x) * 43758.5453123f), -1.0f + 2.0f * Frac(sinf(seed.y) * 43758.5453123f), -1.0f + 2.0f * Frac(sinf(seed.z) * 43758.5453123f));
}

float PlayerMoveParticleIdle::Noise(KuroEngine::Vec3<float> arg_st)
{
	KuroEngine::Vec3<float> intValue = { std::floor(arg_st.x) ,std::floor(arg_st.y) ,std::floor(arg_st.z) };
	KuroEngine::Vec3<float> floatValue = { Frac(arg_st.x) ,Frac(arg_st.y) ,Frac(arg_st.z) };

	//���̗אړ_�̍��W�����߂�B
	KuroEngine::Vec3<float> u;
	u.x = floatValue.x * floatValue.x * (3.0f - 2.0f * floatValue.x);
	u.y = floatValue.y * floatValue.y * (3.0f - 2.0f * floatValue.y);
	u.z = floatValue.z * floatValue.z * (3.0f - 2.0f * floatValue.z);

	//�e�אړ_�ł̃m�C�Y�����߂�B
	float center = Random3D(intValue).Dot(floatValue - KuroEngine::Vec3<float>(0, 0, 0));
	float right = Random3D(intValue + KuroEngine::Vec3<float>(1, 0, 0)).Dot(floatValue - KuroEngine::Vec3<float>(1, 0, 0));
	float top = Random3D(intValue + KuroEngine::Vec3<float>(0, 1, 0)).Dot(floatValue - KuroEngine::Vec3<float>(0, 1, 0));
	float rightTop = Random3D(intValue + KuroEngine::Vec3<float>(1, 1, 0)).Dot(floatValue - KuroEngine::Vec3<float>(1, 1, 0));
	float front = Random3D(intValue + KuroEngine::Vec3<float>(0, 0, 1)).Dot(floatValue - KuroEngine::Vec3<float>(0, 0, 1));
	float rightFront = Random3D(intValue + KuroEngine::Vec3<float>(1, 0, 1)).Dot(floatValue - KuroEngine::Vec3<float>(1, 0, 1));
	float topFront = Random3D(intValue + KuroEngine::Vec3<float>(0, 1, 1)).Dot(floatValue - KuroEngine::Vec3<float>(0, 1, 1));
	float rightTopFront = Random3D(intValue + KuroEngine::Vec3<float>(1, 1, 1)).Dot(floatValue - KuroEngine::Vec3<float>(1, 1, 1));

	//�m�C�Y�l���Ԃ���B
	float x1 = Lerp(center, right, u.x);
	float x2 = Lerp(top, rightTop, u.x);
	float y1 = Lerp(front, rightFront, u.x);
	float y2 = Lerp(topFront, rightTopFront, u.x);

	float xy1 = Lerp(x1, x2, u.y);
	float xy2 = Lerp(y1, y2, u.y);

	return Lerp(xy1, xy2, u.z);
}

float PlayerMoveParticleIdle::PerlinNoise(KuroEngine::Vec3<float> arg_st, int arg_octaves, float arg_persistence, float arg_lacunarity, KuroEngine::Vec3<float> arg_pos)
{

	float amplitude = 1.0;

	//�v���C���[�̃��[���h���W�Ɋ�Â��m�C�Y����
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

	noiseValue = (sum / maxValue + 1.0f) * 0.5f; //�m�C�Y�l��0.0����1.0�͈̔͂ɍă}�b�s���O


	return noiseValue;

}
