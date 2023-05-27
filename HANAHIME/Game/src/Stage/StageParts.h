#pragma once
#pragma once
#include<array>
#include<memory>
#include<optional>
#include"Common/Transform.h"
#include"Render/RenderObject/ModelInfo/ModelMesh.h"
#include"../Graphics/BasicDrawParameters.h"
#include"ForUser/Timer.h"
#include"../HUD/CheckPointUI.h"

namespace KuroEngine
{
	class Model;
	class Camera;
	class LightManager;
	class ModelAnimator;
}

class Player;
class Switch;

//当たり判定用ポリゴン
struct TerrianHitPolygon
{
	bool m_isActive;					//このポリゴンが有効化されているかのフラグ
	KuroEngine::ModelMesh::Vertex m_p0;	//頂点0
	KuroEngine::ModelMesh::Vertex m_p1;	//頂点1
	KuroEngine::ModelMesh::Vertex m_p2;	//頂点2
};
struct AABB {
	KuroEngine::Vec3<float> m_min;
	KuroEngine::Vec3<float> m_max;
	struct CollisionInfo {
		KuroEngine::Vec3<float> m_pushBack;
	};
	//AABBの当たり判定
	std::optional<CollisionInfo> CheckAABBCollision(const AABB& arg_aabb1);
};

class TerrianMeshCollider
{
public:
	//当たり判定用ポリゴンコンテナ
	std::vector<std::vector<TerrianHitPolygon>> m_collisionMesh;
	//メッシュとメッシュの当たり判定に使用する、ポリゴンから作られた立方体コンテナ
	std::vector<std::vector<AABB>> m_aabb;
	const float CUBE_Z = 5.0f;

public:
	//当たり判定用メッシュを作成。
	void BuilCollisionMesh(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_transform);
	//当たり判定用ポリゴンコンテナゲッタ
	const std::vector<std::vector<TerrianHitPolygon>>& GetCollisionMesh()const { return m_collisionMesh; }
private:
	//3頂点から立方体を生成する。
	AABB CreateCubeFromPolygon(const KuroEngine::Vec3<float>& arg_v1, const KuroEngine::Vec3<float>& arg_v2, const KuroEngine::Vec3<float>& arg_v3, const KuroEngine::Vec3<float>& arg_normal);
};

//地形情報
class StageParts
{
public:
	enum STAGE_PARTS_TYPE
	{
		//地形など動的オブジェクト
		TERRIAN,
		START_POINT,
		GOAL_POINT,
		APPEARANCE,
		MOVE_SCAFFOLD,
		LEVER,
		IVY_ZIP_LINE,
		IVY_BLOCK,
		SPLATOON_FENCE,
		GATE,
		CHECK_POINT,
		STAR_COIN,
		BACKGROUND,

		//敵など静的オブジェクト
		MINI_BUG,	//チビ虫
		DOSSUN_RING,	//ドッスンリング
		BATTERY,	//砲台

		NUM, NONE
	};
	static const std::string& GetTypeKeyOnJson(STAGE_PARTS_TYPE arg_type);

private:
	static std::array<std::string, STAGE_PARTS_TYPE::NUM>s_typeKeyOnJson;

protected:
	//地形情報種別
	const STAGE_PARTS_TYPE m_type = NONE;
	//モデルポインタ
	std::weak_ptr<KuroEngine::Model>m_model;
	//デフォルト（元データ）のトランスフォーム
	const KuroEngine::Transform m_initializedTransform;
	//トランスフォーム
	KuroEngine::Transform m_transform;
	//現在のフレームで移動した量。
	KuroEngine::Vec3<float> m_moveAmount;

