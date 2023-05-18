#include "PlayerCollision.h"
#include "Player.h"
#include"Render/RenderObject/Camera.h"
#include"../OperationConfig.h"
#include"FrameWork/Importer.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"../Graphics/BasicDraw.h"
#include"../Stage/Stage.h"
#include"../Graphics/BasicDrawParameters.h"
#include"../../../../src/engine/ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"FrameWork/UsersInput.h"
#include"../SoundConfig.h"
#include"PlayerCollision.h"
#include"CollisionDetectionOfRayAndMesh.h"

bool PlayerCollision::HitCheckAndPushBack(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo)
{
	/*
	arg_from … 移動前の座標
	arg_to … 移動後の座標
	arg_terrianArray … 地形の配列
	arg_terrianNormal … 当たった地形のメッシュの法線、格納先
	*/

	//CastRayに渡す引数
	PlayerCollision::CastRayArgument castRayArgument;
	castRayArgument.m_stageType = StageParts::TERRIAN;
	for (auto& index : castRayArgument.m_checkDeathCounter) {
		index = 0;
	}
	for (auto& index : castRayArgument.m_checkHitAround) {
		index = false;
	}

	//地面との当たり判定
	CheckHitGround(arg_from, arg_newPos, arg_nowStage, arg_hitInfo, castRayArgument);

	//死んだか(挟まっているか)どうかを判定
	CheckDeath(arg_from, arg_newPos, arg_nowStage, arg_hitInfo, castRayArgument);

	//周囲の壁との当たり判定
	CheckHitAround(arg_from, arg_newPos, arg_nowStage, arg_hitInfo, castRayArgument);

	//ジップラインとの当たり判定
	CheckZipline(arg_newPos, arg_nowStage);

	//死んでいたら処理を飛ばす。
	bool isDeath = false;
	isDeath = false;
	isDeath |= (castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::RIGHT)] && castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::LEFT)]);
	isDeath |= (castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::TOP)] && castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::BOTTOM)]);
	isDeath |= (castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::FRONT)] && castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::BEHIND)]);
	if (isDeath) {
		++m_refPlayer->m_deathTimer;
	}
	else {
		m_refPlayer->m_deathTimer = 0;
	}

	//死んでいたら
	if (m_refPlayer->DEATH_TIMER < m_refPlayer->m_deathTimer) {
		m_refPlayer->m_isDeath = true;
		return false;

	}

	return true;
}

void PlayerCollision::CheckDeath(const KuroEngine::Vec3<float> arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, PlayerCollision::CastRayArgument& arg_castRayArgment)
{

	const float DEATH_LENGTH = 1.0f;

	//全方向にレイを飛ばして死亡チェック。
	CheckHitAllObject(&PlayerCollision::CheckHitDeath_Around, arg_newPos, arg_from, arg_castRayArgment, arg_nowStage);

}

void PlayerCollision::CheckHitAround(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, PlayerCollision::CastRayArgument& arg_castRayArgment) {


	//プレイヤーが沈んでいるときは、上方向にレイを飛ばして上に柵がないかをチェックする。
	m_refPlayer->m_canOldUnderGroundRelease = m_refPlayer->m_canUnderGroundRelease;
	m_refPlayer->m_canUnderGroundRelease = true;
	//フェンスとの当たり判定
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//動く足場でない
		if (terrian->GetType() != StageParts::SPLATOON_FENCE)continue;

		//距離によってカリング
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 2.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

		//動く足場としてキャスト
		auto ivyBlock = dynamic_pointer_cast<SplatoonFence>(terrian);

		//モデル情報取得
		auto model = terrian->GetModel();

		//メッシュを走査
		for (auto& modelMesh : model.lock()->m_meshes)
		{

			//CastRayに渡す引数を更新。
			auto hitmesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//判定↓============================================

			//当たり判定を実行
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(m_refPlayer->GetNowPos(), m_refPlayer->GetTransform().GetUp(), hitmesh);

			if (output.m_isHit && fabs(output.m_distance) < m_refPlayer->GetTransform().GetScale().x * 2.0f) {

				m_refPlayer->m_canUnderGroundRelease = false;

				//プレイヤーが地中に潜っていなかったら鎮める。
				//if (!m_refPlayer->GetIsUnderGround()) {
				m_refPlayer->m_isUnderGround = true;
				m_refPlayer->m_underGroundEaseTimer = 1.0f;
				//}

			}

			//=================================================
		}
	}



	//四方にレイを飛ばして押し戻し
	CheckHitAllObject(&PlayerCollision::CheckHitAround_Around, arg_newPos, arg_from, arg_castRayArgment, arg_nowStage);

	//周囲の衝突点があったら、それの最短距離を求めてジャンプ先を決める。
	int impactPointSize = static_cast<int>(arg_castRayArgment.m_impactPoint.size());
	if (0 < impactPointSize) {

		//全ての衝突点について崖を超えていないかをチェックする。
		for (auto& index : arg_castRayArgment.m_impactPoint) {

			//崖チェック
			CheckCliff(index, arg_nowStage);

			//本当に飛べるのかチェック
			CheckCanJump(index, arg_nowStage);

		}

		//最小値保存用変数
		float minDistance = std::numeric_limits<float>().max();
		int minIndex = -1;

		//全衝突点の中から最短の位置にあるものを検索する。
		for (auto& index : arg_castRayArgment.m_impactPoint) {

			//衝突点が有効化されていなかったら処理を飛ばす。
			if (!index.m_isActive) continue;

			//距離が保存されているやつより大きかったら処理を飛ばす。
			float distance = (arg_newPos - index.m_impactPos).Length();
			if (minDistance < distance) continue;

			//距離やインデックスのデータを保存。
			minDistance = distance;
			minIndex = static_cast<int>(&index - &arg_castRayArgment.m_impactPoint[0]);

		}

		//地点が保存されていなかったら処理を飛ばす。
		if (minIndex != -1) {

			//引っ掛かっているオブジェクトがすぐにジャンプできるやつだったらフラグを更新。
			if (arg_castRayArgment.m_impactPoint[minIndex].m_isFastJump) {
				m_refPlayer->m_canJump = true;
			}

			if (arg_castRayArgment.m_impactPoint[minIndex].m_isAppearWall) {
				arg_hitInfo->m_terrianNormal = m_refPlayer->m_transform.GetUp();
			}
			//ジャンプができる状態だったらジャンプする。
			else if (m_refPlayer->m_canJump) {

				//最短の衝突点を求めたら、それをジャンプ先にする。
				arg_hitInfo->m_terrianNormal = arg_castRayArgment.m_impactPoint[minIndex].m_normal;

				//ジャンプのパラメーターも決める。
				m_refPlayer->m_playerMoveStatus = Player::PLAYER_MOVE_STATUS::JUMP;
				m_refPlayer->m_jumpTimer = 0;
				m_refPlayer->m_jumpStartPos = arg_newPos;
				m_refPlayer->m_bezierCurveControlPos = m_refPlayer->m_jumpStartPos + m_refPlayer->m_transform.GetUp() * m_refPlayer->WALL_JUMP_LENGTH;
				m_refPlayer->m_jumpEndPos = arg_castRayArgment.m_impactPoint[minIndex].m_impactPos + arg_castRayArgment.m_impactPoint[minIndex].m_normal * (m_refPlayer->m_transform.GetScale().x / 2.0f);
				m_refPlayer->m_jumpEndPos += m_refPlayer->m_transform.GetUp() * m_refPlayer->WALL_JUMP_LENGTH;

				//ジャンプした瞬間の入力を保存しておく。
				m_refPlayer->m_jumpTrrigerRowMoveVec = m_refPlayer->m_jumpRowMoveVec;

				//プレイヤーの入力を反転させる。
				bool isHitCeiling = arg_hitInfo->m_terrianNormal.y < -0.9f && !m_refPlayer->m_isCameraUpInverse;
				bool isHitGround = 0.9f < arg_hitInfo->m_terrianNormal.y && m_refPlayer->m_isCameraUpInverse;
				if (isHitCeiling || isHitGround) {
					m_refPlayer->m_isCameraInvX = true;
				}

				//天井か床だったらカメラを反転させる。
				if (arg_hitInfo->m_terrianNormal.y < -0.9f) {

					m_refPlayer->m_isCameraUpInverse = true;

				}
				else if (0.9f < arg_hitInfo->m_terrianNormal.y) {

					m_refPlayer->m_isCameraUpInverse = false;

				}

				//ジャンプしたのでタイマーを初期化
				m_refPlayer->m_canJumpDelayTimer = 0;

			}
			//ジャンプができない状態だったら押し戻す。
			else {

				arg_newPos = arg_from + m_refPlayer->m_gimmickVel;
				arg_hitInfo->m_terrianNormal = m_refPlayer->m_transform.GetUp();

				//引っ掛かりのタイマーを更新 この値が一定値をこえたらジャンプできるようになる
				++m_refPlayer->m_canJumpDelayTimer;

			}

			return;

		}

	}



	//どことも当たっていなかったら現在の上ベクトルを地形の上ベクトルとしてみる。
	arg_hitInfo->m_terrianNormal = m_refPlayer->m_transform.GetUp();

	//どこにも引っかかってないのでタイマーを初期化する。
	m_refPlayer->m_canJumpDelayTimer = 0;


}

