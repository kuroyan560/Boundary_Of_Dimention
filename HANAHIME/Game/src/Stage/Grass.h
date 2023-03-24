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
	//頂点上限
	static const int s_vertexMax = 1024;
	//インスタンシング描画上限
	static const int s_instanceMax = 1024;
	//テクスチャの数
	static const int s_textureNumMax = 5;

	//パイプライン
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

	//行列以外のデータ用構造体（好きなの入れてね）
	struct CBVdata
	{
	}m_constData;
	std::shared_ptr<KuroEngine::ConstantBuffer>m_constBuffer;

	//テクスチャ
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, s_textureNumMax>m_texBuffer;

//仮のやつ======================
	//仮置き草ブロック
	std::shared_ptr<KuroEngine::Model>m_grassBlockModel;
	//仮で置いた草ブロックのワールド行列配列
	std::vector<KuroEngine::Matrix>m_grassWorldMatArray;
	//１フレーム前のプレイヤーの位置
	KuroEngine::Vec3<float>m_oldPlayerPos;
	//草ブロックを植えるスパン
	KuroEngine::Timer m_plantTimer;
//=============================

public:
	Grass();
	void Init();
	void Update(const float arg_timeScale, const KuroEngine::Vec3<float> arg_playerPos, const KuroEngine::Quaternion arg_playerRotate);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	/// <summary>
	/// 草を植える（仮置き草ブロック）
	/// </summary>
	/// <param name="arg_worldMat">植える草のワールド行列</param>
	void PlantGrassBlock(KuroEngine::Transform arg_transform);

	/// <summary>
	/// 草を植える（ビルボード）
	/// </summary>
	/// <param name="arg_pos">座標</param>
	/// <param name="arg_normal">法線</param>
	void Plant(KuroEngine::Vec3<float>arg_pos, KuroEngine::Vec3<float>arg_normal);
};