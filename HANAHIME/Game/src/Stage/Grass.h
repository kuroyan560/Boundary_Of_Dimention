#pragma once
#pragma once
#include"KuroEngine.h"
#include"../../../../src/engine/Common/Transform.h"
#include"ForUser/Timer.h"
#include<vector>
#include<memory>
#include<array>
#include"../Graphics/BasicDrawParameters.h"

namespace KuroEngine
{
	class VertexBuffer;
	class ConstantBuffer;
	class TextureBuffer;
	class RWStructuredBuffer;
	class StructuredBuffer;
	class Model;
	class Camera;
	class LightManager;
	class GraphicsPipeline;
	class ComputePipeline;
}

class WaterPaintBlend;

class Grass
{
	//���_���
	static const int s_vertexMax = 1024;
	//�C���X�^���V���O�`����
	static const int s_instanceMax = 1024;
	//�e�N�X�`���̐�
	static const int s_textureNumMax = 5;

	//���𐶂₷���`�F�b�N�Ŏg�p����l
	static const int GRASS_GROUP = 16;						//�X���b�h�O���[�v�Ɋ܂܂�鑐�̐�
	static const int GRASS_SPAN = 10;						//�s�N�Z���P�ʂő��𐶂₷�Ԋu
	static const int GRASS_SEARCH_X = 1280 / GRASS_SPAN;	//���𐶂₷�ꏊ��T���ۂɑ������鐔X
	static const int GRASS_SEARCH_Y = 720 / GRASS_SPAN;		//���𐶂₷�ꏊ��T���ۂɑ������鐔Y
	static const int GRASSF_SEARCH_COUNT = GRASS_SEARCH_X * GRASS_SEARCH_Y;		//���𐶂₷�ꏊ��T���ۂɑ������鐔���v
	static const int GRASSF_SEARCH_DISPATCH_X = GRASS_SEARCH_X / GRASS_GROUP;	//�������鐔�ɃX���b�h�O���[�v���l�������lX
	static const int GRASSF_SEARCH_DISPATCH_Y = GRASS_SEARCH_Y / GRASS_GROUP;	//�������鐔�ɃX���b�h�O���[�v���l�������lY

	//�p�C�v���C��
	std::shared_ptr<KuroEngine::GraphicsPipeline>m_pipeline;

	//�A�������̏��
	struct PlantGrass
	{
		KuroEngine::Vec3<float>m_pos = { 0,0,0 };
		int m_texIdx = 0;
		KuroEngine::Vec3<float>m_normal = { 0,1,0 };
		float m_sineLength;
		float m_appearY;		//�o���G�t�F�N�g�Ɏg�p����ϐ� Y�����ǂ��܂ŕ\�������邩�B
		float m_appearYTimer;
		int m_isAlive;
	};
	//�A�������̏��z��o�b�t�@
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_plantGrassBuffer;
	//�A�������̃J�E���^�o�b�t�@
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_plantGrassCounterBuffer;

	//���ނ�̃C�j�V�����C�U���
	struct GrassInitializer
	{
		KuroEngine::Vec3<float>m_pos;
		float m_sineLength;
		KuroEngine::Vec3<float>m_up;
		int m_texIdx;
		int m_isAlive;
		KuroEngine::Vec3<float>m_pad;
	};
	//��x�ɐ����ł���ő吔
	static const int GENERATE_MAX_ONCE = 2000;
	//�����\��̑��ނ�C�j�V�����C�U�̔z��
	std::vector<GrassInitializer>m_grassInitializerArray;
	//�����\��̑��ނ�C�j�V�����C�U�z��o�b�t�@
	std::shared_ptr<KuroEngine::StructuredBuffer>m_stackGrassInitializerBuffer;

	//�A�����鑐�̍ő吔
	int m_plantGrassMax = 10000;

	//��x�ɐA���鑐�̐�
	static const int PLANT_ONCE_COUNT = 512;