void PlayerCollision::CheckHitGround(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, PlayerCollision::CastRayArgument& arg_castRayArgment) {

	//プレイヤーの回転を考慮しない、法線情報だけを見た場合のトランスフォーム。
	KuroEngine::Transform localTrans;
	localTrans.SetRotate(m_refPlayer->m_normalSpinQ);


	//プレイヤーの回転を考慮しない回転行列からレイを飛ばす方向を取得する。
	KuroEngine::Vec3<float> rightDir = localTrans.GetRight();
	KuroEngine::Vec3<float> frontDir = localTrans.GetFront();

	//ギミックに当たっているかどうかの変数を初期化
	m_refPlayer->m_prevOnGimmick = m_refPlayer->m_onGimmick;
	m_refPlayer->m_onGimmick = false;

	//接地フラグを保存
	m_refPlayer->m_prevOnGround = m_refPlayer->m_onGround;
	m_refPlayer->m_onGround = false;

	//崖チェックで下方向にレイを伸ばす際のレイの長さ
	m_checkUnderRayLength = m_refPlayer->m_transform.GetScale().y * 2.0f;
	m_checkCliffRayLength = m_refPlayer->m_transform.GetScale().y * 10.0f;

	//崖判定用フラグ
	m_isHitCliff = { false,false,false,false };

	//四方から下方向にレイを飛ばして崖チェック。
	CheckHitAllObject(&PlayerCollision::CheckHitCliff_Under, arg_newPos, arg_from, arg_castRayArgment, arg_nowStage);

	//当たっていなかったらプレイヤー方向にレイを飛ばして衝突点を記録する。
	m_impactPoint = std::array<std::vector<KuroEngine::Vec3<float>>, 4>();
	m_impactPointBuff = KuroEngine::Vec3<float>();	//座標一時保存用
	CheckHitAllObject(&PlayerCollision::CheckHitCliff_SearchImpactPoint, arg_newPos, arg_from, arg_castRayArgment, arg_nowStage);

	//求められた衝突点の中から各方向の一番近い衝突点を見つける。
	std::vector<KuroEngine::Vec3<float>> nearPos;
	std::vector<KuroEngine::Vec3<float>> nearestPos;
	//まずは右から
	for (auto& index : m_impactPoint[static_cast<int>(RAY_DIR_ID::RIGHT)]) {

		float minLength = std::numeric_limits<float>().max();
		KuroEngine::Vec3<float> castPos = arg_newPos + rightDir * m_refPlayer->WALL_JUMP_LENGTH - m_refPlayer->m_transform.GetUp() * m_checkUnderRayLength;
		float length = KuroEngine::Vec3<float>(castPos - index).Length();
		if (length < minLength) {
			minLength = length;
			nearPos.emplace_back(index);
		}

	}
	if (0 < nearPos.size())nearestPos.emplace_back(nearPos.back());
	nearPos.clear();
	//次は左
	for (auto& index : m_impactPoint[static_cast<int>(RAY_DIR_ID::LEFT)]) {

		float minLength = std::numeric_limits<float>().max();
		KuroEngine::Vec3<float> castPos = arg_newPos - rightDir * m_refPlayer->WALL_JUMP_LENGTH - m_refPlayer->m_transform.GetUp() * m_checkUnderRayLength;
		float length = KuroEngine::Vec3<float>(castPos - index).Length();
		if (length < minLength) {
			minLength = length;
			nearPos.emplace_back(index);
		}

	}
	if (0 < nearPos.size())nearestPos.emplace_back(nearPos.back());
	nearPos.clear();
	//次は前
	for (auto& index : m_impactPoint[static_cast<int>(RAY_DIR_ID::FRONT)]) {

		float minLength = std::numeric_limits<float>().max();
		KuroEngine::Vec3<float> castPos = arg_newPos + frontDir * m_refPlayer->WALL_JUMP_LENGTH - m_refPlayer->m_transform.GetUp() * m_checkUnderRayLength;
		float length = KuroEngine::Vec3<float>(castPos - index).Length();
		if (length < minLength) {
			minLength = length;
			nearPos.emplace_back(index);
		}
	}
	if (0 < nearPos.size())nearestPos.emplace_back(nearPos.back());
	nearPos.clear();
	//最後に後ろ
	for (auto& index : m_impactPoint[static_cast<int>(RAY_DIR_ID::BEHIND)]) {

		float minLength = std::numeric_limits<float>().max();
		KuroEngine::Vec3<float> castPos = arg_newPos - frontDir * m_refPlayer->WALL_JUMP_LENGTH - m_refPlayer->m_transform.GetUp() * m_checkUnderRayLength;
		float length = KuroEngine::Vec3<float>(castPos - index).Length();
		if (length < minLength) {
			minLength = length;
			nearPos.emplace_back(index);
		}
	}
	if (0 < nearPos.size())nearestPos.emplace_back(nearPos.back());
	nearPos.clear();

	//衝突点の平均値が押し戻し場所
	if (0 < nearestPos.size()) {
		KuroEngine::Vec3<float> pushBackPos;
		for (auto& index : nearestPos) {
			pushBackPos += index;
		}
		if (1 < nearestPos.size()) {
			int a = 0;
		}
		pushBackPos /= static_cast<float>(nearestPos.size());
		arg_newPos = pushBackPos;
	}


	//最終的な結果をもとに下方向にレイを飛ばして接地判定を行う。

	//地形配列走査
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//モデル情報取得
		auto model = terrian.GetModel().lock();
		//情報を取得。
		arg_castRayArgment.m_stageType = StageParts::TERRIAN;

		//メッシュを走査
		for (auto& modelMesh : model->m_meshes)
		{
			//メッシュ情報取得
			auto& mesh = modelMesh.mesh;

			//CastRayに渡す引数を更新。
			arg_castRayArgment.m_mesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//判定↓============================================

			//動いた方向基準の姿勢
			KuroEngine::Transform moveQtransform;
			moveQtransform.SetRotate(m_refPlayer->m_moveQ);

			m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos + moveQtransform.GetFront() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
			m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos - moveQtransform.GetFront() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
			m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos + moveQtransform.GetRight() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
			m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos - moveQtransform.GetRight() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);

			//=================================================
		}
	}
	//動く足場との当たり判定
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//動く足場でない
		if (terrian->GetType() != StageParts::MOVE_SCAFFOLD)continue;

		//距離によってカリング
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 100.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

		//動く足場としてキャスト
		auto moveScaffold = dynamic_pointer_cast<MoveScaffold>(terrian);

		//プレイヤーが乗っているかのフラグを一旦おる。
		moveScaffold->SetOnPlayer(false);

		//モデル情報取得
		auto model = terrian->GetModel();
		//情報を取得。
		arg_castRayArgment.m_stageType = terrian->GetType();
		//ステージ情報を保存。
		arg_castRayArgment.m_stage = terrian;

		//メッシュを走査
		for (auto& modelMesh : model.lock()->m_meshes)
		{
			//メッシュ情報取得
			auto& mesh = modelMesh.mesh;

			//CastRayに渡す引数を更新。
			arg_castRayArgment.m_mesh = moveScaffold->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//判定↓============================================

			//動いた方向基準の姿勢
			KuroEngine::Transform moveQtransform;
			moveQtransform.SetRotate(m_refPlayer->m_moveQ);

			m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos + moveQtransform.GetFront() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
			m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos - moveQtransform.GetFront() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
			m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos + moveQtransform.GetRight() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
			m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos - moveQtransform.GetRight() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);


			//まずは真下にレイを飛ばす。
			bool isHit = CastRay(arg_newPos, arg_newPos, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

			//次に動いた方向の後ろ側からレイを飛ばして当たっていたらギミックを起動する。
			if (isHit) {

				m_refPlayer->m_onGimmick = true;

				//プレイヤーが乗ったことをギミック側に伝える。
				moveScaffold->OnPlayer();

			}

			//=================================================
		}
	}

	//蔦ブロックとの当たり判定
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//動く足場でない
		if (terrian->GetType() != StageParts::IVY_BLOCK)continue;

		//距離によってカリング
		const float DEADLINE = 100.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

		//動く足場としてキャスト
		auto ivyBlock = dynamic_pointer_cast<IvyBlock>(terrian);

		//モデル情報取得
		auto model = terrian->GetModel();
		//情報を取得。
		arg_castRayArgment.m_stageType = terrian->GetType();
		//ステージ情報を保存。
		arg_castRayArgment.m_stage = terrian;

		//メッシュを走査
		for (auto& modelMesh : model.lock()->m_meshes)
		{
			//メッシュ情報取得
			auto& mesh = modelMesh.mesh;

			//CastRayに渡す引数を更新。
			arg_castRayArgment.m_mesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//判定↓============================================

			//ブロックが出現中のときの処理
			if (ivyBlock->GetIsAppear()) {

				//動いた方向基準の姿勢
				KuroEngine::Transform moveQtransform;
				moveQtransform.SetRotate(m_refPlayer->m_moveQ);

				m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos + moveQtransform.GetFront() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
				m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos - moveQtransform.GetFront() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
				m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos + moveQtransform.GetRight() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);
				m_refPlayer->m_onGround |= CastRay(arg_newPos, arg_newPos - moveQtransform.GetRight() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);

				//出現していたら
				if (ivyBlock->GetIsAppear()) {

					//移動方向基準の後ろの方にレイを飛ばす。
					bool isHit = CastRay(arg_newPos, arg_newPos - moveQtransform.GetFront() * m_refPlayer->m_transform.GetScale().x, -m_refPlayer->m_transform.GetUp(), m_refPlayer->m_transform.GetScale().y, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

					//プレイヤーが乗っている判定のときにレイが当たらなかったら、出現状態を切る。
					if (ivyBlock->GetOnPlayer() && !isHit) {

						ivyBlock->Disappear();

						//SEを鳴らす。
						SoundConfig::Instance()->Play(SoundConfig::SE_GRASS);

					}
					//プレイヤーが乗っている判定。
					else if (isHit) {

						ivyBlock->OnPlayer();

					}

				}

			}
			//出現中じゃないときの処理
			else {

				//プレイヤーにヒットしていたら、大きめの当たり判定を行い、当たっていなかったら出現させる。
				if (ivyBlock->GetOnPlayer()) {
					bool isHit = KuroEngine::Vec3<float>(arg_newPos - ivyBlock->GetPos()).Length() <= ivyBlock->GetHitScaleMax();
					if (!isHit) {

						ivyBlock->Appear();
						ivyBlock->OffPlayer();

						//SEを鳴らす。
						SoundConfig::Instance()->Play(SoundConfig::SE_GRASS);

					}
				}
				//プレイヤーのヒットしていなかったら、小さめの当たり判定を行い、出現させられるようにする。
				else {
					bool isHit = KuroEngine::Vec3<float>(arg_newPos - ivyBlock->GetPos()).Length() <= ivyBlock->GetHitScaleMin();
					if (isHit) {

						ivyBlock->OnPlayer();

					}
				}


			}


			//=================================================
		}
	}

}