	virtual void OnInit() {};
public:
	StageParts(STAGE_PARTS_TYPE arg_type, std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:m_type(arg_type), m_model(arg_model), m_initializedTransform(arg_initTransform), m_transform(arg_initTransform)
	{
	}
	virtual ~StageParts() {}

	void Init();
	virtual void Update(Player& arg_player) = 0;
	virtual void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//地形情報種別ゲッタ
	const STAGE_PARTS_TYPE& GetType()const { return m_type; }

	//トランスフォームゲッタ
	KuroEngine::Transform& GetTransform() { return m_transform; }
	const KuroEngine::Transform& GetInitTransform() { return m_initializedTransform; }

	void SetTransform(const KuroEngine::Transform& transform)
	{
		m_transform = transform;
	}
	KuroEngine::Vec3<float> GetMoveAmount() { return m_moveAmount; }

	//ステージ情報のゲッタ
	const std::weak_ptr<KuroEngine::Model>& GetModel()const { return m_model; }
};

//ギミックとは別の通常の地形
class Terrian : public StageParts
{
	TerrianMeshCollider m_collider;

public:
	Terrian(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, std::weak_ptr<KuroEngine::Model>arg_collisionModel)
		:StageParts(TERRIAN, arg_model, arg_initTransform)
	{
		m_collider.BuilCollisionMesh(arg_collisionModel, m_initializedTransform);
	}
	void Update(Player& arg_player)override {}
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;
	const std::vector<std::vector<TerrianHitPolygon>>& GetCollisionMesh()const { return m_collider.GetCollisionMesh(); }
};

//スタート地点
class StartPoint : public StageParts
{
public:
	StartPoint(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(START_POINT, arg_model, arg_initTransform) {}
	void Update(Player& arg_player)override {}
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override {};
};

//ゴール地点
class GoalPoint : public StageParts
{
	bool m_hitPlayer = false;

public:
	GoalPoint(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(GOAL_POINT, arg_model, arg_initTransform) {
	}
	void OnInit()override
	{
		m_offset.SetScale({ 0.0f, 0.0f, 0.0f });
	}
	void Update(Player& arg_player)override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	const bool& HitPlayer()const { return m_hitPlayer; }

	//描画時のオフセット
	KuroEngine::Transform m_offset;
};

//見かけだけのオブジェクト（描画だけで何もしない）
class Appearance : public StageParts
{
	static std::map<std::string, std::weak_ptr<KuroEngine::Model>>s_models;

	TerrianMeshCollider m_collider;
public:
	static void ModelsUpdate();

	Appearance(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, std::weak_ptr<KuroEngine::Model>arg_collisionModel);
	void Update(Player& arg_player)override {}
	const std::vector<std::vector<TerrianHitPolygon>>& GetCollisionMesh()const { return m_collider.GetCollisionMesh(); }
};

//動く足場
class MoveScaffold : public StageParts
{
public:
	TerrianMeshCollider m_collider;
private:

	//トランスフォームの配列（0からスタート、他のTerrian以外のパーツに当たるか最後まで到達したら折り返し）
	std::vector<KuroEngine::Vec3<float>>m_translationArray;

	int m_maxTranslation;		//移動する地点 - 1の数
	int m_nowTranslationIndex;	//現在の移動する地点のIndex
	int m_nextTranslationIndex;	//次の移動する地点のIndex
	float m_moveLength;			//次の地点まで移動する量
	float m_nowMoveLength;		//移動している現在量
	KuroEngine::Vec3<float> m_moveDir;	//移動方向
	bool m_isOder;				//順番に移動

	//プレイヤーが乗ってから動き出すまでのタイマー
	KuroEngine::Timer m_moveStartTimer;
	const float MOVE_START_TIMER = 30.0f;

	//有効化フラグ
	bool m_isActive;
	bool m_isOldActive;
	bool m_prevOnPlayer;
	bool m_onPlayer;	//プレイヤーが乗っているか
	bool m_isStop;		//別の動く足場とぶつかって一時停止したか。
	bool m_isOldStop;		//別の動く足場とぶつかって一時停止したか。

