#include "Stage.h"
#include"FrameWork/Importer.h"
#include"ForUser/JsonData.h"
#include"FrameWork/Importer.h"
#include"../Graphics/BasicDraw.h"
#include"../../../../src/engine/ForUser/Object/Model.h"

std::string Stage::s_terrianModelDir = "resource/user/model/terrian/";

Stage::Stage()
{
	using namespace KuroEngine;

	//デフォルトのモデル
		//スカイドーム
	static std::shared_ptr<Model>s_defaultSkydomeModel
		= Importer::Instance()->LoadModel("resource/user/model/", "Skydome.glb");
	//森林円柱
	static std::shared_ptr<Model>s_defaultWoodsCylinderModel
		= Importer::Instance()->LoadModel("resource/user/model/", "Woods.glb");

	//デフォルトの画像
		//地面
	static std::shared_ptr<TextureBuffer>s_defaultGroundTex
		= D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/ground.png");

	//デフォルト値設定
	m_skydomeModel = s_defaultSkydomeModel;
	m_woodsCylinderModel = s_defaultWoodsCylinderModel;
	m_groundTex = s_defaultGroundTex;
}

void Stage::TerrianInit(float arg_scaling)
{
	for (auto& terrian : m_terrianArray)
	{
		terrian.m_transform.SetPos(terrian.m_initializedTransform.GetPos() * arg_scaling);
		terrian.m_transform.SetScale(terrian.m_initializedTransform.GetScale() * arg_scaling);
		terrian.m_transform.SetRotate(terrian.m_initializedTransform.GetRotate());

		//当たり判定用のメッシュをモデルのメッシュに合わせる。
		terrian.m_collisionMesh.resize(static_cast<int>(terrian.m_model.lock()->m_meshes.size()));

		//当たり判定用メッシュを作成。
		for (auto& index : terrian.m_model.lock()->m_meshes) {

			BuilCollisionMesh(terrian, index, static_cast<int>(&index - &(terrian.m_model.lock()->m_meshes[0])));

		}

	}

}

void Stage::TerrianDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	for (auto& terrian : m_terrianArray)
	{
		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			terrian.m_model.lock(),
			terrian.m_transform);
	}
}

void Stage::Load(std::string arg_dir, std::string arg_fileName)
{
	using namespace KuroEngine;

	//地形情報クリア
	m_terrianArray.clear();

	JsonData jsonData(arg_dir, arg_fileName);
	for (auto& obj : jsonData.m_jsonData["objects"])
	{
		//地形の名前のパラメータがない
		if (!obj.contains("name"))continue;

		//モデルの名前のパラメータがない
		if (!obj.contains("file_name"))continue;

		//トランスフォームのパラメータがない
		if (!obj.contains("transform"))continue;

		//地形の名前設定
		auto name = obj["name"].get<std::string>();

		//モデル設定
		auto model = Importer::Instance()->LoadModel(s_terrianModelDir, obj["file_name"].get<std::string>() + ".glb");

		//トランスフォーム取得
		auto transformObj = obj["transform"];

		//平行移動
		Vec3<float>translation = { -(float)transformObj["translation"][0],(float)transformObj["translation"][2],-(float)transformObj["translation"][1] };

		//回転
		Vec3<float>rotate = { -(float)transformObj["rotation"][1],-(float)transformObj["rotation"][2], (float)transformObj["rotation"][0] };
		//ラジアンに直す
		rotate.x = Angle::ConvertToRadian(rotate.x);
		rotate.y = Angle::ConvertToRadian(rotate.y);
		rotate.z = Angle::ConvertToRadian(rotate.z);

		//スケーリング
		Vec3<float>scaling = { (float)transformObj["scaling"][0],(float)transformObj["scaling"][2] ,(float)transformObj["scaling"][1] };

		//トランスフォーム設定
		Transform transform;
		transform.SetPos(translation);
		transform.SetRotate(XMQuaternionRotationRollPitchYaw(rotate.z, rotate.y, rotate.x));
		transform.SetScale(scaling);

		//地形追加
		m_terrianArray.emplace_back(name, model, transform);
	}
}

void Stage::BuilCollisionMesh(Terrian& arg_terrian, KuroEngine::ModelMesh& arg_mesh, int arg_meshIndex) {

	/*-- ① モデル情報から当たり判定用のポリゴンを作り出す --*/

	//当たり判定用ポリゴン
	struct Polygon {
		bool m_isActive;					//このポリゴンが有効化されているかのフラグ
		KuroEngine::ModelMesh::Vertex m_p0;	//頂点0
		KuroEngine::ModelMesh::Vertex m_p1;	//頂点1
		KuroEngine::ModelMesh::Vertex m_p2;	//頂点2
	};

	//当たり判定用ポリゴンコンテナを作成。
	arg_terrian.m_collisionMesh[arg_meshIndex].resize(arg_mesh.mesh->indices.size() / static_cast<size_t>(3));

	//当たり判定用ポリゴンコンテナにデータを入れていく。
	for (auto& index : arg_terrian.m_collisionMesh[arg_meshIndex]) {

		// 現在のIndex数。
		int nowIndex = static_cast<int>(&index - &arg_terrian.m_collisionMesh[0][0]);
		
		// 頂点情報を保存。
		index.m_p0 = arg_mesh.mesh->vertices[arg_mesh.mesh->indices[nowIndex * 3 + 0]];
		index.m_p1 = arg_mesh.mesh->vertices[arg_mesh.mesh->indices[nowIndex * 3 + 1]];
		index.m_p2 = arg_mesh.mesh->vertices[arg_mesh.mesh->indices[nowIndex * 3 + 2]];

		// ポリゴンを有効化。
		index.m_isActive = true;

	}


	/*-- ② ポリゴンをワールド変換する --*/

	//ワールド行列
	DirectX::XMMATRIX targetRotMat = DirectX::XMMatrixRotationQuaternion(arg_terrian.m_transform.GetRotate());
	DirectX::XMMATRIX targetWorldMat = DirectX::XMMatrixIdentity();
	targetWorldMat *= DirectX::XMMatrixScaling(arg_terrian.m_transform.GetScale().x, arg_terrian.m_transform.GetScale().y, arg_terrian.m_transform.GetScale().z);
	targetWorldMat *= targetRotMat;
	targetWorldMat.r[3].m128_f32[0] = arg_terrian.m_transform.GetPos().x;
	targetWorldMat.r[3].m128_f32[1] = arg_terrian.m_transform.GetPos().y;
	targetWorldMat.r[3].m128_f32[2] = arg_terrian.m_transform.GetPos().z;
	for (auto& index : arg_terrian.m_collisionMesh[arg_meshIndex]) {
		//頂点を変換
		index.m_p0.pos = KuroEngine::Math::TransformVec3(index.m_p0.pos, targetWorldMat);
		index.m_p1.pos = KuroEngine::Math::TransformVec3(index.m_p1.pos, targetWorldMat);
		index.m_p2.pos = KuroEngine::Math::TransformVec3(index.m_p2.pos, targetWorldMat);
		//法線を回転行列分だけ変換
		index.m_p0.normal = KuroEngine::Math::TransformVec3(index.m_p0.normal, targetRotMat);
		index.m_p0.normal.Normalize();
		index.m_p1.normal = KuroEngine::Math::TransformVec3(index.m_p1.normal, targetRotMat);
		index.m_p1.normal.Normalize();
		index.m_p2.normal = KuroEngine::Math::TransformVec3(index.m_p2.normal, targetRotMat);
		index.m_p2.normal.Normalize();
	}

}