void PlayerCollision::CheckCliff(PlayerCollision::ImpactPointData& arg_impactPointData, std::weak_ptr<Stage> arg_nowStage)
{

	//有効化されていなかったら処理を飛ばす。
	if (!arg_impactPointData.m_isActive) return;

	//崖判定用のレイの長さ
	const float CLIFF_RAY_LENGTH = m_refPlayer->WALL_JUMP_LENGTH + m_refPlayer->m_transform.GetScale().Length();

	//地形配列走査
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//モデル情報取得
		auto model = terrian.GetModel().lock();

		//メッシュを走査
		for (auto& modelMesh : model->m_meshes)
		{

			//判定↓============================================

			//当たり判定を行うメッシュ。
			std::vector<TerrianHitPolygon> mesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//下方向にレイを飛ばす。
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * 0.5f, -m_refPlayer->m_transform.GetUp(), mesh);

			//レイがメッシュに衝突しており、衝突地点までの距離がレイの長さより小さかったら衝突している。
			if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

				//壁に当たった時点で崖ではないので処理を飛ばす。
				return;

			}

			//=================================================
		}
	}

	//動く足場との当たり判定
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//動く足場でない
		if (terrian->GetType() != StageParts::MOVE_SCAFFOLD)continue;

		//距離によってカリング
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 100.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

		//動く足場としてキャスト
		auto moveScaffold = dynamic_pointer_cast<MoveScaffold>(terrian);

		//モデル情報取得
		auto model = terrian->GetModel();

		//メッシュを走査
		for (auto& modelMesh : model.lock()->m_meshes) {

			//判定↓============================================

			//当たり判定を行うメッシュ。
			std::vector<TerrianHitPolygon> mesh = moveScaffold->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//下方向にレイを飛ばす。
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * 0.5f, -m_refPlayer->m_transform.GetUp(), mesh);

			//レイがメッシュに衝突しており、衝突地点までの距離がレイの長さより小さかったら衝突している。
			if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

				//壁に当たった時点で崖ではないので処理を飛ばす。
				return;

			}

			//=================================================
		}
	}

	//蔦ブロックとの当たり判定
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//動く足場でない
		if (terrian->GetType() != StageParts::IVY_BLOCK)continue;

		//距離によってカリング
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 5.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

		//動く足場としてキャスト
		auto ivyBlock = dynamic_pointer_cast<IvyBlock>(terrian);

		//出現状態じゃなかったら処理を飛ばす。
		if (!ivyBlock->GetIsAppear()) continue;

		//モデル情報取得
		auto model = terrian->GetModel();

		//メッシュを走査
		for (auto& modelMesh : model.lock()->m_meshes)
		{

			//当たり判定を行うメッシュ。
			std::vector<TerrianHitPolygon> mesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//下方向にレイを飛ばす。
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * 0.5f, -m_refPlayer->m_transform.GetUp(), mesh);

			//レイがメッシュに衝突しており、衝突地点までの距離がレイの長さより小さかったら衝突している。
			if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

				//壁に当たった時点で崖ではないので処理を飛ばす。
				return;

			}
			//=================================================
		}
	}

	//見えない壁との当たり判定
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//動く足場でない
		if (terrian->GetType() != StageParts::APPEARANCE)continue;

		//動く足場としてキャスト
		auto ivyBlock = dynamic_pointer_cast<Appearance>(terrian);

		//モデル情報取得
		auto model = terrian->GetModel();

		//メッシュを走査
		for (auto& modelMesh : model.lock()->m_meshes)
		{

			//当たり判定を行うメッシュ。
			std::vector<TerrianHitPolygon> mesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//下方向にレイを飛ばす。
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * 0.5f, -m_refPlayer->m_transform.GetUp(), mesh);

			//レイがメッシュに衝突しており、衝突地点までの距離がレイの長さより小さかったら衝突している。
			if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

				//壁に当たった時点で崖ではないので処理を飛ばす。
				return;

			}
			//=================================================
		}
	}

	//フェンスとの当たり判定
	if (!m_refPlayer->GetIsUnderGround()) {
		for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
		{
			//動く足場でない
			if (terrian->GetType() != StageParts::SPLATOON_FENCE)continue;

			//距離によってカリング
			const float DEADLINE = terrian->GetTransform().GetScale().Length() * 2.0f;
			float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
			if (DEADLINE < distance) continue;

			//動く足場としてキャスト
			auto ivyBlock = dynamic_pointer_cast<SplatoonFence>(terrian);

			//モデル情報取得
			auto model = terrian->GetModel();

			//メッシュを走査
			for (auto& modelMesh : model.lock()->m_meshes)
			{

				//当たり判定を行うメッシュ。
				std::vector<TerrianHitPolygon> mesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

				//下方向にレイを飛ばす。
				CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * 0.5f, -m_refPlayer->m_transform.GetUp(), mesh);

				//レイがメッシュに衝突しており、衝突地点までの距離がレイの長さより小さかったら衝突している。
				if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

					//壁に当たった時点で崖ではないので処理を飛ばす。
					return;

				}
				//=================================================
			}
		}
	}

	//最後まで壁に当たってなかったら崖を超えているので無効化する。
	arg_impactPointData.m_isActive = false;

}