	//座標関連
	KuroEngine::Vec3<float> m_nowPos;
	KuroEngine::Vec3<float> m_oldPos;

	const float MOVE_SPEED = 0.4f;

	std::weak_ptr<KuroEngine::Model>m_collisionModel;

public:
	MoveScaffold(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, std::vector<KuroEngine::Vec3<float>>arg_translationArray, std::weak_ptr<KuroEngine::Model>arg_collisionModel)
		:StageParts(MOVE_SCAFFOLD, arg_model, arg_initTransform), m_translationArray(arg_translationArray)
	{
		m_maxTranslation = static_cast<int>(arg_translationArray.size()) - 1;
		m_isOldActive = false;
		m_collisionModel = arg_collisionModel;
	}

	void OnInit()override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	void Update(Player& arg_player)override;

	const std::vector<std::vector<TerrianHitPolygon>>& GetCollisionMesh()const { return m_collider.GetCollisionMesh(); }

	//有効化 無効化
	void Activate() { m_isActive = true; }
	void Deactivate() { m_isActive = false; }
	bool GetIsActive() { return m_isActive; }
	void Stop() { m_isStop = true; }	//地形同士がぶつかったときに一時停止する処理。

	//プレイヤーが上に乗っているかのフラグを切り替える。
	void SetOnPlayer(bool arg_onPlayer) { m_onPlayer = arg_onPlayer; }
	void OnPlayer();

	//当たり判定再構築
	void ReBuildCollisionMesh();

	//押し戻す。
	void PushBack(KuroEngine::Vec3<float> arg_pushBack);

	//移動した量
	KuroEngine::Vec3<float> GetNowPos() { return m_nowPos; }
	KuroEngine::Vec3<float> GetOldPos() { return m_oldPos; }
};

//レバー
class Lever :public StageParts
{
public:
	static const int INVALID_ID = -1;
	static const std::string TURN_ON_ANIM_NAME;
	static const std::string TURN_OFF_ANIM_NAME;

private:
	friend class Stage;

	//識別番号
	int m_id = INVALID_ID;

	//動作するスイッチのポインタ
	Switch* m_parentSwitch = nullptr;

	//箱コライダーの情報
	struct BoxCollider
	{
		//中心座標
		KuroEngine::Vec3<float>m_center = { 0,0,0 };
		//大きさ
		KuroEngine::Vec3<float>m_size = { 1,1,1 };
	}m_boxCollider;

	//初期化時のフラグ
	const bool m_initFlg = false;

	//衝突判定トリガー用変数
	bool m_isHit;
	bool m_isOldHit;

	//オンオフ
	bool m_flg = false;

	std::shared_ptr<KuroEngine::ModelAnimator>m_modelAnimator;

public:
	Lever(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, int arg_id, bool arg_initFlg = false);

	void OnInit()override;
	void Update(Player& arg_player)override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	const bool& GetLeverFlg()const { return m_flg; }

};

//ジップライン蔓
class IvyZipLine : public StageParts
{
	//トランスフォームの配列（0からスタート、最後まで到達したら折り返し）
	std::vector<KuroEngine::Vec3<float>>m_translationArray;

	bool m_isActive;
	bool m_isHitStartPos;
	bool m_isReadyPlayer;	//プレイヤー側で動かす準備ができたか。

	int m_maxTranslation;		//移動する地点 - 1の数
	int m_nowTranslationIndex;	//現在の移動する地点のIndex
	int m_nextTranslationIndex;	//次の移動する地点のIndex
	float m_moveLength;			//次の地点まで移動する量
	float m_nowMoveLength;		//移動している現在量
	KuroEngine::Vec3<float> m_moveDir;	//移動方向

public:

