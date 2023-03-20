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

	//UAVデータ用構造体
	struct UAVdata
	{
		KuroEngine::Vec3<float>m_pos;
	};
	std::vector<UAVdata>m_uavDataArray;
	std::shared_ptr<KuroEngine::VertexBuffer>m_uavDataBuffer;

	//CBVデータ用構造体
	struct CBVdata
	{
		float m_timeScale = 1.0f;
	}m_constData;
	std::shared_ptr<KuroEngine::ConstantBuffer>m_constBuffer;

	//初期化用コンピュートパイプライン
	std::shared_ptr<KuroEngine::ComputePipeline>m_initComputePipeline;
	//更新用コンピュートパイプライン
	std::shared_ptr<KuroEngine::ComputePipeline>m_updateComputePipeline;

public:
	Grass();
	void Init();
	void Update(float arg_timeScale);
	void Draw();
};