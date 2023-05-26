#pragma once
#pragma once
#include"KuroEngine.h"
#include"../../../../src/engine/Common/Transform.h"
#include"ForUser/Timer.h"
#include<vector>
#include<memory>
#include<array>
#include"../Graphics/BasicDrawParameters.h"
#include "Stage.h"

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
	//�C���X�^���V���O�`����
	static const int s_instanceMax = 1024;

	//�A�����鑐�̍ő吔
	static const int s_plantGrassMax = 10000;

	//���𐶂₷���`�F�b�N�Ŏg�p����l
	static const int GRASS_GROUP = 16;						//�X���b�h�O���[�v�Ɋ܂܂�鑐�̐�
	//static const int GRASS_SPAN = 10;						//�s�N�Z���P�ʂő��𐶂₷�Ԋu
	static const int GRASS_SPAN = 10;						//�s�N�Z���P�ʂő��𐶂₷�Ԋu
	static const int GRASS_SEARCH_X = 1280 / GRASS_SPAN;	//���𐶂₷�ꏊ��T���ۂɑ������鐔X
	static const int GRASS_SEARCH_Y = 720 / GRASS_SPAN;		//���𐶂₷�ꏊ��T���ۂɑ������鐔Y
	static const int GRASSF_SEARCH_COUNT = GRASS_SEARCH_X * GRASS_SEARCH_Y;		//���𐶂₷�ꏊ��T���ۂɑ������鐔���v
	static const int GRASSF_SEARCH_DISPATCH_X = GRASS_SEARCH_X / GRASS_GROUP;	//�������鐔�ɃX���b�h�O���[�v���l�������lX
	static const int GRASSF_SEARCH_DISPATCH_Y = GRASS_SEARCH_Y / GRASS_GROUP;	//�������鐔�ɃX���b�h�O���[�v���l�������lY

	//�p�C�v���C��
	std::shared_ptr<KuroEngine::GraphicsPipeline>m_pipeline;

	//�A�������̏��
	struct GrassData
	{
		KuroEngine::Vec3<float>m_pos = { 0,0,0 };
		KuroEngine::Vec3<float>m_normal = { 0,1,0 };
		int m_modelIdx = 0;
		float m_sineLength = 0;
		float m_sineTimer = 0;
		float m_appearY = 0;		//�o���G�t�F�N�g�Ɏg�p����ϐ� Y�����ǂ��܂ŕ\�������邩�B
		float m_appearYTimer = 0;
		bool m_isCheckGround  = false;
		int m_terrianIdx = -1;
		bool m_isDead = false;
		bool m_isCheckNear = false;	//�����ς݂�
	};
	std::vector<GrassData>m_plantGrassDataArray;
	std::vector<KuroEngine::Matrix>m_plantGrassWorldMatArray;

	//�R���s���[�g�p�C�v���C�����
	enum COMPUTE_PHASE { SEARCH_PLANT_POS, CHECK_IS_BRIGHT, NUM };
	//�R���s���[�g�p�C�v���C��
	std::array<std::shared_ptr<KuroEngine::ComputePipeline>, COMPUTE_PHASE::NUM>m_cPipeline;

	//�`��p�O���t�B�b�N�X�p�C�v���C��
	std::shared_ptr<KuroEngine::GraphicsPipeline>m_gPipeline;

	//�v���C���[�̏��𑗂�萔�o�b�t�@
	struct PlayerInfo
	{
		KuroEngine::Vec3<float>m_pos;
		float m_plantLighrRante;
	};
	std::shared_ptr<KuroEngine::ConstantBuffer>m_playerInfoBuffer;

	//�����ʒu���v�Z����̂Ɏg�p����萔�o�b�t�@
	struct SearchPlantPosConstData
	{
		int m_grassCount;
		float m_seed;
	};
	std::shared_ptr<KuroEngine::ConstantBuffer>m_searchPlantPosConstBuffer;

	//���茋�ʂ̊i�[�f�[�^
	struct SearchPlantResult
	{
		//int m_aroundGrassCount = 0;
		KuroEngine::Vec3<float> m_plantPos;
		int m_isSuccess;
		KuroEngine::Vec3<float> m_plantNormal;
		int m_pad;
	};

	//���f��
	static const int s_modelNumMax = 3;
	std::array<std::shared_ptr<KuroEngine::Model>, s_modelNumMax>m_modelArray;

	//�����Ă��邩�̔��茋�ʂ̊m��o�b�t�@
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_checkBrightResultBuffer;
	//�����ʒu�T���̌��ʂ��i�[����o�b�t�@
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_searchPlantResultBuffer;

	//���ނ�̍��W�z��o�b�t�@
	std::shared_ptr<KuroEngine::StructuredBuffer>m_plantGrassPosArrayBuffer;
	std::vector<KuroEngine::Vec3<float>>m_plantGrassPosArray;

	
	//�`��Ɏg�����f�����Ƃ̑��̃��[���h�s��
	struct GrassInfo {
		KuroEngine::Matrix m_worldMat;
		float m_grassTimer;
		KuroEngine::Vec3<float> m_pad;
	};
	std::array<std::vector<GrassInfo>, s_modelNumMax>m_grassWorldMatricies;
	//�`��Ɏg�����f�����Ƃ̑��̃��[���h�s��o�b�t�@
	std::array<std::shared_ptr<KuroEngine::StructuredBuffer>, s_modelNumMax>m_grassWorldMatriciesBuffer;

	//�P�t���[���O�̃v���C���[�̈ʒu
	KuroEngine::Vec3<float>m_oldPlayerPos;
	KuroEngine::Vec3<float>m_playerPos;
	//����A����X�p��
	KuroEngine::Timer m_plantTimer;

	//�����蔻��̑傫��
	const float HIT_SCALE = 2.0f;

public:
	Grass();
	void Init();
	void Update(const float arg_timeScale, bool arg_isPlayerOverheat, const KuroEngine::Transform arg_playerTransform, std::weak_ptr<KuroEngine::Camera> arg_cam, float arg_plantInfluenceRange, const std::weak_ptr<Stage>arg_nowStage, bool arg_isAttack, KuroEngine::Vec3<float> arg_moveSpeed);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, float arg_plantInfluenceRange, bool arg_isAttack);

private:

	/// <summary>
	/// �����͂₷�ꏊ���擾����B
	/// </summary>
	/// <returns> t:�����Ă���  f:�����Ă��Ȃ� </returns>
	std::array<Grass::SearchPlantResult, GRASSF_SEARCH_COUNT> SearchPlantPos(KuroEngine::Transform arg_playerTransform);

	void UpdateGrassEasing(Grass::GrassData& arg_grass, int arg_index, KuroEngine::Vec3<float> arg_moveSpeed);
	void GrassCheckHit(Grass::GrassData& arg_grass, const std::weak_ptr<Stage>arg_nowStage);

};