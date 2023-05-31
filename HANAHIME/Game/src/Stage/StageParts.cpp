#include"StageParts.h"
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
#include"GateManager.h"
#include"CheckPointHitFlag.h"
#include"../OperationConfig.h"
#include"../System/SaveDataManager.h"
#include"StageManager.h"

std::array<std::string, StageParts::STAGE_PARTS_TYPE::NUM>StageParts::s_typeKeyOnJson =
{
	"Terrian","Start","Goal","Appearance","MoveScaffold","Lever","Ivy_Zipline","IvyBlock","SplatoonFence","Gate","CheckPoint","StarCoin","BackGround","Kinoko",
	"MiniBug","DossunRing","Battery",
};

const std::string &StageParts::GetTypeKeyOnJson(STAGE_PARTS_TYPE arg_type)
{
	return s_typeKeyOnJson[arg_type];
}

void StageParts::Init()
{
	//m_transform.SetPos(m_initializedTransform.GetPos());
	//m_transform.SetScale(m_initializedTransform.GetScale());
	//m_transform.SetRotate(m_initializedTransform.GetRotate());
	m_transform = m_initializedTransform;
	OnInit();
}

void StageParts::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
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
		auto &mesh = arg_model.lock()->m_meshes[meshIdx].mesh;

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
		for (auto &index : m_collisionMesh[meshIdx]) {

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
		DirectX::XMMATRIX targetWorldMat = arg_transform.GetMatWorld();
		for (auto &index : m_collisionMesh[meshIdx]) {
			//頂点を変換
			index.m_p0.pos = KuroEngine::Math::TransformVec3(index.m_p0.pos, targetWorldMat);
			index.m_p1.pos = KuroEngine::Math::TransformVec3(index.m_p1.pos, targetWorldMat);
			index.m_p2.pos = KuroEngine::Math::TransformVec3(index.m_p2.pos, targetWorldMat);
			//法線を回転行列分だけ変換
			index.m_p0.normal = KuroEngine::Math::TransformVec3(index.m_p0.normal, arg_transform.GetRotateWorld());
			index.m_p0.normal.Normalize();
			index.m_p1.normal = KuroEngine::Math::TransformVec3(index.m_p1.normal, arg_transform.GetRotateWorld());
			index.m_p1.normal.Normalize();
			index.m_p2.normal = KuroEngine::Math::TransformVec3(index.m_p2.normal, arg_transform.GetRotateWorld());
			index.m_p2.normal.Normalize();
		}
	}

	//上記で作った当たり判定用ポリゴンを元に、当たり判定用のBOXを作る。
	m_aabb.clear();
	for (auto &stage : m_collisionMesh) {

		//格納するデータを保存。
		m_aabb.emplace_back();

		for (auto &index : stage) {

			//当たり判定BOXを入れるデータ
			m_aabb.back().emplace_back(CreateCubeFromPolygon(index.m_p0.pos, index.m_p1.pos, index.m_p2.pos, index.m_p0.normal));

		}

	}

}

AABB TerrianMeshCollider::CreateCubeFromPolygon(const KuroEngine::Vec3<float> &arg_v1, const KuroEngine::Vec3<float> &arg_v2, const KuroEngine::Vec3<float> &arg_v3, const KuroEngine::Vec3<float> &arg_normal) {

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

	for (auto &index : cubeVertices) {
		aabb.m_min.x = std::min(aabb.m_min.x, index.x);
		aabb.m_min.y = std::min(aabb.m_min.y, index.y);
		aabb.m_min.z = std::min(aabb.m_min.z, index.z);
		aabb.m_max.x = std::max(aabb.m_max.x, index.x);
		aabb.m_max.y = std::max(aabb.m_max.y, index.y);
		aabb.m_max.z = std::max(aabb.m_max.z, index.z);
	}

	return aabb;
}