void PlayerCollision::CheckCanJump(PlayerCollision::ImpactPointData& arg_impactPointData, std::weak_ptr<Stage> arg_nowStage)
{

	//有効化されていなかったら処理を飛ばす。
	if (!arg_impactPointData.m_isActive) return;

	//崖判定用のレイの長さ
	const float CLIFF_RAY_LENGTH = m_refPlayer->WALL_JUMP_LENGTH + m_refPlayer->m_transform.GetScale().Length();

	//地形配列走査
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//モデル情報取得
		auto model = terrian.GetModel().lock();

		//メッシュを走査
		for (auto& modelMesh : model->m_meshes)
		{

			//判定↓============================================

			//当たり判定を行うメッシュ。
			std::vector<TerrianHitPolygon> mesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//下方向にレイを飛ばす。
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision((arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * m_refPlayer->m_transform.GetScale().x) + m_refPlayer->m_transform.GetUp() * m_refPlayer->WALL_JUMP_LENGTH, -arg_impactPointData.m_normal, mesh);

			//レイがメッシュに衝突しており、衝突地点までの距離がレイの長さより小さかったら衝突している。
			if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

				//壁に当たった時点で崖ではないので処理を飛ばす。
				return;

			}

			//=================================================
		}
	}

	//動く足場との当たり判定
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//動く足場でない
		if (terrian->GetType() != StageParts::MOVE_SCAFFOLD)continue;

		//距離によってカリング
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 100.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

		//動く足場としてキャスト
		auto moveScaffold = dynamic_pointer_cast<MoveScaffold>(terrian);

		//モデル情報取得
		auto model = terrian->GetModel();

		//メッシュを走査
		for (auto& modelMesh : model.lock()->m_meshes) {

			//判定↓============================================

			//当たり判定を行うメッシュ。
			std::vector<TerrianHitPolygon> mesh = moveScaffold->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//下方向にレイを飛ばす。
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision((arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * m_refPlayer->m_transform.GetScale().x) + m_refPlayer->m_transform.GetUp() * m_refPlayer->WALL_JUMP_LENGTH, -arg_impactPointData.m_normal, mesh);

			if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

				//壁に当たった時点で崖ではないので処理を飛ばす。
				return;

			}

			//=================================================
		}
	}

	//蔦ブロックとの当たり判定
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//動く足場でない
		if (terrian->GetType() != StageParts::IVY_BLOCK)continue;

		//距離によってカリング
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 5.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

		//動く足場としてキャスト
		auto ivyBlock = dynamic_pointer_cast<IvyBlock>(terrian);

		//出現状態じゃなかったら処理を飛ばす。
		if (!ivyBlock->GetIsAppear()) continue;

		//モデル情報取得
		auto model = terrian->GetModel();

		//メッシュを走査
		for (auto& modelMesh : model.lock()->m_meshes) {

			//判定↓============================================

			//当たり判定を行うメッシュ。
			std::vector<TerrianHitPolygon> mesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//下方向にレイを飛ばす。
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision((arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * m_refPlayer->m_transform.GetScale().x) + m_refPlayer->m_transform.GetUp() * m_refPlayer->WALL_JUMP_LENGTH, -arg_impactPointData.m_normal, mesh);

			//レイがメッシュに衝突しており、衝突地点までの距離がレイの長さより小さかったら衝突している。
			if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

				//壁に当たった時点で崖ではないので処理を飛ばす。
				return;

			}

			//=================================================
		}
	}

	//見えない壁との当たり判定
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//動く足場でない
		if (terrian->GetType() != StageParts::APPEARANCE)continue;

		//距離によってカリング
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 100.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

		//見えない壁としてキャスト
		auto appearWall = dynamic_pointer_cast<Appearance>(terrian);

		//モデル情報取得
		auto model = terrian->GetModel();

		//メッシュを走査
		for (auto& modelMesh : model.lock()->m_meshes) {

			//判定↓============================================

			//当たり判定を行うメッシュ。
			std::vector<TerrianHitPolygon> mesh = appearWall->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//下方向にレイを飛ばす。
			CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision((arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * m_refPlayer->m_transform.GetScale().x) + m_refPlayer->m_transform.GetUp() * m_refPlayer->WALL_JUMP_LENGTH, -arg_impactPointData.m_normal, mesh);

			//レイがメッシュに衝突しており、衝突地点までの距離がレイの長さより小さかったら衝突している。
			if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

				//壁に当たった時点で崖ではないので処理を飛ばす。
				return;

			}

			//=================================================
		}
	}

	//フェンスとの当たり判定
	if (!m_refPlayer->GetIsUnderGround()) {
		for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
		{
			//動く足場でない
			if (terrian->GetType() != StageParts::SPLATOON_FENCE)continue;

			//距離によってカリング
			const float DEADLINE = terrian->GetTransform().GetScale().Length() * 2.0f;
			float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
			if (DEADLINE < distance) continue;

			//見えない壁としてキャスト
			auto appearWall = dynamic_pointer_cast<SplatoonFence>(terrian);

			//モデル情報取得
			auto model = terrian->GetModel();

			//メッシュを走査
			for (auto& modelMesh : model.lock()->m_meshes) {

				//判定↓============================================

				//当たり判定を行うメッシュ。
				std::vector<TerrianHitPolygon> mesh = appearWall->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

				//下方向にレイを飛ばす。
				CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision((arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * m_refPlayer->m_transform.GetScale().x) + m_refPlayer->m_transform.GetUp() * m_refPlayer->WALL_JUMP_LENGTH, -arg_impactPointData.m_normal, mesh);

				//レイがメッシュに衝突しており、衝突地点までの距離がレイの長さより小さかったら衝突している。
				if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

					//壁に当たった時点で崖ではないので処理を飛ばす。
					return;

				}

				//=================================================
			}
		}
	}

	//最後まで壁に当たってなかったら崖を超えているので無効化する。
	arg_impactPointData.m_isActive = false;

}

