#pragma once
#include"Common/Vec.h"
#include<vector>
#include<memory>
namespace KuroEngine
{
	class ConstantBuffer;
	class VertexBuffer;
	class ComputePipeline;
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

public:
	Grass();
	void Init();
	void Update(float arg_timeScale);
	void Draw();
};