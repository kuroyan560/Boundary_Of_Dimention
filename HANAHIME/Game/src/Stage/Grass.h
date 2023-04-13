#pragma once
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
		int m_isAlive;
		int m_pad;
	};
	//植えた草の情報配列バッファ
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_plantGrassBuffer;
	//植えた草のカウンタバッファ
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_plantGrassCounterBuffer;

	//草むらのイニシャライザ情報
	struct GrassInitializer
	{
		KuroEngine::Vec3<float>m_pos;
		float m_sineLength;
		KuroEngine::Vec3<float>m_up;
		int m_texIdx;
		KuroEngine::Vec3<float>m_pad;
		int m_isAlive;
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
	static const int PLANT_ONCE_COUNT = 256;

	//コンピュートパイプライン種別
	enum COMPUTE_PHASE { INIT, SEARCH_PLANT_POS, APPEAR, UPDATE, SORT, DISAPPEAR, NUM };
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
		float m_seed;
		KuroEngine::Vec2<float> m_pad;
		int m_grassCount;
		int m_plantOnceCount;
	};
	std::shared_ptr<KuroEngine::ConstantBuffer>m_otherTransformConstBuffer;

	//行列以外のデータ用構造体（好きなの入れてね）
	struct CBVdata
	{
		//プレイヤーの座標
		KuroEngine::Vec3<float> m_playerPos;
		//判定を飛ばす距離
		float m_checkClipOffset = 2.0f;
		//周辺に既に草が生えているか確認する際の範囲
		float m_checkRange = 0.5f;
		//草むら登場時のイージング速度
		float m_appearEaseSpeed = 0.2f;
		//草むら死亡時のイージング速度
		float m_deadEaseSpeed = 0.03f;
		//草を揺らす際のSine量 つまり風
		float m_sineWave = 0;
		//草を枯らす距離
		float m_deathDistance = 8.0f;
		KuroEngine::Vec3<float> m_pad;
	}m_constData;
	std::shared_ptr<KuroEngine::ConstantBuffer>m_constBuffer;

	//判定結果の格納データ
	struct CheckResult
	{
		//int m_aroundGrassCount = 0;
		KuroEngine::Vec3<float> m_plantPos;
		int m_isSuccess;
		KuroEngine::Vec3<float> m_plantNormal;
		int m_pad;
	};
	//周辺に草むらがあるか確認した結果を格納するバッファ
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_checkResultBuffer;

	//ソートと削除処理で使うunsigned int のバッファー
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_sortAndDisappearNumBuffer;

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
	void Update(const float arg_timeScale, const KuroEngine::Transform arg_playerTransform, std::weak_ptr<KuroEngine::Camera> arg_cam, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	/// <summary>
	/// 草を植える（ビルボード）
	/// </summary>
	/// <param name="arg_transform">座標</param>
	/// <param name="arg_grassPosScatter">散らし具合</param>
	/// <param name="arg_waterPaintBlend">水彩画風ブレンドポストエフェクト</param>
	void Plant(KuroEngine::Transform arg_transform, KuroEngine::Transform arg_playerTransform, KuroEngine::Vec2<float> arg_grassPosScatter, WaterPaintBlend& arg_waterPaintBlend);

private:

	/// <summary>
	/// 草をはやす場所を取得する。
	/// </summary>
	/// <returns> t:生えている  f:生えていない </returns>
	std::array<CheckResult, PLANT_ONCE_COUNT> SearchPlantPos(KuroEngine::Transform arg_playerTransform);

};