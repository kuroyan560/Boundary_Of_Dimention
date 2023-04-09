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
	//頂点上限
	static const int s_vertexMax = 1024;
	//インスタンシング描画上限
	static const int s_instanceMax = 1024;
	//テクスチャの数
	static const int s_textureNumMax = 5;

	//パイプライン
	std::shared_ptr<KuroEngine::GraphicsPipeline>m_pipeline;

	//植えた草の情報
	struct PlantGrass
	{
		KuroEngine::Vec3<float>m_pos = { 0,0,0 };
		int m_texIdx = 0;
		KuroEngine::Vec3<float>m_normal = { 0,1,0 };
		float m_sineLength;
		float m_appearY;		//出現エフェクトに使用する変数 Y軸をどこまで表示させるか。
		float m_appearYTimer;
		int pad[2];
	};
	//植えた草の情報配列バッファ
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_plantGrassBuffer;
	//植えた草のカウンタバッファ
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_plantGrassCounterBuffer;

	//草むらのイニシャライザ情報
	struct GrassInitializer
	{
		KuroEngine::Vec3<float>m_pos;
		KuroEngine::Vec3<float>m_up;
		float m_sineLength;
		int m_texIdx;
	};
	//一度に生成できる最大数
	static const int GENERATE_MAX_ONCE = 2000;
	//生成予定の草むらイニシャライザの配列
	std::vector<GrassInitializer>m_grassInitializerArray;
	//生成予定の草むらイニシャライザ配列バッファ
	std::shared_ptr<KuroEngine::StructuredBuffer>m_stackGrassInitializerBuffer;

	//植えられる草の最大数
	int m_plantGrassMax = 10000;

	//一度に植える草の数
	int m_plantOnceCountMin = 3;
	int m_plantOnceCountMax = 6;

	//コンピュートパイプライン種別
	enum COMPUTE_PHASE { INIT, CHECK_AROUND, GENERATE, UPDATE, NUM };
	//コンピュートパイプライン
	std::array<std::shared_ptr<KuroEngine::ComputePipeline>, COMPUTE_PHASE::NUM>m_cPipeline;
	//描画用グラフィックスパイプライン
	std::shared_ptr<KuroEngine::GraphicsPipeline>m_gPipeline;

	//頂点バッファ
	std::shared_ptr<KuroEngine::VertexBuffer>m_vertBuffer;

	//専用の定数バッファに送るプレイヤーのトランスフォーム情報
	struct TransformCBVData
	{
		KuroEngine::Vec3<float>m_camPos;
		float pad;
		KuroEngine::Vec3<float>m_checkPlantPos;
	};
	std::shared_ptr<KuroEngine::ConstantBuffer>m_otherTransformConstBuffer;

	//行列以外のデータ用構造体（好きなの入れてね）
	struct CBVdata
	{
		//判定を飛ばす距離
		float m_checkClipOffset = 2.0f;
		//周辺に既に草が生えているか確認する際の範囲
		float m_checkRange = 1.0f;
		//草むら登場時のイージング速度
		float m_appearEaseSpeed = 0.05f;
		//草を揺らす際のSine量 つまり風
		float m_sineWave = 0;
		//プレイヤーの座標
		KuroEngine::Vec3<float> m_playerPos;
	}m_constData;
	std::shared_ptr<KuroEngine::ConstantBuffer>m_constBuffer;

	//判定結果の格納データ
	struct CheckResult
	{
		int m_aroundGrassCount = 0;
	};
	//周辺に草むらがあるか確認した結果を格納するバッファ
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_checkResultBuffer;

	//テクスチャ
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, s_textureNumMax>m_texBuffer;
	std::array<std::shared_ptr<KuroEngine::TextureBuffer>, s_textureNumMax>m_normalTexBuffer;

	//１フレーム前のプレイヤーの位置
	KuroEngine::Vec3<float>m_oldPlayerPos;
	//草を植えるスパン
	KuroEngine::Timer m_plantTimer;

public:
	Grass();
	void Init();
	void Update(const float arg_timeScale, const KuroEngine::Transform arg_playerTransform, bool arg_playerOnGround, KuroEngine::Transform arg_camTransform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	/// <summary>
	/// 草を植える（ビルボード）
	/// </summary>
	/// <param name="arg_transform">座標</param>
	/// <param name="arg_grassPosScatter">散らし具合</param>
	/// <param name="arg_waterPaintBlend">水彩画風ブレンドポストエフェクト</param>
	void Plant(KuroEngine::Transform arg_transform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend);

private:

	/// <summary>
	/// 周囲に草が生えているかをチェックする。
	/// </summary>
	/// <param name="arg_checkPos"> チェックする座標 </param>
	/// <returns> t:生えている  f:生えていない </returns>
	bool IsGrassAround(const KuroEngine::Vec3<float> arg_checkPos);

};