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
		float m_sineLength;
		float m_appearY;		//出現エフェクトに使用する変数 Y軸をどこまで表示させるか。
	};
	std::array<Vertex, s_vertexMax>m_vertices;
	std::shared_ptr<KuroEngine::VertexBuffer>m_vertBuffer;
	int m_deadVertexIdx = 0;

	//草エフェクト用
	float m_vertexSineWave;			//草の頂点を揺らすSin波の加算量。
	std::array<float, s_vertexMax> m_appearYTimer;

	//行列以外のデータ用構造体（好きなの入れてね）
	struct CBVdata
	{
		KuroEngine::Vec3<float>m_pos = { 0,0,0 };
		float m_sineWave = 0;
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

	IndividualDrawParameter m_drawParam;

public:
	Grass();
	void Init();
	void Update(const float arg_timeScale, const KuroEngine::Vec3<float> arg_playerPos, const KuroEngine::Quaternion arg_playerRotate, KuroEngine::Transform arg_camTransform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	/// <summary>
	/// 草を植える（仮置き草ブロック）
	/// </summary>
	/// <param name="arg_worldMat">植える草のワールド行列</param>
	/// <param name="arg_grassPosScatter">散らし具合</param>
	/// <param name="arg_waterPaintBlend">水彩画風ブレンドポストエフェクト</param>
	void PlantGrassBlock(KuroEngine::Transform arg_transform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend);

	/// <summary>
	/// 草を植える（ビルボード）
	/// </summary>
	/// <param name="arg_transform">座標</param>
	/// <param name="arg_grassPosScatter">散らし具合</param>
	/// <param name="arg_waterPaintBlend">水彩画風ブレンドポストエフェクト</param>
	void Plant(KuroEngine::Transform arg_transform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend);
};