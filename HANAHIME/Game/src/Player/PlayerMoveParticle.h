#pragma once
#pragma once
#include "../../../../src/engine/Common/Transform.h"
#include "../../../../src/engine/ForUser/Timer.h"
#include "../../../../src/engine/Render/RenderObject/LightManager.h"
#include <array>
#include <ForUser/Object/Model.h>

//�v���C���[���ړ�����Ƃ��̃p�[�e�B�N��
class PlayerMoveParticle {

private:

	static const int EXIT_TIMER = 120;
	struct ParticleData
	{

		KuroEngine::Vec3<float> m_st;
		KuroEngine::Transform m_transform;
		KuroEngine::Timer m_exitTimer;	 //������܂ł̃^�C�}�[
		bool m_isAlive;

		ParticleData() : m_isAlive(false), m_exitTimer(120) {};

	};

	static const int PARTICLE_COUNT = 256;
	std::array<ParticleData, 256> m_particle;	//�p�[�e�B�N���{��


	std::shared_ptr<KuroEngine::Model> m_model;


public:

	PlayerMoveParticle();

	//����������
	void Init();

	//�X�V����
	void Update();

	//�`�揈��
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//��������
	void Generate(const KuroEngine::Vec3<float>& arg_playerPos, const KuroEngine::Vec3<float>& arg_scatter);


private:

	/// <summary>
	/// �J�[���m�C�Y�����߂�B
	/// </summary>
	/// <param name="arg_pos"></param>
	/// <returns> �ړ��� </returns>
	KuroEngine::Vec3<float> CurlNoise3D(const KuroEngine::Vec3<float>& arg_st, const KuroEngine::Vec3<float>& arg_pos);

	//hlsl��frac�֐� modf��p���ď����_���擾����悤�ɂ��Ă���B
	float Frac(float arg_x);

	//3D�̃����_���n�b�V���֐� hlsl�Ŏg�p���Ă���z�����̂܂܎����Ă������߁A����������frac
	KuroEngine::Vec3<float> Random3D(KuroEngine::Vec3<float> arg_st);

	//���[�v�֐� ������t��1�𒴂��Ă��܂����߁A�V����������ǉ����܂����B
	inline float Lerp(float arg_a, float arg_b, float arg_t) {
		return arg_a * (1 - arg_t) + arg_b * arg_t;
	}

	//�i�q�̃m�C�Y�𒲂ׁA��Ԃ���B
	float Noise(KuroEngine::Vec3<float> arg_st);

	//3D�p�[�����m�C�Y�֐�
	float PerlinNoise(KuroEngine::Vec3<float> arg_st, int arg_octaves, float arg_persistence, float arg_lacunarity, KuroEngine::Vec3<float> arg_pos);

};