	//蔦に乗ることができる範囲
	const float JUMP_SCALE = 2.0f;
	const float ZIPLINE_SPEED = 1.0f;

public:
	IvyZipLine(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, std::vector<KuroEngine::Vec3<float>>arg_translationArray)
		:StageParts(IVY_ZIP_LINE, arg_model, arg_initTransform), m_translationArray(arg_translationArray) {

		//デバッグ用で先頭のジップラインのY座標を下げる。
		m_translationArray.front().x -= 5.0f;
		m_translationArray.front().y -= 6.0f;
		m_translationArray.front().z += 15.0f;

		//ジップラインを追加
		m_translationArray.emplace_back(m_translationArray.back());
		m_translationArray.back().x -= 0.0f;
		m_translationArray.back().z += 30.0f;

		//ジップラインを追加
		m_translationArray.emplace_back(m_translationArray.back());
		m_translationArray.back().x -= 6.0f;
		m_translationArray.back().y += 1.0f;

		//ジップライン移動に必要な変数を初期化
		m_maxTranslation = static_cast<int>(m_translationArray.size()) - 1;
		m_nowTranslationIndex = 0;
		m_nextTranslationIndex = 0;
		m_moveLength = 0;
		m_nowMoveLength = 0;

		m_isActive = false;
		m_isReadyPlayer = false;

	}

	void Update(Player& arg_player)override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	//始点か終点に当たったら
	void CheckHit(bool arg_isHitStartPos);

	//ジップラインの制御点の数
	int GetTranslationArraySize() { return static_cast<int>(m_translationArray.size()); }

	//プレイヤーは動ける。
	void CanMovePlayer() { m_isReadyPlayer = true; }

	//始点と終点を取得
	KuroEngine::Vec3<float> GetStartPoint() { return m_translationArray.front(); }
	KuroEngine::Vec3<float> GetEndPoint() { return m_translationArray.back(); }


	//イージングの始点と終点を取得する関数。
	KuroEngine::Vec3<float> GetPoint(bool arg_isEaseStart);

};

//蔓ブロック（消えたり出現したりするブロック）
class IvyBlock : public StageParts
{
	//消失している間に描画するモデル
	std::shared_ptr<KuroEngine::Model>m_nonExistModel;
	//消失している間に描画するモデルのマテリアル
	std::shared_ptr<KuroEngine::Material>m_nonExistMaterial;
	IndividualDrawParameter m_nonExistDrawParam;


	//ブロックの左上手前座標
	KuroEngine::Vec3<float>m_leftTopFront;
	//ブロックの右下奥座標
	KuroEngine::Vec3<float>m_rightBottomBack;
	//当たり判定用コリジョン
	TerrianMeshCollider m_collider;
	//出現中か
	bool m_isAppear;
	bool m_prevOnPlayer;
	bool m_onPlayer;	//プレイヤーが乗っているか

	//イージングタイマー系
	float m_easingTimer;
	const float EASING_TIMER = 0.03f;

	const float HIT_SCALE_MIN = 7.5f * 2.0f;
	const float HIT_SCALE_MAX = 15.5f * 2.0f;
	const float SCALE_DEF = 15.0f;

	std::weak_ptr<KuroEngine::Model>m_collisionModel;

	void ReuilCollisionMesh()
	{
		m_collider.BuilCollisionMesh(m_collisionModel, m_transform);
	}

public:
	IvyBlock(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, KuroEngine::Vec3<float>arg_leftTopFront, KuroEngine::Vec3<float>arg_rightBottomBack, std::weak_ptr<KuroEngine::Model>arg_collisionModel);

	void Update(Player& arg_player)override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	void OnInit()override
	{
		m_isAppear = true;
		m_onPlayer = false;
		m_prevOnPlayer = false;
		m_easingTimer = EASING_TIMER;

		ReuilCollisionMesh();

		m_nonExistDrawParam.m_alpha = 0.0f;
	}

	void Appear();
	void Disappear();

	void OnPlayer() { m_onPlayer = true; }
	void OffPlayer() { m_onPlayer = false; }
	bool GetOnPlayer() { return m_onPlayer; }
	bool GetIsAppear() { return m_isAppear; }