	//�R���s���[�g�p�C�v���C�����
	enum COMPUTE_PHASE { INIT, SEARCH_PLANT_POS, APPEAR, UPDATE, SORT, DISAPPEAR, NUM };
	//�R���s���[�g�p�C�v���C��
	std::array<std::shared_ptr<KuroEngine::ComputePipeline>, COMPUTE_PHASE::NUM>m_cPipeline;
	//�`��p�O���t�B�b�N�X�p�C�v���C��
	std::shared_ptr<KuroEngine::GraphicsPipeline>m_gPipeline;

	//���_�o�b�t�@
	std::shared_ptr<KuroEngine::VertexBuffer>m_vertBuffer;

	//��p�̒萔�o�b�t�@�ɑ���v���C���[�̃g�����X�t�H�[�����
	struct TransformCBVData
	{
		KuroEngine::Vec3<float>m_camPos;
		float m_seed;
		KuroEngine::Vec3<float> m_playerPos;
		int m_grassCount;
	};
	std::shared_ptr<KuroEngine::ConstantBuffer>m_otherTransformConstBuffer;

	//�s��ȊO�̃f�[�^�p�\���́i�D���Ȃ̓���Ăˁj
	struct CBVdata
	{
		KuroEngine::Matrix matView; // �r���[�s��
		KuroEngine::Matrix matProjection;	//�v���W�F�N�V�����s��
		KuroEngine::Vec3<float> eye = { 0,0,0 }; // �J�������W�i���[���h���W�j
		//������΂�����
		float m_checkClipOffset = 2.0f;
		//�v���C���[�̍��W
		KuroEngine::Vec3<float> m_playerPos;
		//���ӂɊ��ɑ��������Ă��邩�m�F����ۂ͈̔�
		float m_checkRange = 0.5f;
		//���ނ�o�ꎞ�̃C�[�W���O���x
		float m_appearEaseSpeed = 0.2f;
		//���ނ玀�S���̃C�[�W���O���x
		float m_deadEaseSpeed = 0.03f;
		//����h�炷�ۂ�Sine�� �܂蕗
		float m_sineWave = 0;
		//�����͂炷����
		float m_deathDistance = 8.0f;
	}m_constData;
	std::shared_ptr<KuroEngine::ConstantBuffer>m_constBuffer;

	//���茋�ʂ̊i�[�f�[�^
	struct CheckResult
	{
		//int m_aroundGrassCount = 0;
		KuroEngine::Vec3<float> m_plantPos;
		int m_isSuccess;
		KuroEngine::Vec3<float> m_plantNormal;
		int m_pad;
	};
	//���ӂɑ��ނ炪���邩�m�F�������ʂ��i�[����o�b�t�@
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_checkResultBuffer;

	//�\�[�g�ƍ폜�����Ŏg��unsigned int �̃o�b�t�@�[
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_sortAndDisappearNumBuffer;

	//�e�N�X�`��
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, s_textureNumMax>m_texBuffer;
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, s_textureNumMax>m_normalTexBuffer;

	//�P�t���[���O�̃v���C���[�̈ʒu
	KuroEngine::Vec3<float>m_oldPlayerPos;
	//����A����X�p��
	KuroEngine::Timer m_plantTimer;

	//�����蔻��̑傫��
	const float HIT_SCALE = 2.0f;

public:
	Grass();
	void Init();
	void Update(const float arg_timeScale, const KuroEngine::Transform arg_playerTransform, std::weak_ptr<KuroEngine::Camera> arg_cam, float arg_plantInfluenceRange);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, float arg_plantInfluenceRange, bool arg_isAttack);

	/// <summary>
	/// ����A����i�r���{�[�h�j
	/// </summary>
	/// <param name="arg_transform">���W</param>
	/// <param name="arg_grassPosScatter">�U�炵�</param>
	/// <param name="arg_waterPaintBlend">���ʉ敗�u�����h�|�X�g�G�t�F�N�g</param>
	void Plant(KuroEngine::Transform arg_transform, KuroEngine::Transform arg_playerTransform);

private:

	/// <summary>
	/// �����͂₷�ꏊ���擾����B
	/// </summary>
	/// <returns> t:�����Ă���  f:�����Ă��Ȃ� </returns>
	std::array<Grass::CheckResult, GRASSF_SEARCH_COUNT> SearchPlantPos(KuroEngine::Transform arg_playerTransform);

};