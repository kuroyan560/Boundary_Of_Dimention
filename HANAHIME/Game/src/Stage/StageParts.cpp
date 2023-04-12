#include"StageParts.h"
#include"ForUser/Object/Model.h"
#include"../Graphics/BasicDraw.h"

std::array<std::string, StageParts::STAGE_PARTS_TYPE::NUM>StageParts::s_typeKeyOnJson =
{
	"Terrian","Start","Goal","MoveScaffold"
};

const std::string& StageParts::GetTypeKeyOnJson(STAGE_PARTS_TYPE arg_type)
{
	return s_typeKeyOnJson[arg_type];
}

void StageParts::Init()
{
	m_transform.SetPos(m_initializedTransform.GetPos());
	m_transform.SetScale(m_initializedTransform.GetScale());
	m_transform.SetRotate(m_initializedTransform.GetRotate());
	OnInit();
}

void StageParts::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model.lock(),
		m_transform);
}

void Terrian::BuilCollisionMesh()
{
	//当たり判定用のメッシュをモデルのメッシュに合わせる。
	int meshNum = static_cast<int>(m_model.lock()->m_meshes.size());
	m_collisionMesh.resize(meshNum);

	//当たり判定用メッシュを作成。
	for (int meshIdx = 0; meshIdx < meshNum; ++meshIdx)
	{
		auto& mesh = m_model.lock()->m_meshes[meshIdx].mesh;

		/*-- ① モデル情報から当たり判定用のポリゴンを作り出す --*/

	//当たり判定用ポリゴン
		struct Polygon {
			bool m_isActive;					//このポリゴンが有効化されているかのフラグ
			KuroEngine::ModelMesh::Vertex m_p0;	//頂点0
			KuroEngine::ModelMesh::Vertex m_p1;	//頂点1
			KuroEngine::ModelMesh::Vertex m_p2;	//頂点2
		};

		//当たり判定用ポリゴンコンテナを作成。
		m_collisionMesh[meshIdx].resize(mesh->indices.size() / static_cast<size_t>(3));

		//当たり判定用ポリゴンコンテナにデータを入れていく。
		for (auto& index : m_collisionMesh[meshIdx]) {

			// 現在のIndex数。
			int nowIndex = static_cast<int>(&index - &m_collisionMesh[0][0]);

			// 頂点情報を保存。
			index.m_p0 = mesh->vertices[mesh->indices[nowIndex * 3 + 0]];
			index.m_p1 = mesh->vertices[mesh->indices[nowIndex * 3 + 1]];
			index.m_p2 = mesh->vertices[mesh->indices[nowIndex * 3 + 2]];

			// ポリゴンを有効化。
			index.m_isActive = true;

		}

		/*-- ② ポリゴンをワールド変換する --*/
		//ワールド行列
		DirectX::XMMATRIX targetRotMat = DirectX::XMMatrixRotationQuaternion(m_initializedTransform.GetRotate());
		DirectX::XMMATRIX targetWorldMat = DirectX::XMMatrixIdentity();
		targetWorldMat *= DirectX::XMMatrixScaling(m_initializedTransform.GetScale().x, m_initializedTransform.GetScale().y, m_initializedTransform.GetScale().z);
		targetWorldMat *= targetRotMat;
		targetWorldMat.r[3].m128_f32[0] = m_initializedTransform.GetPos().x;
		targetWorldMat.r[3].m128_f32[1] = m_initializedTransform.GetPos().y;
		targetWorldMat.r[3].m128_f32[2] = m_initializedTransform.GetPos().z;
		for (auto& index : m_collisionMesh[meshIdx]) {
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
}

KuroEngine::Transform MoveScaffold::GetTransformWithKey(const KeyTransform& arg_key)
{
	KuroEngine::Transform result;
	result.SetPos(arg_key.m_translation);
	result.SetScale(arg_key.m_scaling);
	result.SetRotate(arg_key.m_rotate);
	return result;
}

void MoveScaffold::Update(Player& arg_player)
{
}