std::optional<AABB::CollisionInfo> AABB::CheckAABBCollision(const AABB &arg_aabb1) {
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

GoalPoint::GoalPoint(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform)
	:StageParts(GOAL_POINT, arg_model, arg_initTransform)
{
	m_saplingModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/stage/", "Goal.glb");
	m_woodModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/stage/", "Goal_Wood.glb");
}

void GoalPoint::Update(Player &arg_player)
{
	using namespace KuroEngine;

	const float HIT_RADIUS = 10.0f;

	//プレイヤーとの当たり判定
	float dist = arg_player.GetTransform().GetPosWorld().Distance(m_transform.GetPosWorld() + (-m_transform.GetUpWorld() * HIT_RADIUS));
	if (!m_hitPlayer)
	{
		m_hitPlayer = (dist < HIT_RADIUS);
	}

	if (m_isGrowUp)
	{
		m_growUpTimer.UpdateTimer(TimeScaleMgr::s_inGame.GetTimeScale());
		m_transform.SetScale(Math::Ease(Out, Elastic, m_growUpTimer.GetTimeRate(), 0.0f, 1.0f));
	}
}

void GoalPoint::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	using namespace KuroEngine;

	static const Vec3<float> MODEL_OFFSET = m_transform.GetUpWorld() * -20.0f;

	Transform drawTransform;
	drawTransform.SetPos(m_transform.GetPosWorld() + m_offset.GetPosWorld() + MODEL_OFFSET);
	drawTransform.SetRotate(m_transform.GetRotate());
	drawTransform.SetScale(m_transform.GetScaleWorld() + m_offset.GetScaleWorld());

	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model.lock(),
		drawTransform);
}

std::map<std::string, std::weak_ptr<KuroEngine::Model>>Appearance::s_models;

void Appearance::ModelsUvUpdate(float arg_timeScale)
{
	//UVアニメーション
	for (auto &modelPtr : s_models)
	{
		auto model = modelPtr.second.lock();

		for (auto &mesh : model->m_meshes)
		{
			for (auto &vertex : mesh.mesh->vertices)
			{
				vertex.uv.y += 0.002f * TimeScaleMgr::s_inGame.GetTimeScale();
				//if (1.0f < vertex.uv.y)vertex.uv.y -= 1.0f;
			}
			mesh.mesh->Mapping();
		}
	}
}

Appearance::Appearance(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, std::weak_ptr<KuroEngine::Model>arg_collisionModel)
	:StageParts(APPEARANCE, arg_model, arg_initTransform)
{
	if (!s_models.contains(arg_model.lock()->m_header.fileName))s_models[arg_model.lock()->m_header.fileName] = arg_model;

	m_collider.BuilCollisionMesh(arg_collisionModel, arg_initTransform);
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

	m_moveStartTimer = KuroEngine::Timer(MOVE_START_TIMER);

	m_transform.SetPos(m_translationArray[0]);

	m_oldPos = m_translationArray[0];
	m_nowPos = m_translationArray[0];

	//当たり判定構築。
	ReBuildCollisionMesh();
}

void MoveScaffold::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{

	StageParts::Draw(arg_cam, arg_ligMgr);

	//移動経路がなかったら飛ばす。
	if (m_maxTranslation < 0) return;

	//移動経路を描画する。
	for (int index = 1; index <= m_maxTranslation; ++index) {
		KuroEngine::DrawFunc3D::DrawLine(arg_cam, m_translationArray[index - 1], m_translationArray[index], KuroEngine::Color(31, 247, 205, 255), 2.0f);
	}

}

void MoveScaffold::Update(Player &arg_player)
{

	m_moveAmount = {};

	//フラグを保存。
	m_prevOnPlayer = m_onPlayer;

	if (!m_onPlayer) {
		m_moveStartTimer.Reset();
	}

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
	m_moveAmount = m_moveDir * moveSpeed;
	m_transform.SetPos(m_transform.GetPos() + m_moveAmount);
	m_nowPos = m_transform.GetPos() + m_moveAmount;

	//プレイヤーも動かす。
	if (arg_player.GetOnGimmick()) {
		arg_player.SetGimmickVel(m_moveAmount);
	}

	//プレイヤーがジャンプ中だったら、ジャンプ地点も動かす。
	if (arg_player.GetIsJump()) {
		arg_player.SetJumpEndPos(arg_player.GetJumpEndPos() + m_moveAmount);
	}

	//いろいろと初期化して次向かう方向を決める。
	if (isFinish) {

		//隙間ができないように押し戻す。
		m_transform.SetPos(m_translationArray[m_nextTranslationIndex]);
		m_nowPos = m_translationArray[m_nextTranslationIndex];

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

		m_moveStartTimer.Reset();


	}

	//当たり判定を再構築。
	ReBuildCollisionMesh();
}