bool PlayerCollision::CastRay(KuroEngine::Vec3<float>& arg_charaPos, const KuroEngine::Vec3<float>& arg_rayCastPos, const KuroEngine::Vec3<float>& arg_rayDir, float arg_rayLength, CastRayArgument& arg_collisionData, RAY_ID arg_rayID, RAY_DIR_ID arg_rayDirID)
{

	/*===== 当たり判定用のレイを撃つ =====*/

	//レイを飛ばす。
	CollisionDetectionOfRayAndMesh::MeshCollisionOutput output = CollisionDetectionOfRayAndMesh::Instance()->MeshCollision(arg_rayCastPos, arg_rayDir, arg_collisionData.m_mesh);

	//レイがメッシュに衝突しており、衝突地点までの距離がレイの長さより小さかったら衝突している。
	if (output.m_isHit && std::fabs(output.m_distance) < arg_rayLength) {

		//ぴったり押し戻してしまうと重力の関係でガクガクしてしまうので、微妙にめり込ませて押し戻す。
		static const float OFFSET = 0.1f;

		//レイの種類によって保存するデータを変える。
		switch (arg_rayID)
		{
		case RAY_ID::GROUND:

			//外部に渡す用のデータを保存
			arg_collisionData.m_bottomTerrianNormal = output.m_normal;

			//押し戻す。
			arg_charaPos += output.m_normal * (std::fabs(output.m_distance - m_refPlayer->m_transform.GetScale().x) - OFFSET);

			m_refPlayer->m_underRayHitPosition = output.m_pos;

			break;

		case RAY_ID::AROUND:

			//レイの衝突地点を保存。
			arg_collisionData.m_impactPoint.emplace_back(ImpactPointData(output.m_pos, output.m_normal));

			//動く床だったらめり込んでしまうので押し戻す。
			if (arg_collisionData.m_stageType == StageParts::MOVE_SCAFFOLD) {

				arg_charaPos += output.m_normal * (std::fabs(output.m_distance - m_refPlayer->m_transform.GetScale().x) - OFFSET);

			}
			//動けない壁かフェンスだったら
			else if (arg_collisionData.m_stageType == StageParts::APPEARANCE || arg_collisionData.m_stageType == StageParts::SPLATOON_FENCE) {

				arg_charaPos += output.m_normal * (std::fabs(output.m_distance - arg_rayLength) - OFFSET);
				arg_collisionData.m_impactPoint.back().m_isAppearWall = true;
				arg_collisionData.m_bottomTerrianNormal = m_refPlayer->m_transform.GetUp();

			}
			else if (m_refPlayer->m_prevOnGimmick) {

				arg_collisionData.m_impactPoint.back().m_isFastJump = true;

			}

			break;

		case RAY_ID::CLIFF:
		{

			//外部に渡す用のデータを保存
			arg_collisionData.m_bottomTerrianNormal = m_refPlayer->m_transform.GetUp();

			//衝突地点から上方向に位置をずらす。
			const float m_checkUnderRayLength = m_refPlayer->m_transform.GetScale().y * 2.0f;
			output.m_pos += m_refPlayer->m_transform.GetUp() * m_checkUnderRayLength;

			//押し戻した位置に座標を設定。
			arg_charaPos = output.m_pos - (output.m_normal * m_refPlayer->m_transform.GetScale().x * (m_refPlayer->WALL_JUMP_LENGTH - OFFSET));

		}
		break;

		case RAY_ID::CHECK_GIMMICK:

			m_refPlayer->m_onGimmick = true;

			//さらにギミックに当たったトリガーだったらギミックを有効化させる。
			if (!m_refPlayer->m_prevOnGimmick) {
				dynamic_pointer_cast<MoveScaffold>(arg_collisionData.m_stage.lock())->Activate();
			}


			break;

		case RAY_ID::CHECK_DEATH:

			arg_collisionData.m_checkDeathCounter[static_cast<int>(arg_rayDirID)] = true;

			break;

		case RAY_ID::CHECK_CLIFF:

			break;

		case RAY_ID::CHECK_IVY:

			m_refPlayer->m_gimmickExitPos.emplace_back(output.m_pos + output.m_normal);
			m_refPlayer->m_gimmickExitNormal.emplace_back(output.m_normal);

			break;

		default:
			break;
		}

		//当たった
		return true;


	}
	else {

		//当たらなかった
		return false;

	}

}

void PlayerCollision::CheckHit(KuroEngine::Vec3<float>& arg_frompos, KuroEngine::Vec3<float>& arg_nowpos, std::weak_ptr<Stage>arg_nowStage) {

	HitCheckResult hitResult;
	if (!HitCheckAndPushBack(arg_frompos, arg_nowpos, arg_nowStage, &hitResult))return;

	//地形の法線が真下を向いているときに誤差できれいに0,-1,0になってくれないせいでうまくいかないので苦肉の策。
	if (hitResult.m_terrianNormal.y < -0.9f) {
		hitResult.m_terrianNormal = { 0,-1,0 };
	}

	//法線方向を見るクォータニオン
	m_refPlayer->m_normalSpinQ = KuroEngine::Math::GetLookAtQuaternion({ 0,1,0 }, hitResult.m_terrianNormal);

	//カメラの回転でY軸回転させるクォータニオン。移動方向に回転しているように見せかけるためのもの。m_refPlayer->m_cameraJumpLerpAmountは補間後のカメラに向かって補間するため。
	DirectX::XMVECTOR ySpin;
	if (m_refPlayer->m_isCameraUpInverse) {
		ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, -(m_refPlayer->m_cameraRotMove + m_refPlayer->m_cameraJumpLerpAmount) + DirectX::XM_PI);
	}
	else {
		ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_refPlayer->m_cameraRotMove + m_refPlayer->m_cameraJumpLerpAmount);
	}

	//プレイヤーの移動方向でY軸回転させるクォータニオン。移動方向に回転しているように見せかけるためのもの。
	DirectX::XMVECTOR playerYSpin;
	playerYSpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_refPlayer->m_playerRotY);

	//カメラ方向でのクォータニオンを求める。進む方向などを判断するのに使用するのはこっち。Fの一番最初にこの値を入れることでplayerYSpinの回転を打ち消す。
	m_refPlayer->m_cameraQ = DirectX::XMQuaternionMultiply(m_refPlayer->m_normalSpinQ, ySpin);

	//プレイヤーの移動方向でY軸回転させるクォータニオンをカメラのクォータニオンにかけて、プレイヤーを移動方向に向かせる。
	m_refPlayer->m_moveQ = DirectX::XMQuaternionMultiply(m_refPlayer->m_cameraQ, playerYSpin);

	//ジャンプ状態だったら
	if (m_refPlayer->m_playerMoveStatus == Player::PLAYER_MOVE_STATUS::JUMP) {

		//ジャンプ後に回転するようにする。

		//クォータニオンを保存。
		m_refPlayer->m_jumpEndQ = m_refPlayer->m_moveQ;
		m_refPlayer->m_jumpStartQ = m_refPlayer->m_prevTransform.GetRotate();
		m_refPlayer->m_transform.SetRotate(m_refPlayer->m_prevTransform.GetRotate());

	}
	else {

		//当たった面基準の回転にする。
		m_refPlayer->m_transform.SetRotate(m_refPlayer->m_moveQ);

	}

}

