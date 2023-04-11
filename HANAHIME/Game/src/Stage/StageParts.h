#pragma once
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
protected:
	//モデルポインタ
	std::weak_ptr<KuroEngine::Model>m_model;
	//デフォルト（元データ）のトランスフォーム
	const KuroEngine::Transform m_initializedTransform;
	//トランスフォーム
	KuroEngine::Transform m_transform;

	virtual void OnInit() {};

public:
	StageParts(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:m_model(arg_model), m_initializedTransform(arg_initTransform) {}
	virtual ~StageParts() {}

	void Init(float arg_scaling);
	void Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr);
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
protected:
	void OnInit()override;

public:
	Terrian(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_initTransform)
		:StageParts(arg_model, arg_initTransform) {}
};