void MoveScaffold::PushBack(KuroEngine::Vec3<float> arg_pushBack) {

	//押し戻す。
	m_nowMoveLength -= arg_pushBack.Length();
	m_transform.SetPos(m_transform.GetPos() - m_moveDir * arg_pushBack);

}

void MoveScaffold::ReBuildCollisionMesh() {
	m_collider.BuilCollisionMesh(m_collisionModel, m_transform);
}

void MoveScaffold::OnPlayer() {

	m_onPlayer = true;

	//乗ったトリガーの時にタイマーを起動。
	if (m_onPlayer && !m_prevOnPlayer) {
		m_moveStartTimer.UpdateTimer();
	}

	//そしてタイマーが動いていたらタイマーを動かす。(ゴールしてても乗ったままでずっと動き続けるのを避けるために、乗った瞬間しかタイマーを動かせないようにしている。)
	if (0.0f < m_moveStartTimer.GetTimeRate()) {
		m_moveStartTimer.UpdateTimer();
	}

	if (m_moveStartTimer.IsTimeUp()) {

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

Lever::Lever(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, int arg_id, bool arg_initFlg)
	:StageParts(LEVER, arg_model, arg_initTransform), m_id(arg_id), m_initFlg(arg_initFlg)
{
	auto &anims = arg_model.lock()->m_skelton->animations;
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

void Lever::Update(Player &arg_player)
{
	//スイッチの状態が固定されている
	if (m_parentSwitch->IsFixed())return;

	//衝突フラグを保存。
	m_isOldHit = m_isHit;
	m_isHit = false;

	//植物繁殖光との当たり判定
	for (auto &lig : GrowPlantLight::GrowPlantLightArray())
	{
		//内積でライトの法線とプレイヤーの上座標が離れてたら無効化する。
		if (DirectX::XM_PIDIV4 < acosf(arg_player.GetTransform().GetUpWorld().Dot(m_transform.GetUpWorld()))) continue;

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

void Lever::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model.lock(),
		m_transform,
		KuroEngine::AlphaBlendMode_None,
		m_modelAnimator->GetBoneMatBuff());

#ifdef _DEBUG

	KuroEngine::DrawFunc3D::DrawLine(arg_cam, m_transform.GetPosWorld(), m_transform.GetPosWorld() + m_transform.GetUpWorld() * 10.0f, KuroEngine::Color(0.95f, 0.00f, 0.51f, 1.0f), 1.0f);

#endif // _DEBUG

}

void IvyZipLine::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
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

void IvyZipLine::Update(Player &arg_player)
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

IvyBlock::IvyBlock(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, KuroEngine::Vec3<float> arg_leftTopFront, KuroEngine::Vec3<float> arg_rightBottomBack, std::weak_ptr<KuroEngine::Model>arg_collisionModel)
	:StageParts(IVY_BLOCK, arg_model, arg_initTransform), m_leftTopFront(arg_leftTopFront), m_rightBottomBack(arg_rightBottomBack)
{
	m_nonExistModel = std::shared_ptr<KuroEngine::Model>(new KuroEngine::Model(*arg_model.lock()));
	m_nonExistMaterial = std::shared_ptr<KuroEngine::Material>(new KuroEngine::Material(*arg_model.lock()->m_meshes[0].material));
	m_invisibleIvyBlockTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/invisibleIvyBlock.png");
	m_invisibleIvyBlockReadyTex = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/invisibleIvyBlock_Ready.png");
	m_nonExistMaterial->texBuff[KuroEngine::MATERIAL_TEX_TYPE::COLOR_TEX] = m_invisibleIvyBlockReadyTex;
	for (auto &mesh : m_nonExistModel->m_meshes)
	{
		mesh.material = m_nonExistMaterial;
	}
	m_collisionModel = arg_collisionModel;
	OnInit();
}

void IvyBlock::Update(Player &arg_player)
{

	m_prevOnPlayer = m_onPlayer;

	//イージングタイマーを更新。
	m_easingTimer = std::clamp(m_easingTimer + EASING_TIMER * TimeScaleMgr::s_inGame.GetTimeScale(), 0.0f, 1.0f);

	if (m_easingTimer < 1.0f) {

		ReuilCollisionMesh();

	}

	//出現中だったら。
	if (m_isAppear) {

		float easingValue = m_easingTimer;
		easingValue = KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Back, easingValue, 0.0f, 1.0f);

		//スケーリング
		auto scale = SCALE_DEF * easingValue;
		m_transform.SetScale(scale);

		m_nonExistDrawParam.m_alpha = easingValue;
	}
	else {

		float easingValue = m_easingTimer;
		easingValue = KuroEngine::Math::Ease(KuroEngine::In, KuroEngine::Back, easingValue, 0.0f, 1.0f);

		//スケーリング
		auto scale = SCALE_DEF - SCALE_DEF * easingValue;
		m_transform.SetScale(scale);

		m_nonExistDrawParam.m_alpha = easingValue;
	}

}

void IvyBlock::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model.lock(),
		m_transform);

	if (m_onPlayer) {
		m_nonExistMaterial->texBuff[KuroEngine::MATERIAL_TEX_TYPE::COLOR_TEX] = m_invisibleIvyBlockReadyTex;
	}
	else {
		m_nonExistMaterial->texBuff[KuroEngine::MATERIAL_TEX_TYPE::COLOR_TEX] = m_invisibleIvyBlockTex;
	}
	for (auto &mesh : m_nonExistModel->m_meshes)
	{
		mesh.material = m_nonExistMaterial;
	}

	KuroEngine::Transform invisibleTrans = m_transform;
	invisibleTrans.SetScale(SCALE_DEF);
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

void SplatoonFence::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	StageParts::Draw(arg_cam, arg_ligMgr);
}