	//座標を取得
	KuroEngine::Vec3<float> GetPos() { return m_transform.GetPosWorld(); }

	float GetHitScaleMin() { return HIT_SCALE_MIN; }
	float GetHitScaleMax() { return HIT_SCALE_MAX; }

	const std::vector<std::vector<TerrianHitPolygon>>& GetCollisionMesh()const { return m_collider.GetCollisionMesh(); }
};

//スプラトゥーン風フェンス（くぐって通る）
class SplatoonFence : public StageParts
{
public:
	TerrianMeshCollider m_collider;
public:
	SplatoonFence(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, std::weak_ptr<KuroEngine::Model>arg_collisionModel)
		:StageParts(SPLATOON_FENCE, arg_model, arg_initTransform) {
		m_collider.BuilCollisionMesh(arg_collisionModel, m_initializedTransform);
	}
	void Update(Player& arg_player)override {};
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	const std::vector<std::vector<TerrianHitPolygon>>& GetCollisionMesh()const { return m_collider.GetCollisionMesh(); }
};

class Gate : public StageParts
{
	static const int INVALID_STAGE_NUM = -1;
	static int s_destStageNum;

	//ゲートのテクスチャ
	static const int GATE_TEX_ARRAY_SIZE = 6;
	static std::array<std::shared_ptr<KuroEngine::TextureBuffer>, GATE_TEX_ARRAY_SIZE>s_texArray;
	//テクスチャアニメーション
	int m_texIdx = 0;
	KuroEngine::Timer m_animTimer;

	//演出タイマー
	KuroEngine::Angle m_effectSinCurveAngle;
	float m_effectScale = 1.0f;

	int m_id;
	int m_destStageNum;
	int m_destGateId;


	//出口専用
	bool IsExit() 
	{
		return m_destStageNum == INVALID_STAGE_NUM;
	}

	void OnInit()override
	{
		m_effectSinCurveAngle = 0;
	}

public:
	Gate(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, int arg_id, int arg_destStageNum, int arg_destGateId);

	void Update(Player& arg_player)override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	bool CheckID(int arg_id);
	const int& GetDestStageNum()const { return m_destStageNum; }
	const int& GetDestGateID()const { return m_destGateId; }
};

class CheckPoint : public StageParts
{
	//UI
	static std::shared_ptr<CheckPointUI> s_ui;
	//最後に訪れたチェックポイントのトランスフォーム
	static KuroEngine::Transform s_latestVisitTransform;
	//チェックポイントを１つでも訪れたか
	static bool s_visit;

	int m_order;
	bool m_touched = false;

public:
	static std::weak_ptr<CheckPointUI>UI() 
	{
		if (!s_ui)s_ui = std::make_shared<CheckPointUI>();
		return s_ui; 
	}

	//最後に訪れたチェックポイントのトランスフォームゲッタ
	static KuroEngine::Transform GetLatestVistTransform(const KuroEngine::Transform& arg_alternative)
	{
		//まだ１つも訪れていないなら引数のトランスフォームをそのまま返す
		if (!s_visit)return arg_alternative;
		return s_latestVisitTransform;
	}

	CheckPoint(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, int arg_order);

	void Update(Player& arg_player)override;
};

class StarCoin : public StageParts
{
private:
	//累計の入手数
	static int GET_SUM;
public:
	static const int& GetFlowerSum() { return GET_SUM; }

private:
	bool m_get = false;
	bool m_touched = false;

	void OnInit()override;
public:
	StarCoin(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(CHECK_POINT, arg_model, arg_initTransform) {}

	void Update(Player& arg_player)override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	const bool& IsGet()const { return m_get; }
};

class BackGround : public StageParts
{
public:
	BackGround(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(BACKGROUND, arg_model, arg_initTransform) {}

	void Update(Player& arg_player)override {};
};