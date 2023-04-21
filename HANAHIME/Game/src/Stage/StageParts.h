#pragma once
#include<array>
#include<memory>
#include"Common/Transform.h"
#include"../../../../src/engine/Render/RenderObject/ModelInfo/ModelMesh.h"

namespace KuroEngine
{
	class Model;
	class Camera;
	class LightManager;
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

class TerrianMeshCollider
{
public:
	//当たり判定用ポリゴンコンテナ
	std::vector<std::vector<TerrianHitPolygon>> m_collisionMesh;

public:
	//当たり判定用メッシュを作成。
	void BuilCollisionMesh(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_transform);
	//当たり判定用ポリゴンコンテナゲッタ
	const std::vector<std::vector<TerrianHitPolygon>>& GetCollisionMesh()const { return m_collisionMesh; }
};

//地形情報
class StageParts
{
public:
	enum STAGE_PARTS_TYPE { TERRIAN, START_POINT, GOAL_POINT, APPEARANCE, MOVE_SCAFFOLD, LEVER, IVY_ZIP_LINE, IVY_BLOCK, NUM, NONE };
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

	virtual void OnInit() {};

public:
	StageParts(STAGE_PARTS_TYPE arg_type, std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:m_type(arg_type), m_model(arg_model), m_initializedTransform(arg_initTransform), m_transform(arg_initTransform) {}
	virtual ~StageParts() {}

	void Init();
	virtual void Update(Player& arg_player) = 0;
	virtual void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//地形情報種別ゲッタ
	const STAGE_PARTS_TYPE& GetType()const { return m_type; }

	//トランスフォームゲッタ
	const KuroEngine::Transform& GetTransform()const { return m_transform; }
	void SetTransform(const KuroEngine::Transform& transform)
	{
		m_transform = transform;
	}

	//ステージ情報のゲッタ
	const std::weak_ptr<KuroEngine::Model>& GetModel()const { return m_model; }
};

//ギミックとは別の通常の地形
class Terrian : public StageParts
{
	TerrianMeshCollider m_collider;

public:
	Terrian(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(TERRIAN, arg_model, arg_initTransform)
	{
		m_collider.BuilCollisionMesh(arg_model, arg_initTransform);
	}
	void Update(Player& arg_player)override {}
	const std::vector<std::vector<TerrianHitPolygon>>& GetCollisionMesh()const { return m_collider.GetCollisionMesh(); }
};

//スタート地点
class StartPoint : public StageParts
{
public:
	StartPoint(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(START_POINT, arg_model, arg_initTransform) {}
	void Update(Player& arg_player)override {}
};

//ゴール地点
class GoalPoint : public StageParts
{
	bool m_hitPlayer = false;
public:
	GoalPoint(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(GOAL_POINT, arg_model, arg_initTransform) {}
	void Update(Player& arg_player)override;

	const bool& HitPlayer()const { return m_hitPlayer; }
};

//見かけだけのオブジェクト（描画だけで何もしない）
class Appearance : public StageParts
{
public:
	Appearance(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(APPEARANCE, arg_model, arg_initTransform)
	{
	}
	void Update(Player& arg_player)override {}
};

//動く足場
class MoveScaffold : public StageParts
{
private:
	TerrianMeshCollider m_collider;

	//トランスフォームの配列（0からスタート、他のTerrian以外のパーツに当たるか最後まで到達したら折り返し）
	std::vector<KuroEngine::Vec3<float>>m_translationArray;

	int m_maxTranslation;		//移動する地点 - 1の数
	int m_nowTranslationIndex;	//現在の移動する地点のIndex
	int m_nextTranslationIndex;	//次の移動する地点のIndex
	float m_moveLength;			//次の地点まで移動する量
	float m_nowMoveLength;		//移動している現在量
	KuroEngine::Vec3<float> m_moveDir;	//移動方向
	bool m_isOder;				//順番に移動

	//有効化フラグ
	bool m_isActive;

	//座標関連
	KuroEngine::Vec3<float> m_nowPos;
	KuroEngine::Vec3<float> m_oldPos;

	const float MOVE_SPEED = 0.1f;

public:
	MoveScaffold(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, std::vector<KuroEngine::Vec3<float>>arg_translationArray)
		:StageParts(MOVE_SCAFFOLD, arg_model, arg_initTransform), m_translationArray(arg_translationArray)
	{
		m_maxTranslation = static_cast<int>(arg_translationArray.size()) - 1;
	}

	void OnInit()override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;

	void Update(Player& arg_player)override;

	const std::vector<std::vector<TerrianHitPolygon>>& GetCollisionMesh()const { return m_collider.GetCollisionMesh(); }

	//有効化 無効化
	void Activate() { m_isActive = true; }
	void Deactivate() { m_isActive = false; }
	bool GetIsActive() { return m_isActive; }

	//移動した量
	KuroEngine::Vec3<float> GetNowPos() { return m_nowPos; }
	KuroEngine::Vec3<float> GetOldPos() { return m_oldPos; }
};

//レバー
class Lever :public StageParts
{
public:
	static const int INVALID_ID = -1;

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

public:
	Lever(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, int arg_id, bool arg_initFlg = false)
		:StageParts(LEVER, arg_model, arg_initTransform), m_id(arg_id), m_initFlg(arg_initFlg)
	{
		m_boxCollider.m_center = arg_initTransform.GetPosWorld();
		m_boxCollider.m_size = arg_initTransform.GetScale();
	}

	void OnInit()override
	{
		m_flg = m_initFlg;
		m_isHit = false;
		m_isOldHit = false;
	}
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
		m_translationArray.back().y += 2.0f;

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
	//ブロックの左上手前座標
	KuroEngine::Vec3<float>m_leftTopFront;
	//ブロックの右下奥座標
	KuroEngine::Vec3<float>m_rightBottomBack;

public:
	IvyBlock(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform, KuroEngine::Vec3<float>arg_leftTopFront, KuroEngine::Vec3<float>arg_rightBottomBack)
		:StageParts(IVY_BLOCK, arg_model, arg_initTransform), m_leftTopFront(arg_leftTopFront), m_rightBottomBack(arg_rightBottomBack) {
	}

	void Update(Player& arg_player)override;
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)override;
};