void Terrian::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{

	IndividualDrawParameter param = IndividualDrawParameter::GetDefault();

	BasicDraw::Instance()->Draw_Stage(
		arg_cam,
		arg_ligMgr,
		m_model.lock(),
		m_transform,
		IndividualDrawParameter::GetDefault());

}

std::array<std::shared_ptr<KuroEngine::TextureBuffer>, Gate::GATE_TEX_ARRAY_SIZE>Gate::s_texArray;

Gate::Gate(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, int arg_id, int arg_destStageNum, int arg_destGateId, std::shared_ptr<GuideInsect::CheckPointData>checkPointData)
	:StageParts(GATE, arg_model, arg_initTransform), m_id(arg_id), m_destStageNum(arg_destStageNum), m_destGateId(arg_destGateId), m_guideData(checkPointData)
{
	//テクスチャ読み込み
	if (!s_texArray[0])
	{
		KuroEngine::D3D12App::Instance()->GenerateTextureBuffer(
			s_texArray.data(), "resource/user/tex/stage/gate.png", GATE_TEX_ARRAY_SIZE, { GATE_TEX_ARRAY_SIZE,1 });
	}
	m_guideData->m_pos = m_transform.GetPosWorld();

	static const float TEX_ANIM_INTERVAL = 4.0f;
	m_animTimer.Reset(TEX_ANIM_INTERVAL);
}

