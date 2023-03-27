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
	class GraphicsPipeline;
	class VertexBuffer;
	class ConstantBuffer;
	class TextureBuffer;
	class Model;
	class Camera;
	class LightManager;
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

	struct Vertex
	{
		KuroEngine::Vec3<float>m_pos = { 0,0,0 };
		int m_texIdx = 0;
		KuroEngine::Vec3<float>m_normal = { 0,1,0 };
		int m_isAlive = 0;
		float m_sineLength;
		float m_appearY;		//�o���G�t�F�N�g�Ɏg�p����ϐ� Y�����ǂ��܂ŕ\�������邩�B
	};
	std::array<Vertex, s_vertexMax>m_vertices;
	std::shared_ptr<KuroEngine::VertexBuffer>m_vertBuffer;
	int m_deadVertexIdx = 0;

	//���G�t�F�N�g�p
	float m_vertexSineWave;			//���̒��_��h�炷Sin�g�̉��Z�ʁB
	std::array<float, s_vertexMax> m_appearYTimer;

	//�s��ȊO�̃f�[�^�p�\���́i�D���Ȃ̓���Ăˁj
	struct CBVdata
	{
		KuroEngine::Vec3<float>m_pos = { 0,0,0 };
		float m_sineWave = 0;
	}m_constData;
	std::shared_ptr<KuroEngine::ConstantBuffer>m_constBuffer;

	//�e�N�X�`��
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, s_textureNumMax>m_texBuffer;

//���̂��======================
	//���u�����u���b�N
	std::shared_ptr<KuroEngine::Model>m_grassBlockModel;
	//���Œu�������u���b�N�̃��[���h�s��z��
	std::vector<KuroEngine::Matrix>m_grassWorldMatArray;
	//�P�t���[���O�̃v���C���[�̈ʒu
	KuroEngine::Vec3<float>m_oldPlayerPos;
	//���u���b�N��A����X�p��
	KuroEngine::Timer m_plantTimer;
//=============================

	IndividualDrawParameter m_drawParam;

public:
	Grass();
	void Init();
	void Update(const float arg_timeScale, const KuroEngine::Vec3<float> arg_playerPos, const KuroEngine::Quaternion arg_playerRotate, KuroEngine::Transform arg_camTransform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	/// <summary>
	/// ����A����i���u�����u���b�N�j
	/// </summary>
	/// <param name="arg_worldMat">�A���鑐�̃��[���h�s��</param>
	/// <param name="arg_grassPosScatter">�U�炵�</param>
	/// <param name="arg_waterPaintBlend">���ʉ敗�u�����h�|�X�g�G�t�F�N�g</param>
	void PlantGrassBlock(KuroEngine::Transform arg_transform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend);

	/// <summary>
	/// ����A����i�r���{�[�h�j
	/// </summary>
	/// <param name="arg_transform">���W</param>
	/// <param name="arg_grassPosScatter">�U�炵�</param>
	/// <param name="arg_waterPaintBlend">���ʉ敗�u�����h�|�X�g�G�t�F�N�g</param>
	void Plant(KuroEngine::Transform arg_transform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend);
};