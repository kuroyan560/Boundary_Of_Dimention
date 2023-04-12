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

//地形情報
class StageParts
{
public:
	enum STAGE_PARTS_TYPE { TERRIAN, START_POINT, GOAL_POINT, MOVE_SCAFFOLD, NUM, NONE };
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
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);

	//地形情報種別ゲッタ
	const STAGE_PARTS_TYPE& GetType()const { return m_type; }

	//トランスフォームゲッタ
	const KuroEngine::Transform& GetTransform()const { return m_transform; }
};

//ギミックとは別の通常の地形
class Terrian : public StageParts
{
	friend class Player;

	//当たり判定用ポリゴン
	struct Polygon {
		bool m_isActive;					//このポリゴンが有効化されているかのフラグ
		KuroEngine::ModelMesh::Vertex m_p0;	//頂点0
		KuroEngine::ModelMesh::Vertex m_p1;	//頂点1
		KuroEngine::ModelMesh::Vertex m_p2;	//頂点2
	};
	//当たり判定用ポリゴンコンテナを
	std::vector<std::vector<Polygon>> m_collisionMesh;

	//当たり判定用メッシュを作成。
	void BuilCollisionMesh();

public:
	Terrian(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(TERRIAN, arg_model, arg_initTransform) 
	{
		BuilCollisionMesh();
	}
};

//スタート地点
class StartPoint : public StageParts
{
public:
	StartPoint(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(START_POINT, arg_model, arg_initTransform) {}
};

//ゴール地点
class GoalPoint : public StageParts
{
public:
	GoalPoint(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(GOAL_POINT, arg_model, arg_initTransform) {}
};

//動く足場
class MoveScaffold : public StageParts
{
public:
	//挙動
	class KeyTransform
	{
	public:
		KuroEngine::Quaternion m_rotate;
		KuroEngine::Vec3<float>m_translation;
		KuroEngine::Vec3<float>m_scaling;
	};

private:
	//トランスフォームの配列（0からスタート、他のTerrian以外のパーツに当たるか最後まで到達したら折り返し）
	std::vector<KeyTransform>m_transformArray;

	//KeyTransformからTransformを構築
	KuroEngine::Transform GetTransformWithKey(const KeyTransform& arg_key);

public:
	MoveScaffold(std::weak_ptr<KuroEngine::Model>arg_model, std::vector<KeyTransform>arg_transformArray)
		:StageParts(MOVE_SCAFFOLD, arg_model, GetTransformWithKey(arg_transformArray[0])) {}
};