void Gate::Update(Player &arg_player)
{
	using namespace KuroEngine;
	static const float HIT_RADIUS = 8.0f;
	static const float HIT_SPHERE_OFFSET_Y = -4.0f;
	static const float EFFECT_INTERVAL = 60.0f;
	static const float EFFECT_SCALE_OFFSET = 0.1f;

	if (IsExit())return;

	const float timeScale = TimeScaleMgr::s_inGame.GetTimeScale();

	float dist = (m_transform.GetPosWorld() + m_transform.GetUpWorld() * HIT_SPHERE_OFFSET_Y).Distance(arg_player.GetNowPos());
	bool enter = dist < HIT_RADIUS *m_transform.GetScaleWorld().x;
	GateManager::Instance()->SetEnter(enter, m_destStageNum, m_destGateId);

	//テクスチャアニメーション
	if (m_animTimer.UpdateTimer(timeScale))
	{
		//ループ
		if (GATE_TEX_ARRAY_SIZE <= ++m_texIdx)m_texIdx = 0;
		m_animTimer.Reset();
	}

	m_effectSinCurveAngle += Angle::ROUND() / EFFECT_INTERVAL;
	m_effectScale = 1.0f + powf(sin(m_effectSinCurveAngle), 2.0f) * EFFECT_SCALE_OFFSET;
}

void Gate::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	static const float SIZE_HALF = 10.0f;
	static const float DRAW_POS_OFFSET = -5.0f;

	static const KuroEngine::Vec2<float>PLANE_SIZE = { SIZE_HALF * 2.0f,SIZE_HALF * 2.0f };

	if (IsExit())return;

	BasicDraw::Instance()->DrawBillBoard(arg_cam,
		m_transform.GetPosWorld() + m_transform.GetUpWorld() * DRAW_POS_OFFSET,
		PLANE_SIZE * m_effectScale,
		PLANE_SIZE * m_effectScale,
		s_texArray[m_texIdx]);
}

bool Gate::CheckID(int arg_id)
{
	return m_id == arg_id;
}

std::shared_ptr<CheckPointUI>CheckPoint::s_ui;
KuroEngine::Transform CheckPoint::s_latestVisitTransform;
bool CheckPoint::s_visit = false;

CheckPoint::CheckPoint(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, int arg_order, std::shared_ptr<GuideInsect::CheckPointData>checkPointData)
	:StageParts(CHECK_POINT, arg_model, arg_initTransform), m_order(arg_order), m_guideData(checkPointData), m_fireWork(GPUParticleRender::Instance()->GetStackBuffer())
{
	//UI未生成なら生成
	if (!s_ui)s_ui = std::make_shared<CheckPointUI>();
	KuroEngine::Vec3<float>vec(m_transform.GetUpWorld());

	if (1.0f <= abs(vec.x))
	{
		vec.x *= 10.0f;
	}
	if (1.0f <= abs(vec.y))
	{
		vec.y *= 10.0f;
	}
	if (1.0f <= abs(vec.z))
	{
		vec.z *= 10.0f;
	}

	m_bloomingFlowerModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/Stage/", "CheckPoint_Unlocked.glb");
	m_guideData->m_pos = m_transform.GetPosWorld() + vec;
}

void CheckPoint::OnInit()
{
}

void CheckPoint::Update(Player &arg_player)
{
	static const float CHECK_POINT_RADIUS = 13.0f;

	m_fireWork.Update();

	//起動済なら特に何もしない
	if (m_touched)return;

	//円の衝突
	bool isHit = arg_player.GetTransform().GetPosWorld().DistanceSq(m_transform.GetPosWorld()) < (CHECK_POINT_RADIUS * CHECK_POINT_RADIUS);
	//衝突した瞬間
	if (!m_touched && isHit)
	{
		m_guideData->m_isHitFlag = true;
		//UI出現
		s_ui->Start();
		//最後に訪れたチェックポイントのトランスフォームを記録
		s_latestVisitTransform = m_transform;
		s_visit = true;
		CheckPointHitFlag::Instance()->m_isHitCheckPointTrigger = true;
		SaveDataManager::Instance()->SaveCheckPointOrder(m_order);

		//SE再生
		SoundConfig::Instance()->Play(SoundConfig::SE_UNLOCK_CHECK_POINT);

		m_fireWork.Init(m_transform.GetPosWorld());
	}

	m_touched = isHit;
	m_guideData->m_isHitFlag = isHit;
}

void CheckPoint::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	if (m_touched) {
		BasicDraw::Instance()->Draw_NoGrass(
			arg_cam,
			arg_ligMgr,
			m_bloomingFlowerModel,
			m_transform,
			IndividualDrawParameter::GetDefault());
	}
	else {
		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			m_model.lock(),
			m_transform,
			IndividualDrawParameter::GetDefault());
	}
}

