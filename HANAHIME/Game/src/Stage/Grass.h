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
		int pad[2];
	};
	//�A�������̏��z��o�b�t�@
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_plantGrassBuffer;
	//�A�������̃J�E���^�o�b�t�@
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_plantGrassCounterBuffer;

	//���ނ�̃C�j�V�����C�U���
	struct GrassInitializer
	{
		KuroEngine::Vec3<float>m_pos;
		KuroEngine::Vec3<float>m_up;
		float m_sineLength;
		int m_texIdx;
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
	int m_plantOnceCountMin = 3;
	int m_plantOnceCountMax = 6;

	//�R���s���[�g�p�C�v���C�����
	enum COMPUTE_PHASE { INIT, CHECK_AROUND, GENERATE, UPDATE, NUM };
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
		float pad;
		KuroEngine::Vec3<float>m_checkPlantPos;
	};
	std::shared_ptr<KuroEngine::ConstantBuffer>m_otherTransformConstBuffer;

	//�s��ȊO�̃f�[�^�p�\���́i�D���Ȃ̓���Ăˁj
	struct CBVdata
	{
		//������΂�����
		float m_checkClipOffset = 2.0f;
		//���ӂɊ��ɑ��������Ă��邩�m�F����ۂ͈̔�
		float m_checkRange = 1.0f;
		//���ނ�o�ꎞ�̃C�[�W���O���x
		float m_appearEaseSpeed = 0.05f;
		//����h�炷�ۂ�Sine�� �܂蕗
		float m_sineWave = 0;
		//�v���C���[�̍��W
		KuroEngine::Vec3<float> m_playerPos;
	}m_constData;
	std::shared_ptr<KuroEngine::ConstantBuffer>m_constBuffer;

	//���茋�ʂ̊i�[�f�[�^
	struct CheckResult
	{
		int m_aroundGrassCount = 0;
	};
	//���ӂɑ��ނ炪���邩�m�F�������ʂ��i�[����o�b�t�@
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_checkResultBuffer;

	//�e�N�X�`��
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, s_textureNumMax>m_texBuffer;
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, s_textureNumMax>m_normalTexBuffer;

	//�P�t���[���O�̃v���C���[�̈ʒu
	KuroEngine::Vec3<float>m_oldPlayerPos;
	//����A����X�p��
	KuroEngine::Timer m_plantTimer;

public:
	Grass();
	void Init();
	void Update(const float arg_timeScale, const KuroEngine::Transform arg_playerTransform, bool arg_playerOnGround, KuroEngine::Transform arg_camTransform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	/// <summary>
	/// ����A����i�r���{�[�h�j
	/// </summary>
	/// <param name="arg_transform">���W</param>
	/// <param name="arg_grassPosScatter">�U�炵�</param>
	/// <param name="arg_waterPaintBlend">���ʉ敗�u�����h�|�X�g�G�t�F�N�g</param>
	void Plant(KuroEngine::Transform arg_transform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend);

private:

	/// <summary>
	/// ���͂ɑ��������Ă��邩���`�F�b�N����B
	/// </summary>
	/// <param name="arg_checkPos"> �`�F�b�N������W </param>
	/// <returns> t:�����Ă���  f:�����Ă��Ȃ� </returns>
	bool IsGrassAround(const KuroEngine::Vec3<float> arg_checkPos);

};