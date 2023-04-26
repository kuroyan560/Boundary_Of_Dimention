#include"StageParts.h"
#include"ForUser/Object/Model.h"
#include"../Graphics/BasicDraw.h"
#include"../Player/Player.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"Switch.h"
#include"ForUser/DrawFunc/BillBoard/DrawFuncBillBoard.h"
#include"../SoundConfig.h"
#include"../Player/PlayerCollision.h"
#include"../TimeScaleMgr.h"

std::array<std::string, StageParts::STAGE_PARTS_TYPE::NUM>StageParts::s_typeKeyOnJson =
{
	"Terrian","Start","Goal","Appearance","MoveScaffold","Lever","Ivy","IvyBlock"
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

void TerrianMeshCollider::BuilCollisionMesh(std::weak_ptr<KuroEngine::Model>arg_model, KuroEngine::Transform arg_transform)
{
	//当たり判定用のメッシュをモデルのメッシュに合わせる。
	int meshNum = static_cast<int>(arg_model.lock()->m_meshes.size());
	m_collisionMesh.resize(meshNum);

	//当たり判定用メッシュを作成。
	for (int meshIdx = 0; meshIdx < meshNum; ++meshIdx)
	{
		auto& mesh = arg_model.lock()->m_meshes[meshIdx].mesh;

		/*-- ① モデル情報から当たり判定用のポリゴンを作り出す --*/

	//当たり判定用ポリゴン
		struct TerrianHitPolygon {
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
		DirectX::XMMATRIX targetRotMat = DirectX::XMMatrixRotationQuaternion(arg_transform.GetRotate());
		DirectX::XMMATRIX targetWorldMat = DirectX::XMMatrixIdentity();
		targetWorldMat *= DirectX::XMMatrixScaling(arg_transform.GetScale().x, arg_transform.GetScale().y, arg_transform.GetScale().z);
		targetWorldMat *= targetRotMat;
		targetWorldMat.r[3].m128_f32[0] = arg_transform.GetPos().x;
		targetWorldMat.r[3].m128_f32[1] = arg_transform.GetPos().y;
		targetWorldMat.r[3].m128_f32[2] = arg_transform.GetPos().z;
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

	//上記で作った当たり判定用ポリゴンを元に、当たり判定用のBOXを作る。
	m_aabb.clear();
	for (auto& stage : m_collisionMesh) {

		//格納するデータを保存。
		m_aabb.emplace_back();

		for (auto& index : stage) {

			//当たり判定BOXを入れるデータ
			m_aabb.back().emplace_back(CreateCubeFromPolygon(index.m_p0.pos, index.m_p1.pos, index.m_p2.pos, index.m_p0.normal));

		}

	}

}

AABB TerrianMeshCollider::CreateCubeFromPolygon(const KuroEngine::Vec3<float>& arg_v1, const KuroEngine::Vec3<float>& arg_v2, const KuroEngine::Vec3<float>& arg_v3, const KuroEngine::Vec3<float>& arg_normal) {

	KuroEngine::Vec3<float> edge1 = arg_v2 - arg_v1;
	KuroEngine::Vec3<float> edge2 = arg_v3 - arg_v1;

	KuroEngine::Vec3<float> inward_normal = arg_normal * (-1);
	KuroEngine::Vec3<float> inward_offset = inward_normal * edge1.Length() / std::sqrt(2.0f);

	std::array<KuroEngine::Vec3<float>, 8> cubeVertices;
	cubeVertices[0] = arg_v1;
	cubeVertices[1] = arg_v2;
	cubeVertices[2] = arg_v3;
	cubeVertices[3] = arg_v1 + edge2;
	cubeVertices[4] = arg_v1 + inward_offset;
	cubeVertices[5] = arg_v2 + inward_offset;
	cubeVertices[6] = arg_v3 + inward_offset;
	cubeVertices[7] = arg_v1 + edge2 + inward_offset;

	AABB aabb;
	aabb.m_min = cubeVertices[0];
	aabb.m_max = cubeVertices[0];

	for (auto& index : cubeVertices) {
		aabb.m_min.x = std::min(aabb.m_min.x, index.x);
		aabb.m_min.y = std::min(aabb.m_min.y, index.y);
		aabb.m_min.z = std::min(aabb.m_min.z, index.z);
		aabb.m_max.x = std::max(aabb.m_max.x, index.x);
		aabb.m_max.y = std::max(aabb.m_max.y, index.y);
		aabb.m_max.z = std::max(aabb.m_max.z, index.z);
	}

	return aabb;
}

std::optional<AABB::CollisionInfo> AABB::CheckAABBCollision(const AABB& arg_aabb1) {
	KuroEngine::Vec3<float> pushBack(0.0f, 0.0f, 0.0f);
	float minOverlap = std::numeric_limits<float>::max();
	int minOverlapAxis = -1;

	for (int i = 0; i < 3; ++i) {
		float aabb1Max = (i == 0) ? m_max.x : (i == 1) ? m_max.y : m_max.z;
		float aabb1Min = (i == 0) ? m_min.x : (i == 1) ? m_min.y : m_min.z;
		float aabb2Max = (i == 0) ? arg_aabb1.m_max.x : (i == 1) ? arg_aabb1.m_max.y : arg_aabb1.m_max.z;
		float aabb2Min = (i == 0) ? arg_aabb1.m_min.x : (i == 1) ? arg_aabb1.m_min.y : arg_aabb1.m_min.z;

		if (aabb1Max < aabb2Min || aabb2Max < aabb1Min) {
			return std::nullopt; // 衝突していない
		}

		float overlap1 = aabb1Max - aabb2Min;
		float overlap2 = aabb2Max - aabb1Min;

		if (std::abs(overlap1) < std::abs(overlap2)) {
			if (std::abs(overlap1) < minOverlap) {
				minOverlap = std::abs(overlap1);
				minOverlapAxis = i;
				pushBack = (i == 0) ? KuroEngine::Vec3<float>{overlap1, 0.0f, 0.0f} :
					(i == 1) ? KuroEngine::Vec3<float>{0.0f, overlap1, 0.0f} :
					KuroEngine::Vec3<float>{ 0.0f, 0.0f, overlap1 };
			}
		}
		else {
			if (std::abs(overlap2) < minOverlap) {
				minOverlap = std::abs(overlap2);
				minOverlapAxis = i;
				pushBack = (i == 0) ? KuroEngine::Vec3<float>{-overlap2, 0.0f, 0.0f} :
					(i == 1) ? KuroEngine::Vec3<float>{0.0f, -overlap2, 0.0f} :
					KuroEngine::Vec3<float>{ 0.0f, 0.0f, -overlap2 };
			}
		}
	}

	return CollisionInfo{ pushBack };
}

void GoalPoint::Update(Player& arg_player)
{
	static const float HIT_RADIUS = 3.0f;
	static const float HIT_OFFSET = 5.0f;

	//プレイヤーとの当たり判定
	//if (!m_hitPlayer)
	m_hitPlayer = (arg_player.GetTransform().GetPosWorld().Distance(m_transform.GetPosWorld() + -m_transform.GetUp() * HIT_OFFSET * m_transform.GetScale().x) < HIT_RADIUS);
}

std::map<std::string, std::weak_ptr<KuroEngine::Model>>Appearance::s_models;

void Appearance::ModelsUpdate()
{
	//UVアニメーション
	for (auto& modelPtr : s_models)
	{
		auto model = modelPtr.second.lock();

		for (auto& mesh : model->m_meshes)
		{
			for (auto& vertex : mesh.mesh->vertices)
			{
				vertex.uv.y += 0.002f * TimeScaleMgr::s_inGame.GetTimeScale();
				//if (1.0f < vertex.uv.y)vertex.uv.y -= 1.0f;
			}
			mesh.mesh->Mapping();
		}
	}
}

Appearance::Appearance(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, StageParts* arg_parent)
	:StageParts(APPEARANCE, arg_model, arg_initTransform, arg_parent)
{
	if (!s_models.contains(arg_model.lock()->m_header.fileName))s_models[arg_model.lock()->m_header.fileName] = arg_model;

	m_collider.BuilCollisionMesh(arg_model, arg_initTransform);
}

void MoveScaffold::OnInit()
{
	m_isActive = false;
	m_isOldActive = false;
	m_prevOnPlayer = false;
	m_onPlayer = false;
	m_isStop = false;
	m_isOldStop = false;
	m_isOder = true;
	m_nowTranslationIndex = 0;
	m_nextTranslationIndex = 1;
	m_moveDir = KuroEngine::Vec3<float>(m_translationArray[m_nextTranslationIndex] - m_translationArray[m_nowTranslationIndex]).GetNormal();
	m_moveLength = KuroEngine::Vec3<float>(m_translationArray[m_nextTranslationIndex] - m_translationArray[m_nowTranslationIndex]).Length();
	m_nowMoveLength = 0;

	m_transform.SetPos(m_translationArray[0]);

	m_oldPos = m_translationArray[0];
	m_nowPos = m_translationArray[0];

	//当たり判定構築。
	m_collider.BuilCollisionMesh(m_model, m_transform);
}

void MoveScaffold::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	StageParts::Draw(arg_cam, arg_ligMgr);

	//移動経路がなかったら飛ばす。
	if (m_maxTranslation < 0) return;

	//移動経路を描画する。
	for (int index = 1; index <= m_maxTranslation; ++index) {
		KuroEngine::DrawFunc3D::DrawLine(arg_cam, m_translationArray[index - 1], m_translationArray[index], KuroEngine::Color(255, 255, 255, 255), 0.1f);
	}

}

void MoveScaffold::Update(Player& arg_player)
{

	//フラグを保存。
	m_prevOnPlayer = m_onPlayer;

	//SEの処理
	if ((!m_isOldActive && m_isActive) || (!m_isOldStop && m_isStop)) {
		SoundConfig::Instance()->Play(SoundConfig::SE_MOVE_SCAFFOLD_START);
	}
	if ((m_isOldActive && !m_isActive) || (m_isOldStop && !m_isStop)) {
		SoundConfig::Instance()->Play(SoundConfig::SE_MOVE_SCAFFOLD_STOP);
	}
	m_isOldActive = m_isActive;
	m_isOldStop = m_isStop;

	//有効化されていなかったら処理を飛ばす。
	if (!m_isActive) return;

	//一時停止中だったら処理を飛ばす。
	if (m_isStop) return;

	//ルートが設定されていない。
	assert(m_maxTranslation != 0);

	//座標を保存
	m_oldPos = m_nowPos;

	//移動した量を保存。
	float moveSpeed = MOVE_SPEED * TimeScaleMgr::s_inGame.GetTimeScale();
	m_nowMoveLength += moveSpeed;

	//移動した量が規定値を超えていたら、終わった判定。
	bool isFinish = false;
	if (m_moveLength < m_nowMoveLength) {

		isFinish = true;

		//オーバーした分だけ動かす。
		moveSpeed = m_moveLength - m_nowMoveLength;

	}

	//次の地点へ向かって動かす。
	m_transform.SetPos(m_transform.GetPos() + m_moveDir * moveSpeed);
	m_nowPos = m_transform.GetPos() + m_moveDir * moveSpeed;

	//プレイヤーも動かす。
	if (arg_player.GetOnGimmick()) {
		arg_player.SetGimmickVel(m_moveDir * moveSpeed);
	}

	//プレイヤーがジャンプ中だったら、ジャンプ地点も動かす。
	if (arg_player.GetIsJump()) {
		arg_player.SetJumpEndPos(arg_player.GetJumpEndPos() + m_moveDir * moveSpeed);
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
	m_collider.BuilCollisionMesh(m_model, m_transform);

}

void MoveScaffold::PushBack(KuroEngine::Vec3<float> arg_pushBack) {

	//押し戻す。
	m_nowMoveLength -= arg_pushBack.Length();
	m_transform.SetPos(m_transform.GetPos() - m_moveDir * arg_pushBack);

}

void MoveScaffold::BuildCollisionMesh() {
	m_collider.BuilCollisionMesh(m_model, m_transform);
}

void MoveScaffold::OnPlayer() {

	m_onPlayer = true;

	//乗ったトリガーだったら有効化する。
	if (!m_prevOnPlayer && m_onPlayer) {

		//一時停止中だったら
		if (m_isStop) {

			//現在進んでいる量を反転させる。
			m_nowMoveLength = m_moveLength - m_nowMoveLength;

			//移動方向も反転
			m_moveDir *= -1.0f;

			//フラグとインデックスも反転。
			std::swap(m_nowTranslationIndex, m_nextTranslationIndex);
			m_isOder = !m_isOder;

			m_isStop = false;

			//めり込んでいるのでちょっと移動させる。
			m_nowMoveLength += KuroEngine::Vec3<float>(m_moveDir * MOVE_SPEED).Length();
			m_transform.SetPos(m_transform.GetPos() + m_moveDir * MOVE_SPEED);

		}
		else {
			//普通に有効化
			Activate();
		}

	}

}

const std::string Lever::TURN_ON_ANIM_NAME = "turn_on";
const std::string Lever::TURN_OFF_ANIM_NAME = "turn_off";

Lever::Lever(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, StageParts* arg_parent, int arg_id, bool arg_initFlg)
	:StageParts(LEVER, arg_model, arg_initTransform, arg_parent), m_id(arg_id), m_initFlg(arg_initFlg)
{
	auto& anims = arg_model.lock()->m_skelton->animations;
	if (anims.find(TURN_OFF_ANIM_NAME) == anims.end() && anims.find(TURN_ON_ANIM_NAME) == anims.end())
	{

		KuroEngine::AppearMessageBox("Lever コンストラクタ失敗", "モデル(" + arg_model.lock()->m_header.fileName + ")に turn_on か turn_off のアニメーションが足りてないよ。");
		exit(1);
	}

	m_boxCollider.m_center = arg_initTransform.GetPosWorld();
	m_boxCollider.m_size = arg_initTransform.GetScale();
	m_modelAnimator = std::make_shared<KuroEngine::ModelAnimator>(arg_model);
}

void Lever::OnInit()
{
	m_flg = m_initFlg;
	m_isHit = false;
	m_isOldHit = false;
	m_modelAnimator->SetStartPosture(TURN_ON_ANIM_NAME);
}

void Lever::Update(Player& arg_player)
{
	//スイッチの状態が固定されている
	if (m_parentSwitch->IsFixed())return;

	//衝突フラグを保存。
	m_isOldHit = m_isHit;
	m_isHit = false;

	//植物繁殖光との当たり判定
	for (auto& lig : GrowPlantLight::GrowPlantLightArray())
	{
		if (DirectX::XM_PIDIV4 / 2.0f < acos(arg_player.GetTransform().GetUpWorld().Dot(m_transform.GetUpWorld()))) break;

		//内積でライトの法線とプレイヤーの上座標が離れてたら無効化する。
		if (lig->HitCheckWithBox(m_transform.GetPosWorld(), m_boxCollider.m_size))
		{
			//レバー操作でオンオフ切り替え
			m_isHit = true;
			break;
		}
	}

	//衝突のトリガー判定だったらフラグを切り替える。
	if (m_isHit && !m_isOldHit) {
		m_flg = !m_flg;

		//オン
		if (m_flg)
		{
			SoundConfig::Instance()->Play(SoundConfig::SE_LEVER_ON);
			m_modelAnimator->Play(TURN_ON_ANIM_NAME, false, false);
		}
		//オフ
		else
		{
			SoundConfig::Instance()->Play(SoundConfig::SE_LEVER_OFF);
			m_modelAnimator->Play(TURN_OFF_ANIM_NAME, false, false);
		}
	}

	m_modelAnimator->Update(TimeScaleMgr::s_inGame.GetTimeScale());
}

void Lever::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model.lock(),
		m_transform,
		KuroEngine::AlphaBlendMode_None,
		m_modelAnimator->GetBoneMatBuff());
}

void IvyZipLine::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{

	//蔦を描画
	const int INDEX = static_cast<int>(m_translationArray.size());
	for (int index = 0; index < INDEX - 1; ++index) {

		KuroEngine::DrawFunc3D::DrawLine(arg_cam, m_translationArray[index], m_translationArray[index + 1], KuroEngine::Color(0.35f, 0.91f, 0.55f, 1.0f), 1.0f);

	}

	for (auto index : m_translationArray) {
		KuroEngine::DrawFunc3D::DrawLine(arg_cam, index, index + KuroEngine::Vec3<float>(1.0f, 1.0f, 1.0f), KuroEngine::Color(0.35f, 0.91f, 0.55f, 1.0f), JUMP_SCALE);
	}

}

void IvyZipLine::CheckHit(bool arg_isHitStartPos) {


	m_isHitStartPos = arg_isHitStartPos;
	m_isActive = true;
	m_isReadyPlayer = false;

	//ジップライン移動に必要な変数を準備
	if (arg_isHitStartPos) {
		m_nowTranslationIndex = 0;
		m_nextTranslationIndex = 1;
		m_moveLength = (m_translationArray[1] - m_translationArray.front()).Length();
		m_moveDir = (m_translationArray[1] - m_translationArray.front()).GetNormal();
	}
	else {
		m_nowTranslationIndex = m_maxTranslation;
		m_nextTranslationIndex = m_maxTranslation - 1;
		m_moveLength = (m_translationArray[m_maxTranslation - 1] - m_translationArray.back()).Length();
		m_moveDir = (m_translationArray[m_maxTranslation - 1] - m_translationArray.back()).GetNormal();
	}
	m_nowMoveLength = 0;

}

KuroEngine::Vec3<float> IvyZipLine::GetPoint(bool arg_isEaseStart) {
	if (arg_isEaseStart) {
		if (m_isHitStartPos) {
			return m_translationArray.front();
		}
		else {
			return m_translationArray.back();
		}
	}
	else {
		if (m_isHitStartPos) {
			return m_translationArray.back();
		}
		else {
			return m_translationArray.front();
		}
	}
}

void IvyZipLine::Update(Player& arg_player)
{

	//ジップラインが有効化されている かつ プレイヤーの移動準備が出来ていたらプレイヤーを動かす。
	if (!m_isActive) return;
	if (!m_isReadyPlayer) return;

	//移動した量を保存。
	m_nowMoveLength += ZIPLINE_SPEED;

	//移動した量が規定値を超えていたら、終わった判定。
	float moveSpeed = ZIPLINE_SPEED;
	bool isFinish = false;
	if (m_moveLength < m_nowMoveLength) {

		isFinish = true;

		//オーバーした分だけ動かす。
		moveSpeed = m_moveLength - m_nowMoveLength;

	}

	//次の地点へ向かって動かす。
	arg_player.GetTransform().SetPos(arg_player.GetTransform().GetPos() + m_moveDir * moveSpeed);

	//プレイヤーも動かす。
	if (arg_player.GetOnGimmick()) {
		arg_player.SetGimmickVel(m_moveDir * moveSpeed);
	}

	//プレイヤーがジャンプ中だったら、ジャンプ地点も動かす。
	if (arg_player.GetIsJump()) {
		arg_player.SetJumpEndPos(arg_player.GetJumpEndPos() + m_moveDir * moveSpeed);
	}

	//いろいろと初期化して次向かう方向を決める。
	if (isFinish) {

		//正の方向に進むフラグだったら
		if (m_isHitStartPos) {

			//次のIndexへ
			m_nowTranslationIndex = m_nextTranslationIndex;
			++m_nextTranslationIndex;
			if (m_maxTranslation < m_nextTranslationIndex) {

				//終わっていたら
				m_nextTranslationIndex = m_maxTranslation;
				m_isActive = false;
				arg_player.m_collision.FinishGimmickMove();
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
				m_isActive = false;
				arg_player.m_collision.FinishGimmickMove();

			}

		}

		//移動する方向と量を求める。
		m_moveDir = KuroEngine::Vec3<float>(m_translationArray[m_nextTranslationIndex] - m_translationArray[m_nowTranslationIndex]).GetNormal();
		m_moveLength = KuroEngine::Vec3<float>(m_translationArray[m_nextTranslationIndex] - m_translationArray[m_nowTranslationIndex]).Length();
		m_nowMoveLength = 0;

	}

}

IvyBlock::IvyBlock(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, StageParts* arg_parent, KuroEngine::Vec3<float> arg_leftTopFront, KuroEngine::Vec3<float> arg_rightBottomBack)
	:StageParts(IVY_BLOCK, arg_model, arg_initTransform, arg_parent), m_leftTopFront(arg_leftTopFront), m_rightBottomBack(arg_rightBottomBack) 
{
	m_collider.BuilCollisionMesh(arg_model, arg_initTransform);
	OnInit();
	m_nonExistModel = std::shared_ptr<KuroEngine::Model>(new KuroEngine::Model(*arg_model.lock()));
	m_nonExistMaterial = std::shared_ptr<KuroEngine::Material>(new KuroEngine::Material(*arg_model.lock()->m_meshes[0].material));
	m_nonExistMaterial->texBuff[KuroEngine::MATERIAL_TEX_TYPE::COLOR_TEX] = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/invisibleIvyBlock.png");
	for (auto& mesh : m_nonExistModel->m_meshes)
	{
		mesh.material = m_nonExistMaterial;
	}
}

void IvyBlock::Update(Player& arg_player)
{

	m_prevOnPlayer = m_onPlayer;

	//イージングタイマーを更新。
	m_easingTimer = std::clamp(m_easingTimer + EASING_TIMER * TimeScaleMgr::s_inGame.GetTimeScale(), 0.0f, 1.0f);


	m_collider.BuilCollisionMesh(m_model, m_transform);

	//出現中だったら。
	if (m_isAppear) {

		float easingValue = m_easingTimer;
		easingValue = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Back, easingValue, 0.0f, 1.0f);

		//スケーリング
		auto scale = HIT_SCALE_MIN * easingValue;
		m_transform.SetScale(scale);

		m_nonExistDrawParam.m_alpha = easingValue;
	}
	else {

		float easingValue = m_easingTimer;
		easingValue = KuroEngine::Math::Ease(KuroEngine::In, KuroEngine::Back, easingValue, 0.0f, 1.0f);

		//スケーリング
		auto scale = HIT_SCALE_MIN - HIT_SCALE_MIN * easingValue;
		m_transform.SetScale(scale);

		m_nonExistDrawParam.m_alpha = easingValue;
	}

}

void IvyBlock::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model.lock(),
		m_transform);
	
	KuroEngine::Transform invisibleTrans = m_transform;
	invisibleTrans.SetScale(HIT_SCALE_MIN);
	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_nonExistModel,
		invisibleTrans,
		m_nonExistDrawParam,
		KuroEngine::AlphaBlendMode_Trans,
		nullptr,
		1);
}

void IvyBlock::Appear()
{

	m_isAppear = true;
	m_easingTimer = 0;

}

void IvyBlock::Disappear()
{

	m_onPlayer = false;
	m_isAppear = false;
	m_easingTimer = 0;

}


