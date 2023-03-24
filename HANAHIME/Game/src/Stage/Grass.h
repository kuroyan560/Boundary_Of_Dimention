#pragma once
#include"KuroEngine.h"
#include"../../../../src/engine/Common/Transform.h"
#include"ForUser/Timer.h"
#include<vector>
#include<memory>
#include<array>
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
	};
	std::array<Vertex, s_vertexMax>m_vertices;
	std::shared_ptr<KuroEngine::VertexBuffer>m_vertBuffer;
	int m_deadVertexIdx = 0;

	//�s��ȊO�̃f�[�^�p�\���́i�D���Ȃ̓���Ăˁj
	struct CBVdata
	{
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

public:
	Grass();
	void Init();
	void Update(const float arg_timeScale, const KuroEngine::Vec3<float> arg_playerPos, const KuroEngine::Quaternion arg_playerRotate);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	/// <summary>
	/// ����A����i���u�����u���b�N�j
	/// </summary>
	/// <param name="arg_worldMat">�A���鑐�̃��[���h�s��</param>
	void PlantGrassBlock(KuroEngine::Transform arg_transform);

	/// <summary>
	/// ����A����i�r���{�[�h�j
	/// </summary>
	/// <param name="arg_pos">���W</param>
	/// <param name="arg_normal">�@��</param>
	void Plant(KuroEngine::Vec3<float>arg_pos, KuroEngine::Vec3<float>arg_normal);
};