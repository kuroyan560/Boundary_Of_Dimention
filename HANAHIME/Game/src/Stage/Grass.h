#pragma once
#pragma once
#include"KuroEngine.h"
#include"../../../../src/engine/Common/Transform.h"
#include"ForUser/Timer.h"
#include<vector>
#include<memory>
#include<array>
#include"../Graphics/BasicDrawParameters.h"
#include "Stage.h"

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
	//インスタンシング描画上限
	static const int s_instanceMax = 1024;

	//植えられる草の最大数
	static const int s_plantGrassMax = 10000;

	//草を生やすかチェックで使用する値
	static const int GRASS_GROUP = 16;						//スレッドグループに含まれる草の数
	//static const int GRASS_SPAN = 10;						//ピクセル単位で草を生やす間隔
	static const int GRASS_SPAN = 10;						//ピクセル単位で草を生やす間隔
	static const int GRASS_SEARCH_X = 1280 / GRASS_SPAN;	//草を生やす場所を探す際に走査する数X
	static const int GRASS_SEARCH_Y = 720 / GRASS_SPAN;		//草を生やす場所を探す際に走査する数Y
	static const int GRASSF_SEARCH_COUNT = GRASS_SEARCH_X * GRASS_SEARCH_Y;		//草を生やす場所を探す際に走査する数合計
	static const int GRASSF_SEARCH_DISPATCH_X = GRASS_SEARCH_X / GRASS_GROUP;	//走査する数にスレッドグループを考慮した値X
	static const int GRASSF_SEARCH_DISPATCH_Y = GRASS_SEARCH_Y / GRASS_GROUP;	//走査する数にスレッドグループを考慮した値Y

	//パイプライン
	std::shared_ptr<KuroEngine::GraphicsPipeline>m_pipeline;

	//植えた草の情報
	struct GrassData
	{
		KuroEngine::Vec3<float>m_pos = { 0,0,0 };
		KuroEngine::Vec3<float>m_normal = { 0,1,0 };
		int m_modelIdx = 0;
		float m_sineLength = 0;
		float m_sineTimer = 0;
		float m_appearY = 0;		//出現エフェクトに使用する変数 Y軸をどこまで表示させるか。
		float m_appearYTimer = 0;
		bool m_isCheckGround  = false;
		int m_terrianIdx = -1;
		bool m_isDead = false;
		bool m_isCheckNear = false;	//完了済みか
	};
	std::vector<GrassData>m_plantGrassDataArray;
	std::vector<KuroEngine::Matrix>m_plantGrassWorldMatArray;

	//コンピュートパイプライン種別
	enum COMPUTE_PHASE { SEARCH_PLANT_POS, CHECK_IS_BRIGHT, NUM };
	//コンピュートパイプライン
	std::array<std::shared_ptr<KuroEngine::ComputePipeline>, COMPUTE_PHASE::NUM>m_cPipeline;

	//描画用グラフィックスパイプライン
	std::shared_ptr<KuroEngine::GraphicsPipeline>m_gPipeline;

	//プレイヤーの情報を送る定数バッファ
	struct PlayerInfo
	{
		KuroEngine::Vec3<float>m_pos;
		float m_plantLighrRante;
	};
	std::shared_ptr<KuroEngine::ConstantBuffer>m_playerInfoBuffer;

	//生成位置を計算するのに使用する定数バッファ
	struct SearchPlantPosConstData
	{
		int m_grassCount;
		float m_seed;
	};
	std::shared_ptr<KuroEngine::ConstantBuffer>m_searchPlantPosConstBuffer;

	//判定結果の格納データ
	struct SearchPlantResult
	{
		//int m_aroundGrassCount = 0;
		KuroEngine::Vec3<float> m_plantPos;
		int m_isSuccess;
		KuroEngine::Vec3<float> m_plantNormal;
		int m_pad;
	};

	//モデル
	static const int s_modelNumMax = 3;
	std::array<std::shared_ptr<KuroEngine::Model>, s_modelNumMax>m_modelArray;

	//光っているかの判定結果の確報バッファ
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_checkBrightResultBuffer;
	//生成位置探索の結果を格納するバッファ
	std::shared_ptr<KuroEngine::RWStructuredBuffer>m_searchPlantResultBuffer;

	//草むらの座標配列バッファ
	std::shared_ptr<KuroEngine::StructuredBuffer>m_plantGrassPosArrayBuffer;
	std::vector<KuroEngine::Vec3<float>>m_plantGrassPosArray;

	
	//描画に使うモデルごとの草のワールド行列
	struct GrassInfo {
		KuroEngine::Matrix m_worldMat;
		float m_grassTimer;
		KuroEngine::Vec3<float> m_pad;
	};
	std::array<std::vector<GrassInfo>, s_modelNumMax>m_grassWorldMatricies;
	//描画に使うモデルごとの草のワールド行列バッファ
	std::array<std::shared_ptr<KuroEngine::StructuredBuffer>, s_modelNumMax>m_grassWorldMatriciesBuffer;

	//１フレーム前のプレイヤーの位置
	KuroEngine::Vec3<float>m_oldPlayerPos;
	KuroEngine::Vec3<float>m_playerPos;
	//草を植えるスパン
	KuroEngine::Timer m_plantTimer;

	//当たり判定の大きさ
	const float HIT_SCALE = 2.0f;

public:
	Grass();
	void Init();
	void Update(const float arg_timeScale, bool arg_isPlayerOverheat, const KuroEngine::Transform arg_playerTransform, std::weak_ptr<KuroEngine::Camera> arg_cam, float arg_plantInfluenceRange, const std::weak_ptr<Stage>arg_nowStage, bool arg_isAttack, KuroEngine::Vec3<float> arg_moveSpeed);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, float arg_plantInfluenceRange, bool arg_isAttack);

private:

	/// <summary>
	/// 草をはやす場所を取得する。
	/// </summary>
	/// <returns> t:生えている  f:生えていない </returns>
	std::array<Grass::SearchPlantResult, GRASSF_SEARCH_COUNT> SearchPlantPos(KuroEngine::Transform arg_playerTransform);

	void UpdateGrassEasing(Grass::GrassData& arg_grass, int arg_index, KuroEngine::Vec3<float> arg_moveSpeed);
	void GrassCheckHit(Grass::GrassData& arg_grass, const std::weak_ptr<Stage>arg_nowStage);

};