void PlayerCollision::CheckZipline(const KuroEngine::Vec3<float> arg_newPos, std::weak_ptr<Stage> arg_nowStage) {

	//ジップラインとの当たり判定
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//ジップラインではない
		if (terrian->GetType() != StageParts::IVY_ZIP_LINE)continue;

		//ジップラインとしてキャスト
		auto zipline = dynamic_pointer_cast<IvyZipLine>(terrian);

		//ジップラインに登録されている頂点が1個以下だったら処理を飛ばす。
		if (static_cast<int>(zipline->GetTranslationArraySize() <= 1)) continue;

		//判定↓============================================

		//始点との当たり判定
		bool isHit = KuroEngine::Vec3<float>(zipline->GetStartPoint() - arg_newPos).Length() <= (m_refPlayer->m_transform.GetScale().x + zipline->JUMP_SCALE);
		if (isHit && m_refPlayer->m_canZip) {
			m_refPlayer->m_gimmickStatus = Player::GIMMICK_STATUS::APPEAR;
			m_refPlayer->m_playerMoveStatus = Player::PLAYER_MOVE_STATUS::ZIP;
			m_refPlayer->m_ziplineMoveTimer = 0;
			zipline->CheckHit(true);
			m_refPlayer->m_refZipline = zipline;
			m_refPlayer->m_zipInOutPos = arg_newPos;

			//SEを鳴らす。
			SoundConfig::Instance()->Play(SoundConfig::SE_ZIP_LINE_GET_ON);
		}

		//終点との当たり判定
		isHit = KuroEngine::Vec3<float>(zipline->GetEndPoint() - arg_newPos).Length() <= (m_refPlayer->m_transform.GetScale().x + zipline->JUMP_SCALE);
		if (isHit && m_refPlayer->m_canZip) {
			m_refPlayer->m_gimmickStatus = Player::GIMMICK_STATUS::APPEAR;
			m_refPlayer->m_playerMoveStatus = Player::PLAYER_MOVE_STATUS::ZIP;
			m_refPlayer->m_ziplineMoveTimer = 0;
			zipline->CheckHit(false);
			m_refPlayer->m_refZipline = zipline;
			m_refPlayer->m_zipInOutPos = arg_newPos;

			//SEを鳴らす。
			SoundConfig::Instance()->Play(SoundConfig::SE_ZIP_LINE_GET_ON);
		}

		//=================================================

	}

}

void PlayerCollision::FinishGimmickMove()
{

	//全方向に地面用のレイを飛ばして地面判定をする。

	m_refPlayer->m_gimmickExitPos.clear();
	m_refPlayer->m_gimmickExitNormal.clear();

	//押し戻し座標
	KuroEngine::Vec3<float> pos = m_refPlayer->m_transform.GetPosWorld();

	PlayerCollision::CastRayArgument castRayArgument;

	//レイの長さ
	const float RAY_LENGTH = 10.0f;

	//地形配列走査
	for (auto& terrian : m_refPlayer->m_stage.lock()->GetTerrianArray())
	{
		//モデル情報取得
		auto model = terrian.GetModel().lock();
		//情報を取得。
		castRayArgument.m_stageType = StageParts::TERRIAN;

		//メッシュを走査
		for (auto& modelMesh : model->m_meshes)
		{
			//メッシュ情報取得
			auto& mesh = modelMesh.mesh;

			//CastRayに渡す引数を更新。
			castRayArgument.m_mesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//判定↓============================================

			//右方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(pos, pos, m_refPlayer->m_transform.GetRight(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//左方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(pos, pos, -m_refPlayer->m_transform.GetRight(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//後ろ方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(pos, pos, -m_refPlayer->m_transform.GetFront(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//正面方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(pos, pos, m_refPlayer->m_transform.GetFront(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//下方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(pos, pos, -m_refPlayer->m_transform.GetUp(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//上方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(pos, pos, m_refPlayer->m_transform.GetUp(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//=================================================
		}
	}

	//動く足場との当たり判定
	for (auto& terrian : m_refPlayer->m_stage.lock()->GetGimmickArray())
	{
		//動く足場でない
		if (terrian->GetType() != StageParts::MOVE_SCAFFOLD)continue;

		//動く足場としてキャスト
		auto moveScaffold = dynamic_pointer_cast<MoveScaffold>(terrian);

		//モデル情報取得
		auto model = terrian->GetModel();
		//情報を取得。
		castRayArgument.m_stageType = terrian->GetType();
		//ステージ情報を保存。
		castRayArgument.m_stage = terrian;

		//メッシュを走査
		for (auto& modelMesh : model.lock()->m_meshes)
		{
			//メッシュ情報取得
			auto& mesh = modelMesh.mesh;

			//CastRayに渡す引数を更新。
			castRayArgument.m_mesh = moveScaffold->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//判定↓============================================

			//右方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(pos, pos, m_refPlayer->m_transform.GetRight(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//左方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(pos, pos, -m_refPlayer->m_transform.GetRight(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//後ろ方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(pos, pos, -m_refPlayer->m_transform.GetFront(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//正面方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(pos, pos, m_refPlayer->m_transform.GetFront(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//下方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(pos, pos, -m_refPlayer->m_transform.GetUp(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//上方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(pos, pos, m_refPlayer->m_transform.GetUp(), RAY_LENGTH, castRayArgument, RAY_ID::CHECK_IVY);

			//=================================================
		}
	}

	//最短のものを検索する。
	KuroEngine::Vec3<float> minPos = pos;
	KuroEngine::Vec3<float> normal = { 0,1,0 };
	float minLength = std::numeric_limits<float>().max();
	for (int index = 0; index < static_cast<int>(m_refPlayer->m_gimmickExitNormal.size()); ++index) {
		float length = KuroEngine::Vec3<float>(pos - m_refPlayer->m_gimmickExitPos[index]).Length();
		if (length < minLength) {
			minLength = length;
			minPos = m_refPlayer->m_gimmickExitPos[index];
			normal = m_refPlayer->m_gimmickExitNormal[index];
		}
	}

	m_refPlayer->m_zipInOutPos = minPos;

	//地形の法線が真下を向いているときに誤差できれいに0,-1,0になってくれないせいでうまくいかないので苦肉の策。
	if (normal.y < -0.9f) {
		normal = { 0,-1,0 };
	}

	//法線方向を見るクォータニオン
	m_refPlayer->m_normalSpinQ = KuroEngine::Math::GetLookAtQuaternion({ 0,1,0 }, normal);

	//カメラの回転でY軸回転させるクォータニオン。移動方向に回転しているように見せかけるためのもの。m_refPlayer->m_cameraJumpLerpAmountは補間後のカメラに向かって補間するため。
	DirectX::XMVECTOR ySpin;
	if (normal.y < -0.9f) {
		ySpin = DirectX::XMQuaternionRotationNormal(normal, -(m_refPlayer->m_cameraRotMove + m_refPlayer->m_cameraJumpLerpAmount) + DirectX::XM_PI);
	}
	else {
		ySpin = DirectX::XMQuaternionRotationNormal(normal, m_refPlayer->m_cameraRotMove + m_refPlayer->m_cameraJumpLerpAmount);
	}

	//プレイヤーの移動方向でY軸回転させるクォータニオン。移動方向に回転しているように見せかけるためのもの。
	DirectX::XMVECTOR playerYSpin;
	playerYSpin = DirectX::XMQuaternionRotationNormal(normal, m_refPlayer->m_playerRotY);

	//カメラ方向でのクォータニオンを求める。進む方向などを判断するのに使用するのはこっち。Fの一番最初にこの値を入れることでplayerYSpinの回転を打ち消す。
	m_refPlayer->m_cameraQ = DirectX::XMQuaternionMultiply(m_refPlayer->m_normalSpinQ, ySpin);

	//プレイヤーの移動方向でY軸回転させるクォータニオンをカメラのクォータニオンにかけて、プレイヤーを移動方向に向かせる。
	m_refPlayer->m_moveQ = DirectX::XMQuaternionMultiply(m_refPlayer->m_cameraQ, playerYSpin);

	//ジャンプ状態だったら
	if (m_refPlayer->m_playerMoveStatus == Player::PLAYER_MOVE_STATUS::JUMP) {

		//ジャンプ後に回転するようにする。

		//クォータニオンを保存。
		m_refPlayer->m_jumpEndQ = m_refPlayer->m_moveQ;
		m_refPlayer->m_jumpStartQ = m_refPlayer->m_prevTransform.GetRotate();
		m_refPlayer->m_transform.SetRotate(m_refPlayer->m_prevTransform.GetRotate());

	}
	else {

		//当たった面基準の回転にする。
		m_refPlayer->m_transform.SetRotate(m_refPlayer->m_moveQ);

	}

	m_refPlayer->m_gimmickStatus = Player::GIMMICK_STATUS::EXIT;

	//SEを鳴らす。
	SoundConfig::Instance()->Play(SoundConfig::SE_ZIP_LINE_GET_ON);
}