int StarCoin::s_id = 0;
int StarCoin::GET_SUM = 0;

StarCoin::StarCoin(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform)
	:StageParts(CHECK_POINT, arg_model, arg_initTransform), m_id(s_id++)
{
	m_getModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/stage/", "StarCoin_Get.glb");
}

void StarCoin::OnInit()
{
	m_touched = false;
	m_get = SaveDataManager::Instance()->IsGetStarCoin(StageManager::Instance()->GetNowStageIdx(), m_id);
	m_basePos = m_transform.GetPos();
	m_angle = 0.0f;
	m_isFinishEffectFlag = false;
	m_scale = m_transform.GetScale().x;
}

void StarCoin::Update(Player &arg_player)
{
	const float STAR_COIN_RADIUS = 10.0f;
	const float STAR_COIN_OFFSET_Y = -5.0f;

	//拾われたなら何もしない
	if (m_isFinishEffectFlag)return;

	//プレイヤーとの当たり判定
	KuroEngine::Vec3<float>playerPos = arg_player.GetTransform().GetPosWorld();
	KuroEngine::Vec3<float>myPos = m_transform.GetPosWorld() + m_transform.GetUpWorld() * STAR_COIN_OFFSET_Y;
	bool isHit = playerPos.Distance(myPos) < STAR_COIN_RADIUS;

	//衝突した瞬間
	if (!m_touched && isHit)
	{
		//初めて入手
		if (!m_get)
		{
			GET_SUM++;
			SaveDataManager::Instance()->SaveStarCoin(StageManager::Instance()->GetNowStageIdx(), m_id);
		}
		//SE再生
		SoundConfig::Instance()->Play(SoundConfig::SE_GET_STAR_COIN);

		m_basePos = m_transform.GetPos();
		m_baseAngle = m_angle;
		m_timer.Reset(60 * 1.5f);
		m_disappearTimer.Reset(30);
		m_touched = true;
	}

	//待機中
	if (!m_touched)
	{
		m_angle += 3.0f;

		KuroEngine::Vec3<float>pos(m_basePos);
		pos.y = m_basePos.y + sinf(KuroEngine::Angle::ConvertToRadian(m_angle)) * 2.0f;
		m_transform.SetPos(pos);
	}
	//触れた
	else
	{
		m_angle = m_baseAngle + KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Cubic, m_timer.GetTimeRate(), 0.0f, 360.0f * 5);

		DirectX::XMVECTOR axis = { 0.0f,1.0f,0.0f,0.0f };
		KuroEngine::Quaternion rotation = DirectX::XMQuaternionRotationAxis(axis, KuroEngine::Angle::ConvertToRadian(m_angle));

		KuroEngine::Vec3<float>pos(m_basePos);
		pos.y = m_basePos.y + KuroEngine::Math::Ease(KuroEngine::Out, KuroEngine::Cubic, m_timer.GetTimeRate(), 0.0f, 10.0f);
		m_transform.SetPos(pos);
		m_transform.SetRotate(rotation);

		if (m_timer.UpdateTimer())
		{
			//消滅処理
			m_transform.SetScale(
				m_scale - KuroEngine::Math::Ease(KuroEngine::In, KuroEngine::Back, m_disappearTimer.GetTimeRate(), 0.0f, m_scale)
			);
			//消滅したら描画を切る
			if (m_disappearTimer.UpdateTimer())
			{
				m_isFinishEffectFlag = true;
			}
		}
	}


	//m_touched = isHit;
	if (isHit)m_get = true;
}

void StarCoin::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{
	//拾われたら描画しない
	if (m_isFinishEffectFlag)return;

	//入手したことがないなら通常描画
	if (!m_get)
	{
		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			m_model.lock(),
			m_transform);
		StageParts::Draw(arg_cam, arg_ligMgr);
	}
	//入手したことがあるなら半透明
	else
	{
		auto drawTransform = m_transform;
		drawTransform.SetRotate(XMQuaternionIdentity());

		static IndividualDrawParameter halfAlphaParam;
		halfAlphaParam.m_alpha = 0.5f;
		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			m_getModel,
			m_transform,
			halfAlphaParam);
	}
}

