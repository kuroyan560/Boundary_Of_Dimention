#pragma once
#include"KuroEngine.h"
#include"ForUser/Timer.h"
#include<vector>
#include<memory>
namespace KuroEngine
{
	class ConstantBuffer;
	class VertexBuffer;
	class ComputePipeline;
	class Model;
	class Camera;
	class LightManager;
}

class Grass
{
	static const int THREAD_PER_NUM = 10;

	//UAV�f�[�^�p�\����
	struct UAVdata
	{
		KuroEngine::Vec3<float>m_pos;
	};
	std::vector<UAVdata>m_uavDataArray;
	std::shared_ptr<KuroEngine::VertexBuffer>m_uavDataBuffer;

	//CBV�f�[�^�p�\����
	struct CBVdata
	{
		float m_timeScale = 1.0f;
	}m_constData;
	std::shared_ptr<KuroEngine::ConstantBuffer>m_constBuffer;

	//�������p�R���s���[�g�p�C�v���C��
	std::shared_ptr<KuroEngine::ComputePipeline>m_initComputePipeline;
	//�X�V�p�R���s���[�g�p�C�v���C��
	std::shared_ptr<KuroEngine::ComputePipeline>m_updateComputePipeline;

	//���u�����u���b�N
	std::shared_ptr<KuroEngine::Model>m_grassBlockModel;
	//���Œu�������u���b�N�̃��[���h�s��z��
	std::vector<KuroEngine::Matrix>m_grassWorldMatArray;
	//�P�t���[���O�̃v���C���[�̈ʒu
	KuroEngine::Vec3<float>m_oldPlayerPos;
	//���u���b�N��A����X�p��
	KuroEngine::Timer m_plantTimer;

public:
	Grass();
	void Init();
	void Update(const float arg_timeScale, const KuroEngine::Vec3<float> arg_playerPos, const KuroEngine::Quaternion arg_playerRotate);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	/// <summary>
	/// ����A����
	/// </summary>
	/// <param name="arg_worldMat">�A���鑐�̃��[���h�s��</param>
	void Plant(KuroEngine::Matrix arg_worldMat);
};