void PlayerCollision::CheckHitCliff_Under(KuroEngine::Vec3<float>& arg_newPos, const KuroEngine::Vec3<float>& arg_from, PlayerCollision::CastRayArgument& arg_castRayArgment, std::weak_ptr<Stage> arg_nowStage)
{

	//プレイヤーの回転を考慮しない、法線情報だけを見た場合のトランスフォーム。
	KuroEngine::Transform localTrans;
	localTrans.SetRotate(m_refPlayer->m_normalSpinQ);

	//プレイヤーの回転を考慮しない回転行列からレイを飛ばす方向を取得する。
	KuroEngine::Vec3<float> rightDir = localTrans.GetRight();
	KuroEngine::Vec3<float> frontDir = localTrans.GetFront();

	//右側の周囲のレイが当たっていなかったら。
	if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::RIGHT)]) {

		//下方向にレイを飛ばしてそこが崖かをチェックする。
		KuroEngine::Vec3<float> rayPos = arg_newPos + rightDir * m_refPlayer->WALL_JUMP_LENGTH;
		m_isHitCliff[static_cast<int>(RAY_DIR_ID::RIGHT)] |= CastRay(arg_newPos, rayPos, -m_refPlayer->m_transform.GetUp(), m_checkUnderRayLength, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

	}

	//右側の周囲のレイが当たっていなかったら。
	if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::LEFT)]) {

		//下方向にレイを飛ばしてそこが崖かをチェックする。
		KuroEngine::Vec3<float> rayPos = arg_newPos - rightDir * m_refPlayer->WALL_JUMP_LENGTH;
		m_isHitCliff[static_cast<int>(RAY_DIR_ID::LEFT)] |= CastRay(arg_newPos, rayPos, -m_refPlayer->m_transform.GetUp(), m_checkUnderRayLength, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

	}

	//右側の周囲のレイが当たっていなかったら。
	if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::FRONT)]) {

		//下方向にレイを飛ばしてそこが崖かをチェックする。
		KuroEngine::Vec3<float> rayPos = arg_newPos + frontDir * m_refPlayer->WALL_JUMP_LENGTH;
		m_isHitCliff[static_cast<int>(RAY_DIR_ID::FRONT)] |= CastRay(arg_newPos, rayPos, -m_refPlayer->m_transform.GetUp(), m_checkUnderRayLength, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

	}

	//右側の周囲のレイが当たっていなかったら。
	if (!arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::BEHIND)]) {

		//下方向にレイを飛ばしてそこが崖かをチェックする。
		KuroEngine::Vec3<float> rayPos = arg_newPos - frontDir * m_refPlayer->WALL_JUMP_LENGTH;
		m_isHitCliff[static_cast<int>(RAY_DIR_ID::BEHIND)] |= CastRay(arg_newPos, rayPos, -m_refPlayer->m_transform.GetUp(), m_checkUnderRayLength, arg_castRayArgment, RAY_ID::CHECK_CLIFF);

	}

}

void PlayerCollision::CheckHitCliff_SearchImpactPoint(KuroEngine::Vec3<float>& arg_newPos, const KuroEngine::Vec3<float>& arg_from, PlayerCollision::CastRayArgument& arg_castRayArgment, std::weak_ptr<Stage> arg_nowStage)
{

	//プレイヤーの回転を考慮しない、法線情報だけを見た場合のトランスフォーム。
	KuroEngine::Transform localTrans;
	localTrans.SetRotate(m_refPlayer->m_normalSpinQ);

	//プレイヤーの回転を考慮しない回転行列からレイを飛ばす方向を取得する。
	KuroEngine::Vec3<float> rightDir = localTrans.GetRight();
	KuroEngine::Vec3<float> frontDir = localTrans.GetFront();

	//右側の周囲のレイが当たっていなかったら。
	if (!m_isHitCliff[static_cast<int>(RAY_DIR_ID::RIGHT)]) {

		KuroEngine::Vec3<float> rayPos = arg_newPos + rightDir * m_refPlayer->WALL_JUMP_LENGTH;
		if (CastRay(m_impactPointBuff, rayPos - m_refPlayer->m_transform.GetUp() * m_checkUnderRayLength, -rightDir, m_checkCliffRayLength, arg_castRayArgment, RAY_ID::CLIFF)) {
			m_impactPoint[static_cast<int>(RAY_DIR_ID::RIGHT)].emplace_back(m_impactPointBuff);
		}

	}

	//右側の周囲のレイが当たっていなかったら。
	if (!m_isHitCliff[static_cast<int>(RAY_DIR_ID::LEFT)]) {

		KuroEngine::Vec3<float> rayPos = arg_newPos - rightDir * m_refPlayer->WALL_JUMP_LENGTH;
		if (CastRay(m_impactPointBuff, rayPos - m_refPlayer->m_transform.GetUp() * m_checkUnderRayLength, rightDir, m_checkCliffRayLength, arg_castRayArgment, RAY_ID::CLIFF)) {
			m_impactPoint[static_cast<int>(RAY_DIR_ID::LEFT)].emplace_back(m_impactPointBuff);
		}

	}


	//右側の周囲のレイが当たっていなかったら。
	if (!m_isHitCliff[static_cast<int>(RAY_DIR_ID::FRONT)]) {

		KuroEngine::Vec3<float> rayPos = arg_newPos + frontDir * m_refPlayer->WALL_JUMP_LENGTH;
		if (CastRay(m_impactPointBuff, rayPos - m_refPlayer->m_transform.GetUp() * m_checkUnderRayLength, -frontDir, m_checkCliffRayLength, arg_castRayArgment, RAY_ID::CLIFF)) {
			m_impactPoint[static_cast<int>(RAY_DIR_ID::FRONT)].emplace_back(m_impactPointBuff);
		}

	}

	//右側の周囲のレイが当たっていなかったら。
	if (!m_isHitCliff[static_cast<int>(RAY_DIR_ID::BEHIND)]) {

		KuroEngine::Vec3<float> rayPos = arg_newPos - frontDir * m_refPlayer->WALL_JUMP_LENGTH;
		if (CastRay(m_impactPointBuff, rayPos - m_refPlayer->m_transform.GetUp() * m_checkUnderRayLength, frontDir, m_checkCliffRayLength, arg_castRayArgment, RAY_ID::CLIFF)) {
			m_impactPoint[static_cast<int>(RAY_DIR_ID::BEHIND)].emplace_back(m_impactPointBuff);
		}

	}

}