BackGround::BackGround(std::weak_ptr<KuroEngine::Model> arg_model, KuroEngine::Transform arg_initTransform, bool arg_shadowShader)
	:StageParts(BACKGROUND, arg_model, arg_initTransform), m_shadowShader(arg_shadowShader)
{
	m_backGroundObjectTexBuffer = KuroEngine::D3D12App::Instance()->GenerateTextureBuffer("resource/user/tex/BackGroundObjectTexture.png");
}

void BackGround::Draw(KuroEngine::Camera &arg_cam, KuroEngine::LightManager &arg_ligMgr)
{

	if (m_shadowShader)
	{
		BasicDraw::Instance()->Draw_BackGround(
			m_backGroundObjectTexBuffer,
			arg_cam,
			arg_ligMgr,
			m_model,
			m_transform,
			IndividualDrawParameter::GetDefault());
	}
	else
	{
		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			m_model,
			m_transform,
			IndividualDrawParameter::GetDefault());
	}
}

std::array<std::shared_ptr<KuroEngine::Model>,Kinoko::KINOKO_PATTERN_NUM>Kinoko::s_kinokoModel;
std::array<std::shared_ptr<KuroEngine::Model>,Kinoko::KINOKO_PATTERN_NUM>Kinoko::s_brightKinokoModel;

void Kinoko::OnInit()
{
	m_isHit = false;
	m_patternIdx = std::clamp(KuroEngine::GetRand(KINOKO_PATTERN_NUM), 0, KINOKO_PATTERN_NUM - 1);
	m_expandTimer.Reset(0.0f);
}

Kinoko::Kinoko(KuroEngine::Transform arg_initTransform)
	:StageParts(KINOKO, s_kinokoModel[0], arg_initTransform)
{
	using namespace KuroEngine;
	if (!s_kinokoModel[0])
	{
		std::string dir = "resource/user/model/stage/kinoko/";

		for (int kinokoIdx = 0; kinokoIdx < KINOKO_PATTERN_NUM; ++kinokoIdx)
		{
			s_kinokoModel[kinokoIdx] = Importer::Instance()->LoadModel(dir, std::to_string(kinokoIdx) + ".glb");
			s_kinokoModel[kinokoIdx]->m_meshes[0].material->constData.lambert.emissive *= 0.2f;
			s_kinokoModel[kinokoIdx]->m_meshes[0].material->Mapping();
			s_brightKinokoModel[kinokoIdx] = Importer::Instance()->LoadModel(dir, "bright_" + std::to_string(kinokoIdx) + ".glb");
			s_brightKinokoModel[kinokoIdx]->m_meshes[0].material->constData.lambert.emissive = { 0.52f,0.98f,1.0f };
			s_brightKinokoModel[kinokoIdx]->m_meshes[0].material->constData.lambert.emissive *= 0.4f;
			s_brightKinokoModel[kinokoIdx]->m_meshes[0].material->Mapping();
		}
	}
	OnInit();
}

void Kinoko::Update(Player& arg_player)
{
	using namespace KuroEngine;

	m_expandTimer.UpdateTimer(TimeScaleMgr::s_inGame.GetTimeScale());

	m_transform.SetScale(KuroEngine::Math::Ease(Out, Elastic, m_expandTimer.GetTimeRate(), m_smallScale, 1.0f));

	static const float HIT_RADIUS = 8.0f;
	bool hit = arg_player.GetTransform().GetPosWorld().DistanceSq(m_transform.GetPosWorld()) < HIT_RADIUS * HIT_RADIUS;
	if (!oldHit && hit)
	{
		m_expandTimer.Reset(30.0f);
		m_smallScale = 0.6f;
		if (!m_isHit)
		{
			m_isHit = true;
			m_smallScale = 0.0f;
		}
	}
	oldHit = hit;
}

void Kinoko::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr)
{
	auto model = m_isHit ? s_brightKinokoModel[m_patternIdx] : s_kinokoModel[m_patternIdx];

	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		model,
		m_transform);
}
