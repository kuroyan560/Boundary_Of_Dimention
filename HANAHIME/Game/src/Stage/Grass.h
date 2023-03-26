#pragma once
#include"KuroEngine.h"
#include"ForUser/Timer.h"
#include<vector>
#include<memory>
#include"../Graphics/BasicDrawParameters.h"
namespace KuroEngine
{
	class ConstantBuffer;
	class VertexBuffer;
	class ComputePipeline;
	class Model;
	class Camera;
	class LightManager;
}

class WaterPaintBlend;

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

	//仮置き草ブロック
	std::shared_ptr<KuroEngine::Model>m_grassBlockModel;
	//仮で置いた草ブロックのワールド行列配列
	std::vector<KuroEngine::Matrix>m_grassWorldMatArray;
	//１フレーム前のプレイヤーの位置
	KuroEngine::Vec3<float>m_oldPlayerPos;
	//草ブロックを植えるスパン
	KuroEngine::Timer m_plantTimer;

	IndividualDrawParameter m_drawParam;

public:
	Grass();
	void Init();
	void Update(const float arg_timeScale, const KuroEngine::Vec3<float> arg_playerPos, const KuroEngine::Quaternion arg_playerRotate, WaterPaintBlend& arg_waterPaintBlend);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	/// <summary>
	/// 草を植える
	/// </summary>
	/// <param name="arg_worldMat">植える草のワールド行列</param>
	/// <param name="arg_pos">植える草の座標</param>
	/// <param name="arg_waterPaintBlend">水彩画ブレンドポストエフェクトの参照</param>
	void Plant(KuroEngine::Matrix arg_worldMat, KuroEngine::Vec3<float>arg_pos, WaterPaintBlend& arg_waterPaintBlend);
};