void PlayerCollision::CheckHitDeath_Around(KuroEngine::Vec3<float>& arg_newPos, const KuroEngine::Vec3<float>& arg_from, PlayerCollision::CastRayArgument& arg_castRayArgment, std::weak_ptr<Stage> arg_nowStage)
{

	const float DEATH_LENGTH = 1.0f;

	//スプラフェンスは死亡判定を行わない。
	if (arg_castRayArgment.m_stageType == StageParts::SPLATOON_FENCE) return;

	//右方向にレイを飛ばす。
	CastRay(arg_newPos, arg_newPos, m_refPlayer->m_transform.GetRight(), DEATH_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::RIGHT);

	//左方向にレイを飛ばす。
	CastRay(arg_newPos, arg_newPos, -m_refPlayer->m_transform.GetRight(), DEATH_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::LEFT);

	//後ろ方向にレイを飛ばす。
	CastRay(arg_newPos, arg_newPos, -m_refPlayer->m_transform.GetFront(), DEATH_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::BEHIND);

	//正面方向にレイを飛ばす。
	CastRay(arg_newPos, arg_newPos, m_refPlayer->m_transform.GetFront(), DEATH_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::FRONT);

	//上方向にレイを飛ばす。
	CastRay(arg_newPos, arg_newPos, m_refPlayer->m_transform.GetUp(), DEATH_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::TOP);

	CastRay(arg_newPos, arg_newPos, -m_refPlayer->m_transform.GetUp(), DEATH_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::BOTTOM);

}

void PlayerCollision::CheckHitAround_Around(KuroEngine::Vec3<float>& arg_newPos, const KuroEngine::Vec3<float>& arg_from, PlayerCollision::CastRayArgument& arg_castRayArgment, std::weak_ptr<Stage> arg_nowStage)
{

	//プレイヤーの回転を考慮しない、法線情報だけを見た場合のトランスフォーム。
	KuroEngine::Transform localTrans;
	localTrans.SetRotate(m_refPlayer->m_normalSpinQ);

	//プレイヤーの回転を考慮しない回転行列からレイを飛ばす方向を取得する。
	KuroEngine::Vec3<float> rightDir = localTrans.GetRight();
	KuroEngine::Vec3<float> frontDir = localTrans.GetFront();

	//右方向にレイを飛ばす。これは壁にくっつく用。
	arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::RIGHT)] |= CastRay(arg_newPos, arg_from, rightDir, m_refPlayer->WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

	//左方向にレイを飛ばす。これは壁にくっつく用。
	arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::LEFT)] |= CastRay(arg_newPos, arg_from, -rightDir, m_refPlayer->WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

	//後ろ方向にレイを飛ばす。これは壁にくっつく用。
	arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::BEHIND)] |= CastRay(arg_newPos, arg_from, -frontDir, m_refPlayer->WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

	//正面方向にレイを飛ばす。これは壁にくっつく用。
	arg_castRayArgment.m_checkHitAround[static_cast<int>(RAY_DIR_ID::FRONT)] |= CastRay(arg_newPos, arg_from, frontDir, m_refPlayer->WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

}

template<typename Func>
inline void PlayerCollision::CheckHitAllObject(Func arg_func, KuroEngine::Vec3<float>& arg_newPos, const KuroEngine::Vec3<float>& arg_from, PlayerCollision::CastRayArgument& arg_castRayArgment, std::weak_ptr<Stage> arg_nowStage)
{
	//地形配列走査
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//モデル情報取得
		auto model = terrian.GetModel().lock();
		//情報を取得。
		arg_castRayArgment.m_stageType = StageParts::TERRIAN;

		//メッシュを走査
		for (auto& modelMesh : model->m_meshes)
		{
			//メッシュ情報取得
			auto& mesh = modelMesh.mesh;

			//CastRayに渡す引数を更新。
			arg_castRayArgment.m_mesh = terrian.GetCollisionMesh()[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//判定↓============================================

			//当たり判定を実行
			(this->*arg_func)(arg_newPos, arg_from, arg_castRayArgment, arg_nowStage);

			//=================================================
		}
	}

	//動く足場との当たり判定
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//動く足場でない
		if (terrian->GetType() != StageParts::MOVE_SCAFFOLD)continue;

		//距離によってカリング
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 100.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

		//動く足場としてキャスト
		auto moveScaffold = dynamic_pointer_cast<MoveScaffold>(terrian);

		//モデル情報取得
		auto model = terrian->GetModel();
		//情報を取得。
		arg_castRayArgment.m_stageType = terrian->GetType();
		//ステージ情報を保存。
		arg_castRayArgment.m_stage = terrian;

		//メッシュを走査
		for (auto& modelMesh : model.lock()->m_meshes)
		{
			//メッシュ情報取得
			auto& mesh = modelMesh.mesh;

			//CastRayに渡す引数を更新。
			arg_castRayArgment.m_mesh = moveScaffold->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//判定↓============================================

			//当たり判定を実行
			(this->*arg_func)(arg_newPos, arg_from, arg_castRayArgment, arg_nowStage);

			//=================================================
		}
	}

	//蔦ブロックとの当たり判定
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//動く足場でない
		if (terrian->GetType() != StageParts::IVY_BLOCK)continue;

		//距離によってカリング
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 5.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

		//動く足場としてキャスト
		auto ivyBlock = dynamic_pointer_cast<IvyBlock>(terrian);

		//出現状態じゃなかったら処理を飛ばす。
		if (!ivyBlock->GetIsAppear()) continue;

		//モデル情報取得
		auto model = terrian->GetModel();
		//情報を取得。
		arg_castRayArgment.m_stageType = terrian->GetType();
		//ステージ情報を保存。
		arg_castRayArgment.m_stage = terrian;

		//メッシュを走査
		for (auto& modelMesh : model.lock()->m_meshes)
		{
			//メッシュ情報取得
			auto& mesh = modelMesh.mesh;

			//CastRayに渡す引数を更新。
			arg_castRayArgment.m_mesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//判定↓============================================

			//当たり判定を実行
			(this->*arg_func)(arg_newPos, arg_from, arg_castRayArgment, arg_nowStage);

			//=================================================
		}
	}

	//動けない壁との当たり判定
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//動く足場でない
		if (terrian->GetType() != StageParts::APPEARANCE)continue;

		//動く足場としてキャスト
		auto ivyBlock = dynamic_pointer_cast<Appearance>(terrian);

		//モデル情報取得
		auto model = terrian->GetModel();
		//情報を取得。
		arg_castRayArgment.m_stageType = terrian->GetType();
		//ステージ情報を保存。
		arg_castRayArgment.m_stage = terrian;

		//メッシュを走査
		for (auto& modelMesh : model.lock()->m_meshes)
		{
			//メッシュ情報取得
			auto& mesh = modelMesh.mesh;

			//CastRayに渡す引数を更新。
			arg_castRayArgment.m_mesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//判定↓============================================

			//当たり判定を実行
			(this->*arg_func)(arg_newPos, arg_from, arg_castRayArgment, arg_nowStage);

			//=================================================
		}
	}

	//プレイヤーが潜っていたら無視
	if (m_refPlayer->GetIsUnderGround()) return;

	//フェンスとの当たり判定
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//動く足場でない
		if (terrian->GetType() != StageParts::SPLATOON_FENCE)continue;

		//距離によってカリング
		const float DEADLINE = terrian->GetTransform().GetScale().Length() * 2.0f;
		float distance = KuroEngine::Vec3<float>(terrian->GetTransform().GetPosWorld() - m_refPlayer->m_transform.GetPosWorld()).Length();
		if (DEADLINE < distance) continue;

		//動く足場としてキャスト
		auto ivyBlock = dynamic_pointer_cast<SplatoonFence>(terrian);

		//モデル情報取得
		auto model = terrian->GetModel();
		//情報を取得。
		arg_castRayArgment.m_stageType = terrian->GetType();
		//ステージ情報を保存。
		arg_castRayArgment.m_stage = terrian;

		//メッシュを走査
		for (auto& modelMesh : model.lock()->m_meshes)
		{
			//メッシュ情報取得
			auto& mesh = modelMesh.mesh;

			//CastRayに渡す引数を更新。
			arg_castRayArgment.m_mesh = ivyBlock->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//判定↓============================================

			//当たり判定を実行
			(this->*arg_func)(arg_newPos, arg_from, arg_castRayArgment, arg_nowStage);

			//=================================================
		}
	}

}