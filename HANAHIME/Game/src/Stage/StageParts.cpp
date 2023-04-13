#include"StageParts.h"
#include"ForUser/Object/Model.h"
#include"../Graphics/BasicDraw.h"
#include"../Player/Player.h"
#include"../../../../src/engine/ForUser/DrawFunc/3D/DrawFunc3D.h"

std::array<std::string, StageParts::STAGE_PARTS_TYPE::NUM>StageParts::s_typeKeyOnJson =
{
	"Terrian","Start","Goal","Appearance","MoveScaffold"
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

	OnDraw(arg_cam, arg_ligMgr);
}

void StageParts::BuilCollisionMesh()
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
			int nowIndex = static_cast<int>(&index - &m_collisionMesh[meshIdx][0]);

			// 頂点情報を保存。
			index.m_p0 = mesh->vertices[mesh->indices[nowIndex * 3 + 0]];
			index.m_p1 = mesh->vertices[mesh->indices[nowIndex * 3 + 1]];
			index.m_p2 = mesh->vertices[mesh->indices[nowIndex * 3 + 2]];

			// ポリゴンを有効化。
			index.m_isActive = true;

		}

		/*-- ② ポリゴンをワールド変換する --*/
		//ワールド行列
		DirectX::XMMATRIX targetRotMat = DirectX::XMMatrixRotationQuaternion(m_transform.GetRotate());
		DirectX::XMMATRIX targetWorldMat = DirectX::XMMatrixIdentity();
		targetWorldMat *= DirectX::XMMatrixScaling(m_transform.GetScale().x, m_transform.GetScale().y, m_transform.GetScale().z);
		targetWorldMat *= targetRotMat;
		targetWorldMat.r[3].m128_f32[0] = m_transform.GetPos().x;
		targetWorldMat.r[3].m128_f32[1] = m_transform.GetPos().y;
		targetWorldMat.r[3].m128_f32[2] = m_transform.GetPos().z;
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


void MoveScaffold::OnInit()
{
	m_isActive = false;
	m_isOder = true;
	m_nowTranslationIndex = 0;
	m_nextTranslationIndex = 0;
	m_moveLength = 0;
	m_nowMoveLength = 0;
	m_moveDir = KuroEngine::Vec3<float>();
	
	m_transform.SetPos(m_translationArray[0]);

	//当たり判定を再構築。
	BuilCollisionMesh();
}

void MoveScaffold::OnDraw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{

	//移動経路がなかったら飛ばす。
	if (m_maxTranslation < 0) return;

	//移動経路を描画する。
	for (int index = 1; index <= m_maxTranslation; ++index) {
		KuroEngine::DrawFunc3D::DrawLine(arg_cam, m_translationArray[index - 1], m_translationArray[index], KuroEngine::Color(255, 255, 255, 255), 0.1f);
	}

}

void MoveScaffold::Update(Player& arg_player)
{

	//有効化されていなかったら処理を飛ばす。
	if (!m_isActive) return;

	//ルートが設定されていない。
	assert(m_maxTranslation != 0);

	//移動した量を保存。
	m_nowMoveLength += MOVE_SPEED;

	//移動した量が規定値を超えていたら、終わった判定。
	float moveSpeed = MOVE_SPEED;
	bool isFinish = false;
	if (m_moveLength < m_nowMoveLength) {

		isFinish = true;

		//オーバーした分だけ動かす。
		moveSpeed = m_moveLength - m_nowMoveLength;

	}

	//次の地点へ向かって動かす。
	m_transform.SetPos(m_transform.GetPos() + m_moveDir * moveSpeed);

	//プレイヤーも動かす。
	if (arg_player.GetOnGimmick()) {
		arg_player.SetGimmickVel(m_moveDir * moveSpeed);
	}

	//いろいろと初期化して次向かう方向を決める。
	if (isFinish) {

		//正の方向に進むフラグだったら
		if (m_isOder) {

			//次のIndexへ
			m_nowTranslationIndex = m_nextTranslationIndex;
			++m_nextTranslationIndex;
			if (m_maxTranslation < m_nextTranslationIndex) {

				//終わっていたら
				m_nextTranslationIndex = m_maxTranslation;
				m_isOder = false;
				m_isActive = false;

			}

		}
		//負の方向に進むフラグだったら
		else {

			//次のIndexへ
			m_nowTranslationIndex = m_nextTranslationIndex;
			--m_nextTranslationIndex;
			if (m_nextTranslationIndex < 0) {

				//終わっていたら
				m_nextTranslationIndex = 0;
				m_isOder = true;
				m_isActive = false;

			}

		}

		//移動する方向と量を求める。
		m_moveDir = KuroEngine::Vec3<float>(m_translationArray[m_nextTranslationIndex] - m_translationArray[m_nowTranslationIndex]).GetNormal();
		m_moveLength = KuroEngine::Vec3<float>(m_translationArray[m_nextTranslationIndex] - m_translationArray[m_nowTranslationIndex]).Length();
		m_nowMoveLength = 0;


	}

	//当たり判定を再構築。
	BuilCollisionMesh();

}