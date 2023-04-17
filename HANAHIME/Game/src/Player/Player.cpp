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

void Player::OnImguiItems()
{
	using namespace KuroEngine;

	//トランスフォーム
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Transform"))
	{
		auto pos = m_transform.GetPos();
		auto angle = m_transform.GetRotateAsEuler();

		if (ImGui::DragFloat3("Position", (float*)&pos, 0.5f))
		{
			m_transform.SetPos(pos);
		}

		//操作しやすいようにオイラー角に変換
		KuroEngine::Vec3<float>eular = { angle.x.GetDegree(),angle.y.GetDegree(),angle.z.GetDegree() };
		if (ImGui::DragFloat3("Eular", (float*)&eular, 0.5f))
		{
			m_transform.SetRotate(Angle::ConvertToRadian(eular.x), Angle::ConvertToRadian(eular.y), Angle::ConvertToRadian(eular.z));
		}
		ImGui::TreePop();

		//前ベクトル
		auto front = m_transform.GetFront();
		ImGui::Text("Front : %.2f ,%.2f , %.2f", front.x, front.y, front.z);

		//上ベクトル
		auto up = m_transform.GetUp();
		ImGui::Text("Up : %.2f ,%.2f , %.2f", up.x, up.y, up.z);

		ImGui::Text("OnGround : %d", m_onGround);

	}

	//移動
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Move")) {

		ImGui::DragFloat("MoveAccel", &m_moveAccel, 0.01f);
		ImGui::DragFloat("MaxSpeed", &m_maxSpeed, 0.01f);
		ImGui::DragFloat("Brake", &m_brake, 0.01f);

		ImGui::TreePop();
	}

	//カメラ
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Camera"))
	{
		// ImGui::DragFloat3("Target", (float*)&target, 0.5f);
		ImGui::DragFloat("Sensitivity", &m_camSensitivity, 0.05f);

		ImGui::TreePop();
	}
}

bool Player::HitCheckAndPushBack(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo)
{
	/*
	arg_from … 移動前の座標
	arg_to … 移動後の座標
	arg_terrianArray … 地形の配列
	arg_terrianNormal … 当たった地形のメッシュの法線、格納先
	*/

	//CastRayに渡す引数
	Player::CastRayArgument castRayArgument;
	castRayArgument.m_onGround = false;
	castRayArgument.m_stageType = StageParts::TERRIAN;
	for (auto& index : castRayArgument.m_checkDeathCounter) {
		index = 0;
	}

	//ギミックに当たっているかどうかの変数を初期化
	m_prevOnGimmick = m_onGimmick;
	m_onGimmick = false;

	//周囲の壁との当たり判定
	CheckHitAround(arg_from, arg_newPos, arg_nowStage, arg_hitInfo, castRayArgument);

	//ジャンプ中は地面との当たり判定を行わない(ジャンプ先の上ベクトルが今の地面の上ベクトルになってしまうため)
	if (m_playerMoveStatus == PLAYER_MOVE_STATUS::JUMP) return true;

	//地面との当たり判定
	CheckHitGround(arg_from, arg_newPos, arg_nowStage, arg_hitInfo, castRayArgument);

	//死んだか(挟まっているか)どうかを判定
	CheckDeath(arg_from, arg_newPos, arg_nowStage, arg_hitInfo, castRayArgument);

	//死んでいたら処理を飛ばす。
	m_isDeath = false;
	m_isDeath |= (castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::RIGHT)] && castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::LEFT)]);
	m_isDeath |= (castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::TOP)] && castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::BOTTOM)]);
	m_isDeath |= (castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::FRONT)] && castRayArgument.m_checkDeathCounter[static_cast<int>(RAY_DIR_ID::BEHIND)]);
	if (m_isDeath) {
		return false;
	}

	return true;
}

void Player::CheckDeath(const KuroEngine::Vec3<float> arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, Player::CastRayArgument& arg_castRayArgment)
{

	//地形配列走査
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//モデル情報取得
		auto model = terrian.m_model.lock();
		//情報を取得。
		arg_castRayArgment.m_stageType = StageParts::TERRIAN;

		//メッシュを走査
		for (auto& modelMesh : model->m_meshes)
		{
			//メッシュ情報取得
			auto& mesh = modelMesh.mesh;

			//CastRayに渡す引数を更新。
			arg_castRayArgment.m_mesh = terrian.m_collisionMesh[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//判定↓============================================

			//右方向にレイを飛ばす。
			CastRay(arg_newPos, arg_newPos, m_transform.GetRight(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::RIGHT);

			//左方向にレイを飛ばす。
			CastRay(arg_newPos, arg_newPos, -m_transform.GetRight(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::LEFT);

			//後ろ方向にレイを飛ばす。
			CastRay(arg_newPos, arg_newPos, -m_transform.GetFront(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::BEHIND);

			//正面方向にレイを飛ばす。
			CastRay(arg_newPos, arg_newPos, m_transform.GetFront(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::FRONT);

			//上方向にレイを飛ばす。
			CastRay(arg_newPos, arg_newPos, m_transform.GetUp(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::TOP);

			//下方向にレイを飛ばす。
			CastRay(arg_newPos, arg_newPos, -m_transform.GetUp(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::BOTTOM);

			//=================================================
		}
	}

	//ギミックとの当たり判定
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
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
			arg_castRayArgment.m_mesh = terrian->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//判定↓============================================

			//移動した床がプレイヤーから離れる方向に動いていたら死亡判定を飛ばす。
			if (terrian->GetType() == StageParts::MOVE_SCAFFOLD) {

				float oldPosDistance = (arg_newPos - terrian->GetOldPos()).Length();
				float nowPosDistance = (arg_newPos - terrian->GetNowPos()).Length();

				if (oldPosDistance < nowPosDistance) {
					continue;
				}

			}

			//右方向にレイを飛ばす。
			CastRay(arg_newPos, arg_newPos, m_transform.GetRight(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::RIGHT);

			//左方向にレイを飛ばす。
			CastRay(arg_newPos, arg_newPos, -m_transform.GetRight(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::LEFT);

			//後ろ方向にレイを飛ばす。
			CastRay(arg_newPos, arg_newPos, -m_transform.GetFront(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::BEHIND);

			//正面方向にレイを飛ばす。
			CastRay(arg_newPos, arg_newPos, m_transform.GetFront(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::FRONT);

			//上方向にレイを飛ばす。
			CastRay(arg_newPos, arg_newPos, m_transform.GetUp(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::TOP);

			//下方向にレイを飛ばす。
			CastRay(arg_newPos, arg_newPos, -m_transform.GetUp(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::CHECK_DEATH, RAY_DIR_ID::BOTTOM);

			//=================================================
		}
	}

}

void Player::CheckHitAround(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, Player::CastRayArgument& arg_castRayArgment) {

	//地形配列走査
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//モデル情報取得
		auto model = terrian.m_model.lock();
		//情報を取得。
		arg_castRayArgment.m_stageType = StageParts::TERRIAN;

		//メッシュを走査
		for (auto& modelMesh : model->m_meshes)
		{
			//メッシュ情報取得
			auto& mesh = modelMesh.mesh;

			//CastRayに渡す引数を更新。
			arg_castRayArgment.m_mesh = terrian.m_collisionMesh[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//判定↓============================================

			//右方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_newPos, arg_newPos, m_transform.GetRight(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//左方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_newPos, arg_newPos, -m_transform.GetRight(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//後ろ方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_newPos, arg_newPos, -m_transform.GetFront(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//正面方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_newPos, arg_newPos, m_transform.GetFront(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//=================================================
		}
	}

	//ギミックとの当たり判定
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
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
			arg_castRayArgment.m_mesh = terrian->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//判定↓============================================

			//右方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_newPos, arg_newPos, m_transform.GetRight(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//左方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_newPos, arg_newPos, -m_transform.GetRight(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//後ろ方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_newPos, arg_newPos, -m_transform.GetFront(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//正面方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_newPos, arg_newPos, m_transform.GetFront(), WALL_JUMP_LENGTH, arg_castRayArgment, RAY_ID::AROUND);

			//=================================================
		}
	}

	//周囲の衝突点があったら、それの最短距離を求めてジャンプ先を決める。
	int impactPointSize = static_cast<int>(arg_castRayArgment.m_impactPoint.size());
	if (0 < impactPointSize) {

		//全ての衝突点について崖を超えていないかをチェックする。
		for (auto& index : arg_castRayArgment.m_impactPoint) {

			//崖チェック
			CheckCliff(index, arg_nowStage);

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

			//ジャンプができる状態だったらジャンプする。
			if (m_canJump) {

				//最短の衝突点を求めたら、それをジャンプ先にする。
				arg_hitInfo->m_terrianNormal = arg_castRayArgment.m_impactPoint[minIndex].m_normal;

				//ジャンプのパラメーターも決める。
				m_playerMoveStatus = PLAYER_MOVE_STATUS::JUMP;
				m_jumpTimer = 0;
				m_jumpStartPos = arg_newPos;
				m_bezierCurveControlPos = m_jumpStartPos + m_transform.GetUp() * WALL_JUMP_LENGTH;
				m_jumpEndPos = arg_castRayArgment.m_impactPoint[minIndex].m_impactPos + arg_castRayArgment.m_impactPoint[minIndex].m_normal * m_transform.GetScale().x;
				m_jumpEndPos += m_transform.GetUp() * WALL_JUMP_LENGTH;

				//ジャンプしたのでタイマーを初期化
				m_canJumpDelayTimer = 0;

			}
			//ジャンプができない状態だったら押し戻す。
			else {

				arg_newPos = arg_from + m_gimmickVel;
				arg_hitInfo->m_terrianNormal = m_transform.GetUp();

				//引っ掛かりのタイマーを更新 この値が一定値をこえたらジャンプできるようになる
				++m_canJumpDelayTimer;

			}

			return;

		}

	}



	//どことも当たっていなかったら現在の上ベクトルを地形の上ベクトルとしてみる。
	arg_hitInfo->m_terrianNormal = m_transform.GetUp();

	//どこにも引っかかってないのでタイマーを初期化する。
	m_canJumpDelayTimer = 0;


}

void Player::CheckHitGround(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, std::weak_ptr<Stage> arg_nowStage, HitCheckResult* arg_hitInfo, Player::CastRayArgument& arg_castRayArgment) {

	//地形配列走査
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//モデル情報取得
		auto model = terrian.m_model.lock();
		//情報を取得。
		arg_castRayArgment.m_stageType = StageParts::TERRIAN;

		//メッシュを走査
		for (auto& modelMesh : model->m_meshes)
		{
			//メッシュ情報取得
			auto& mesh = modelMesh.mesh;

			//CastRayに渡す引数を更新。
			arg_castRayArgment.m_mesh = terrian.m_collisionMesh[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//判定↓============================================

			//下方向にレイを飛ばす。これは地面との押し戻し用。
			CastRay(arg_newPos, arg_newPos, -m_transform.GetUp(), m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);

			//=================================================
		}
	}
	//ギミックとの当たり判定
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
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
			arg_castRayArgment.m_mesh = terrian->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//判定↓============================================

			//プレイヤーが動いた方向にレイを飛ばす位置をずらす。
			KuroEngine::Vec3<float> moveVec = (arg_newPos - arg_from);

			//下方向にレイを飛ばす。これは地面との押し戻し用。
			CastRay(arg_newPos, arg_newPos, -m_transform.GetUp(), m_transform.GetScale().y, arg_castRayArgment, RAY_ID::GROUND);

			//ずらした分元に戻す。
			arg_newPos - moveVec;

			//=================================================
		}
	}

	//接地フラグを保存
	m_prevOnGround = m_onGround;
	m_onGround = arg_castRayArgment.m_onGround;

	//まだ着地していなかったら着地にする。
	if (!m_isFirstOnGround && m_onGround) {
		m_isFirstOnGround = true;
	}

	//接地していなかったら移動前の位置に戻す。
	if (m_isFirstOnGround && !m_onGround) {
		//ギミックの上にいたらそれも考慮した位置に戻す。
		if (0 < m_gimmickVel.Length()) {
			arg_newPos = arg_from + m_gimmickVel;
		}
		else {
			arg_newPos = arg_from;
		}
		//前フレームでギミックの上にいたら、今もギミックにいる判定にする。(そうしないと移動のギミックの上にいるときに壁際に行くと一瞬降りた判定になり、ギミックが稼働してしまうから。)
		if (m_prevOnGimmick) m_onGimmick = true;
		//次に補間する上ベクトルとプレイヤーの上ベクトルにする。
		arg_hitInfo->m_terrianNormal = m_transform.GetUp();
	}

}

void Player::CheckCliff(Player::ImpactPointData& arg_impactPointData, std::weak_ptr<Stage> arg_nowStage)
{

	//有効化されていなかったら処理を飛ばす。
	if (!arg_impactPointData.m_isActive) return;

	//崖判定用のレイの長さ
	const float CLIFF_RAY_LENGTH = WALL_JUMP_LENGTH + m_transform.GetScale().Length();

	//地形配列走査
	for (auto& terrian : arg_nowStage.lock()->GetTerrianArray())
	{
		//モデル情報取得
		auto model = terrian.m_model.lock();

		//メッシュを走査
		for (auto& modelMesh : model->m_meshes)
		{

			//判定↓============================================

			//当たり判定を行うメッシュ。
			std::vector<StageParts::Polygon> mesh = terrian.m_collisionMesh[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//下方向にレイを飛ばす。
			MeshCollisionOutput output = MeshCollision(arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * 0.5f, -m_transform.GetUp(), mesh);

			//レイがメッシュに衝突しており、衝突地点までの距離がレイの長さより小さかったら衝突している。
			if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

				//壁に当たった時点で崖ではないので処理を飛ばす。
				return;

			}

			//=================================================
		}
	}

	//ギミックとの当たり判定
	for (auto& terrian : arg_nowStage.lock()->GetGimmickArray())
	{
		//モデル情報取得
		auto model = terrian->GetModel();

		//メッシュを走査
		for (auto& modelMesh : model.lock()->m_meshes) {

			//判定↓============================================

			//当たり判定を行うメッシュ。
			std::vector<StageParts::Polygon> mesh = terrian->GetCollisionMesh()[static_cast<int>(&modelMesh - &model.lock()->m_meshes[0])];

			//下方向にレイを飛ばす。
			MeshCollisionOutput output = MeshCollision(arg_impactPointData.m_impactPos + arg_impactPointData.m_normal * 0.5f, -m_transform.GetUp(), mesh);

			//レイがメッシュに衝突しており、衝突地点までの距離がレイの長さより小さかったら衝突している。
			if (output.m_isHit && std::fabs(output.m_distance) < CLIFF_RAY_LENGTH) {

				//壁に当たった時点で崖ではないので処理を飛ばす。
				return;

			}

			//=================================================
		}
	}

	//最後まで壁に当たってなかったら崖を超えているので無効化する。
	arg_impactPointData.m_isActive = false;

}

Player::Player()
	:KuroEngine::Debugger("Player", true, true)
{
	AddCustomParameter("Sensitivity", { "camera", "sensitivity" }, PARAM_TYPE::FLOAT, &m_camSensitivity, "Camera");
	LoadParameterLog();

	//モデル読み込み
	m_model = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Player.glb");
	m_axisModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Axis.glb");
	m_camModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Camera.glb");

	//カメラ生成
	m_cam = std::make_shared<KuroEngine::Camera>("Player's Camera");
	//カメラのコントローラーにアタッチ
	m_camController.AttachCamera(m_cam);

	m_cameraRotY = 0;
	m_cameraQ = DirectX::XMQuaternionIdentity();

	m_moveSpeed = KuroEngine::Vec3<float>();
	m_isFirstOnGround = false;
	m_onGimmick = false;
	m_prevOnGimmick = false;

}

void Player::Init(KuroEngine::Transform arg_initTransform)
{
	m_transform = arg_initTransform;
	m_camController.Init();
	m_cameraRotY = 0;
	m_cameraRotYStorage = 0;
	m_cameraJumpLerpAmount = 0;
	m_cameraJumpLerpStorage = 0;
	m_cameraQ = DirectX::XMQuaternionIdentity();
	m_canJumpDelayTimer = 0;

	m_moveSpeed = KuroEngine::Vec3<float>();
	m_gimmickVel = KuroEngine::Vec3<float>();
	m_isFirstOnGround = false;
	m_onGimmick = false;
	m_isCameraModeFar = true;
	m_prevOnGimmick = false;
	m_isDeath = false;
	m_playerMoveStatus = PLAYER_MOVE_STATUS::MOVE;
}

void Player::Update(const std::weak_ptr<Stage>arg_nowStage)
{
	using namespace KuroEngine;

	//トランスフォームを保存。
	m_prevTransform = m_transform;

	//位置情報関係
	auto beforePos = m_transform.GetPos();
	auto newPos = beforePos;

	//入力された視線移動角度量を取得
	auto scopeMove = OperationConfig::Instance()->GetScopeMove();

	//ジャンプができるかどうか。	一定時間地形に引っ掛かってたらジャンプできる。
	m_canJump = CAN_JUMP_DELAY <= m_canJumpDelayTimer;

	//カメラモードを切り替える。
	if (UsersInput::Instance()->KeyOffTrigger(DIK_RETURN) || UsersInput::Instance()->ControllerOnTrigger(0, KuroEngine::X)) {
		m_isCameraModeFar = m_isCameraModeFar ? false : true;
	}

	//移動ステータスによって処理を変える。
	switch (m_playerMoveStatus)
	{
	case Player::PLAYER_MOVE_STATUS::MOVE:
	{

		//カメラの回転を保存。
		m_cameraRotYStorage += scopeMove.x;

		//プレイヤーの回転をカメラ基準にする。(移動方向の基準がカメラの角度なため)
		m_transform.SetRotate(m_cameraQ);

		//入力された移動量を取得
		m_rowMoveVec = OperationConfig::Instance()->GetMoveVecFuna(XMQuaternionIdentity());	//生の入力方向を取得。プレイヤーを入力方向に回転させる際に、XZ平面での値を使用したいから。

		//入力量が一定以下だったら0にする。
		const float DEADLINE = 0.8f;
		if (m_rowMoveVec.Length() <= DEADLINE) {
			m_rowMoveVec = {};
		}

		//天井にいたら
		if (m_transform.GetUp().y < -0.9f) {
			//Xの移動方向を反転
			m_rowMoveVec.x *= -1.0f;
		}

		//移動させる。
		Move(newPos);

		//入力がなかったら
		if (m_rowMoveVec.Length() <= 0) {

			//カメラの回転を保存。
			m_cameraRotY = m_cameraRotYStorage;

		}
		else {

			//移動した方向を保存。
			m_playerRotY = atan2f(m_rowMoveVec.x, m_rowMoveVec.z);

		}

		//当たり判定
		CheckHit(beforePos, newPos, arg_nowStage);

	}
	break;
	case Player::PLAYER_MOVE_STATUS::JUMP:
	{

		//タイマーを更新。
		m_jumpTimer = std::clamp(m_jumpTimer + JUMP_TIMER, 0.0f, 1.0f);

		float easeAmount = KuroEngine::Math::Ease(Out, Sine, m_jumpTimer, 0.0f, 1.0f);

		//カメラの回転を補間する。
		m_cameraRotYStorage = m_cameraJumpLerpStorage + easeAmount * m_cameraJumpLerpAmount;
		m_cameraRotY = m_cameraJumpLerpStorage + easeAmount * m_cameraJumpLerpAmount;

		//座標を補間する。
		newPos = CalculateBezierPoint(easeAmount, m_jumpStartPos, m_jumpEndPos, m_bezierCurveControlPos);

		//回転を補完する。
		m_transform.SetRotate(DirectX::XMQuaternionSlerp(m_jumpStartQ, m_jumpEndQ, easeAmount));

		//上限に達していたらジャンプを終える。
		if (1.0f <= m_jumpTimer) {
			m_playerMoveStatus = PLAYER_MOVE_STATUS::MOVE;
			m_cameraJumpLerpAmount = 0;
		}

	}
	break;
	default:
		break;
	}


	//座標変化適用
	m_transform.SetPos(newPos);
	m_ptLig.SetPos(newPos);

	//カメラ操作
	m_camController.Update(scopeMove, m_transform.GetPosWorld(), m_cameraRotYStorage, m_isCameraModeFar ? CAMERA_MODE_FAR : CAMERA_MODE_NEAR);

	//ギミックの移動を打ち消す。
	m_gimmickVel = KuroEngine::Vec3<float>();
}

void Player::Draw(KuroEngine::Camera& arg_cam, KuroEngine::LightManager& arg_ligMgr, bool arg_cameraDraw)
{
	/*
	KuroEngine::DrawFunc3D::DrawNonShadingModel(
		m_model,
		m_transform,
		arg_cam);
	*/

	static IndividualDrawParameter drawParam = IndividualDrawParameter::GetDefault();
	drawParam.m_isPlayer = 1;

	BasicDraw::Instance()->Draw_Player(
		arg_cam,
		arg_ligMgr,
		m_model,
		m_transform,
		drawParam);

	/*
	KuroEngine::DrawFunc3D::DrawNonShadingModel(
		m_axisModel,
		m_transform,
		arg_cam);
	*/

	if (arg_cameraDraw)
	{
		auto camTransform = m_cam->GetTransform();
		KuroEngine::DrawFunc3D::DrawNonShadingModel(
			m_camModel,
			camTransform.GetMatWorld(),
			camTransform.GetPos().z,
			arg_cam);
	}
}

void Player::Finalize()
{
}

Player::MeshCollisionOutput Player::MeshCollision(const KuroEngine::Vec3<float>& arg_rayPos, const KuroEngine::Vec3<float>& arg_rayDir, std::vector<Terrian::Polygon>& arg_targetMesh) {


	/*===== メッシュとレイの当たり判定 =====*/


	/*-- ① ポリゴンを法線情報をもとにカリングする --*/

	//法線とレイの方向の内積が0より大きかった場合、そのポリゴンは背面なのでカリングする。
	for (auto& index : arg_targetMesh) {

		index.m_isActive = true;

		if (index.m_p1.normal.Dot(arg_rayDir) < -0.0001f) continue;

		index.m_isActive = false;

	}


	/*-- ② ポリゴンとレイの当たり判定を行い、各情報を記録する --*/

	// 記録用データ
	std::vector<std::pair<Player::MeshCollisionOutput, Terrian::Polygon>> hitDataContainer;

	for (auto& index : arg_targetMesh) {

		//ポリゴンが無効化されていたら次の処理へ
		if (!index.m_isActive) continue;

		//レイの開始地点から平面におろした垂線の長さを求める
		//KuroEngine::Vec3<float> planeNorm = -index.m_p0.normal;
		KuroEngine::Vec3<float> planeNorm = KuroEngine::Vec3<float>(KuroEngine::Vec3<float>(index.m_p0.pos - index.m_p2.pos).GetNormal()).Cross(KuroEngine::Vec3<float>(index.m_p0.pos - index.m_p1.pos).GetNormal());
		float rayToOriginLength = arg_rayPos.Dot(planeNorm);
		float planeToOriginLength = index.m_p0.pos.Dot(planeNorm);
		//視点から平面におろした垂線の長さ
		float perpendicularLine = rayToOriginLength - planeToOriginLength;

		//三角関数を利用して視点から衝突点までの距離を求める
		float dist = planeNorm.Dot(arg_rayDir);
		float impDistance = perpendicularLine / -dist;

		if (std::isnan(impDistance))continue;

		//衝突地点
		KuroEngine::Vec3<float> impactPoint = arg_rayPos + arg_rayDir * impDistance;

		/*----- 衝突点がポリゴンの内側にあるかを調べる -----*/

		/* 辺1本目 */
		KuroEngine::Vec3<float> P1ToImpactPos = (impactPoint - index.m_p0.pos).GetNormal();
		KuroEngine::Vec3<float> P1ToP2 = (index.m_p1.pos - index.m_p0.pos).GetNormal();
		KuroEngine::Vec3<float> P1ToP3 = (index.m_p2.pos - index.m_p0.pos).GetNormal();

		//衝突点と辺1の内積
		float impactDot = P1ToImpactPos.Dot(P1ToP2);
		//点1と点3の内積
		float P1Dot = P1ToP2.Dot(P1ToP3);

		//衝突点と辺1の内積が点1と点3の内積より小さかったらアウト
		if (impactDot < P1Dot) {
			index.m_isActive = false;
			continue;
		}

		/* 辺2本目 */
		KuroEngine::Vec3<float> P2ToImpactPos = (impactPoint - index.m_p1.pos).GetNormal();
		KuroEngine::Vec3<float> P2ToP3 = (index.m_p2.pos - index.m_p1.pos).GetNormal();
		KuroEngine::Vec3<float> P2ToP1 = (index.m_p0.pos - index.m_p1.pos).GetNormal();

		//衝突点と辺2の内積
		impactDot = P2ToImpactPos.Dot(P2ToP3);
		//点2と点1の内積
		float P2Dot = P2ToP3.Dot(P2ToP1);

		//衝突点と辺2の内積が点2と点1の内積より小さかったらアウト
		if (impactDot < P2Dot) {
			index.m_isActive = false;
			continue;
		}

		/* 辺3本目 */
		KuroEngine::Vec3<float> P3ToImpactPos = (impactPoint - index.m_p2.pos).GetNormal();
		KuroEngine::Vec3<float> P3ToP1 = (index.m_p0.pos - index.m_p2.pos).GetNormal();
		KuroEngine::Vec3<float> P3ToP2 = (index.m_p1.pos - index.m_p2.pos).GetNormal();

		//衝突点と辺3の内積
		impactDot = P3ToImpactPos.Dot(P3ToP1);
		//点3と点2の内積
		float P3Dot = P3ToP1.Dot(P3ToP2);

		//衝突点と辺3の内積が点3と点2の内積より小さかったらアウト
		if (impactDot < P3Dot) {
			index.m_isActive = false;
			continue;
		}

		/* ここまで来たらポリゴンに衝突してる！ */
		Player::MeshCollisionOutput data;
		data.m_isHit = true;
		data.m_pos = impactPoint;
		data.m_distance = impDistance;
		data.m_normal = index.m_p0.normal;
		hitDataContainer.emplace_back(std::pair(data, index));

	}


	/*-- ③ 記録した情報から最終的な衝突点を求める --*/

	//hitPorygonの値が1以上だったら距離が最小の要素を検索
	if (0 < hitDataContainer.size()) {

		//距離が最小の要素を検索
		int min = 0;
		float minDistance = std::numeric_limits<float>().max();
		for (auto& index : hitDataContainer) {
			if (fabs(index.first.m_distance) < fabs(minDistance)) {
				minDistance = index.first.m_distance;
				min = static_cast<int>(&index - &hitDataContainer[0]);
			}
		}

		//重心座標を求める。
		KuroEngine::Vec3<float> bary = CalBary(hitDataContainer[min].second.m_p0.pos, hitDataContainer[min].second.m_p1.pos, hitDataContainer[min].second.m_p2.pos, hitDataContainer[min].first.m_pos);

		KuroEngine::Vec3<float> baryBuff = bary;

		//UVWの値がずれるので修正。
		bary.x = baryBuff.y;
		bary.y = baryBuff.z;
		bary.z = baryBuff.x;

		KuroEngine::Vec2<float> uv = KuroEngine::Vec2<float>();

		//重心座標からUVを求める。
		uv.x += hitDataContainer[min].second.m_p0.uv.x * bary.x;
		uv.x += hitDataContainer[min].second.m_p1.uv.x * bary.y;
		uv.x += hitDataContainer[min].second.m_p2.uv.x * bary.z;

		uv.y += hitDataContainer[min].second.m_p0.uv.y * bary.x;
		uv.y += hitDataContainer[min].second.m_p1.uv.y * bary.y;
		uv.y += hitDataContainer[min].second.m_p2.uv.y * bary.z;

		hitDataContainer[min].first.m_uv = uv;

		return hitDataContainer[min].first;
	}
	else {

		return Player::MeshCollisionOutput();

	}


}

KuroEngine::Vec3<float> Player::CalBary(const KuroEngine::Vec3<float>& PosA, const KuroEngine::Vec3<float>& PosB, const KuroEngine::Vec3<float>& PosC, const KuroEngine::Vec3<float>& TargetPos)
{

	/*===== 重心座標を求める =====*/

	KuroEngine::Vec3<float> uvw = KuroEngine::Vec3<float>();

	// 三角形の面積を求める。
	float areaABC = (PosC - PosA).Cross(PosB - PosA).Length() / 2.0f;

	// 重心座標を求める。
	uvw.x = ((PosA - TargetPos).Cross(PosB - TargetPos).Length() / 2.0f) / areaABC;
	uvw.y = ((PosB - TargetPos).Cross(PosC - TargetPos).Length() / 2.0f) / areaABC;
	uvw.z = ((PosC - TargetPos).Cross(PosA - TargetPos).Length() / 2.0f) / areaABC;

	return uvw;

}

bool Player::CastRay(KuroEngine::Vec3<float>& arg_charaPos, const KuroEngine::Vec3<float>& arg_rayCastPos, const KuroEngine::Vec3<float>& arg_rayDir, float arg_rayLength, CastRayArgument& arg_collisionData, RAY_ID arg_rayID, RAY_DIR_ID arg_rayDirID)
{

	/*===== 当たり判定用のレイを撃つ =====*/

	//レイを飛ばす。
	MeshCollisionOutput output = MeshCollision(arg_rayCastPos, arg_rayDir, arg_collisionData.m_mesh);

	//レイがメッシュに衝突しており、衝突地点までの距離がレイの長さより小さかったら衝突している。
	if (output.m_isHit && std::fabs(output.m_distance) < arg_rayLength) {

		//ぴったり押し戻してしまうと重力の関係でガクガクしてしまうので、微妙にめり込ませて押し戻す。
		static const float OFFSET = 0.01f;

		//レイの種類によって保存するデータを変える。
		switch (arg_rayID)
		{
		case Player::RAY_ID::GROUND:

			//外部に渡す用のデータを保存
			arg_collisionData.m_bottomTerrianNormal = output.m_normal;
			arg_collisionData.m_onGround = true;

			//押し戻す。
			arg_charaPos += output.m_normal * (std::fabs(output.m_distance - arg_rayLength) - OFFSET);

			//地形が動く床だったら有効化する。
			if (arg_collisionData.m_stageType == StageParts::MOVE_SCAFFOLD) {

				//ギミックに当たっている判定
				m_onGimmick = true;

				//さらにギミックに当たったトリガーだったらギミックを有効化させる。
				if (!m_prevOnGimmick) {
					arg_collisionData.m_stage.lock()->Activate();
				}

			}

			break;

		case Player::RAY_ID::AROUND:

			//レイの衝突地点を保存。
			arg_collisionData.m_impactPoint.emplace_back(ImpactPointData(output.m_pos, output.m_normal));

			//動く床だったらめり込んでしまうので押し戻す。
			if (arg_collisionData.m_stageType == StageParts::MOVE_SCAFFOLD) {

				arg_charaPos += output.m_normal * (std::fabs(output.m_distance - arg_rayLength) - OFFSET);

			}

			break;

		case Player::RAY_ID::CHECK_DEATH:

			arg_collisionData.m_checkDeathCounter[static_cast<int>(arg_rayDirID)] = true;

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

void Player::Move(KuroEngine::Vec3<float>& arg_newPos) {

	//落下中は入力を無効化。
	if (!m_onGround) {
		m_rowMoveVec = KuroEngine::Vec3<float>();
	}
	m_moveSpeed = m_rowMoveVec * m_maxSpeed;

	//移動速度をクランプ。
	m_moveSpeed.x = std::clamp(m_moveSpeed.x, -m_maxSpeed, m_maxSpeed);
	m_moveSpeed.z = std::clamp(m_moveSpeed.z, -m_maxSpeed, m_maxSpeed);

	//入力された値が無かったら移動速度を減らす。
	if (std::fabs(m_rowMoveVec.x) < 0.001f) {

		m_moveSpeed.x = 0;

	}
	else {
		int a = 0;
	}

	if (std::fabs(m_rowMoveVec.z) < 0.001f) {

		m_moveSpeed.z = 0;

	}

	//ローカル軸の移動方向をプレイヤーの回転に合わせて動かす。
	auto moveAmount = KuroEngine::Math::TransformVec3(m_moveSpeed, m_transform.GetRotate());

	//移動量加算
	arg_newPos += moveAmount;

	//ギミックの移動量も加算。
	arg_newPos += m_gimmickVel;

	//地面に張り付ける用の重力。
	if (!m_onGround) {
		arg_newPos -= m_transform.GetUp() * (m_transform.GetScale().y / 2.0f);
	}

}

void Player::CheckHit(KuroEngine::Vec3<float>& arg_frompos, KuroEngine::Vec3<float>& arg_nowpos, std::weak_ptr<Stage>arg_nowStage) {

	HitCheckResult hitResult;
	if (!HitCheckAndPushBack(arg_frompos, arg_nowpos, arg_nowStage, &hitResult))return;

	//地形の法線が真下を向いているときに誤差できれいに0,-1,0になってくれないせいでうまくいかないので苦肉の策。
	if (hitResult.m_terrianNormal.y < -0.9f) {
		hitResult.m_terrianNormal = { 0,-1,0 };
	}

	//カメラを矯正する。
	AdjustCaneraRotY(m_transform.GetUp(), hitResult.m_terrianNormal);

	//法線方向を見るクォータニオン
	m_normalSpinQ = KuroEngine::Math::GetLookAtQuaternion({ 0,1,0 }, hitResult.m_terrianNormal);

	//カメラの回転でY軸回転させるクォータニオン。移動方向に回転しているように見せかけるためのもの。m_cameraJumpLerpAmountは補間後のカメラに向かって補間するため。
	DirectX::XMVECTOR ySpin;
	if (hitResult.m_terrianNormal.y < -0.9f) {
		ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, -(m_cameraRotY + m_cameraJumpLerpAmount) + DirectX::XM_PI);
	}
	else {
		ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_cameraRotY + m_cameraJumpLerpAmount);
	}

	//プレイヤーの移動方向でY軸回転させるクォータニオン。移動方向に回転しているように見せかけるためのもの。
	DirectX::XMVECTOR playerYSpin;
	playerYSpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_playerRotY);

	//カメラ方向でのクォータニオンを求める。進む方向などを判断するのに使用するのはこっち。Fの一番最初にこの値を入れることでplayerYSpinの回転を打ち消す。
	m_cameraQ = DirectX::XMQuaternionMultiply(m_normalSpinQ, ySpin);

	//プレイヤーの移動方向でY軸回転させるクォータニオンをカメラのクォータニオンにかけて、プレイヤーを移動方向に向かせる。
	m_moveQ = DirectX::XMQuaternionMultiply(m_cameraQ, playerYSpin);

	//ジャンプ状態だったら
	if (m_playerMoveStatus == PLAYER_MOVE_STATUS::JUMP) {

		//ジャンプ後に回転するようにする。

		//クォータニオンを保存。
		m_jumpEndQ = m_moveQ;
		m_jumpStartQ = m_prevTransform.GetRotate();
		m_transform.SetRotate(m_prevTransform.GetRotate());

	}
	else {

		//当たった面基準の回転にする。
		m_transform.SetRotate(m_moveQ);

	}

}

KuroEngine::Vec3<float> Player::CalculateBezierPoint(float arg_time, KuroEngine::Vec3<float> arg_startPoint, KuroEngine::Vec3<float> arg_endPoint, KuroEngine::Vec3<float> arg_controlPoint) {

	float oneMinusT = 1.0f - arg_time;
	float oneMinusTSquared = oneMinusT * oneMinusT;
	float tSquared = arg_time * arg_time;

	float x = oneMinusTSquared * arg_startPoint.x + 2 * oneMinusT * arg_time * arg_controlPoint.x + tSquared * arg_endPoint.x;
	float y = oneMinusTSquared * arg_startPoint.y + 2 * oneMinusT * arg_time * arg_controlPoint.y + tSquared * arg_endPoint.y;
	float z = oneMinusTSquared * arg_startPoint.z + 2 * oneMinusT * arg_time * arg_controlPoint.z + tSquared * arg_endPoint.z;

	return KuroEngine::Vec3<float>(x, y, z);

}

void Player::AdjustCaneraRotY(const KuroEngine::Vec3<float>& arg_nowUp, const KuroEngine::Vec3<float>& arg_nextUp) {

	// 移動方向を矯正するための苦肉の策

	// メモ:この関数で書いてある方向は初期位置(法線(0,1,0)で(0,0,1)を向いている状態)でのものです。

	//回転元の値を保存。
	m_cameraJumpLerpStorage = m_cameraRotYStorage;

	//角度が変わってなかったら飛ばす。
	if (0.9f <= arg_nowUp.Dot(arg_nextUp)) return;

	//プレイヤーが右側の壁にいる場合
	if (arg_nowUp.x <= -0.9f) {

		//上の壁に移動したら
		if (arg_nextUp.y <= -0.9f) {

			m_cameraJumpLerpAmount += DirectX::XM_PI;

		}
		//正面の壁に移動したら
		if (arg_nextUp.z <= -0.9f) {

			m_cameraJumpLerpAmount -= DirectX::XM_PIDIV2;

		}
		//後ろの壁に移動したら
		if (0.9f <= arg_nextUp.z) {

			m_cameraJumpLerpAmount += DirectX::XM_PIDIV2;

		}

	}

	//プレイヤーが左側の壁にいる場合
	if (0.9f <= arg_nowUp.x) {

		//上の壁に移動したら
		if (arg_nextUp.y <= -0.9f) {

			m_cameraJumpLerpAmount -= DirectX::XM_PI;

		}
		//正面の壁に移動したら
		if (arg_nextUp.z <= -0.9f) {

			m_cameraJumpLerpAmount += DirectX::XM_PIDIV2;

		}
		//後ろの壁に移動したら
		if (0.9f <= arg_nextUp.z) {

			m_cameraJumpLerpAmount -= DirectX::XM_PIDIV2;

		}

	}

	//プレイヤーが正面の壁にいる場合
	if (arg_nowUp.z <= -0.9f) {

		//右側の壁に移動したら
		if (arg_nextUp.x <= -0.9f) {

			m_cameraJumpLerpAmount += DirectX::XM_PIDIV2;

		}
		//左側の壁に移動したら
		if (0.9f <= arg_nextUp.x) {

			m_cameraJumpLerpAmount -= DirectX::XM_PIDIV2;

		}

	}

	//プレイヤーが後ろ側の壁にいる場合
	if (0.9f <= arg_nowUp.z) {

		//右側の壁に移動したら
		if (arg_nextUp.x <= -0.9f) {

			m_cameraJumpLerpAmount -= DirectX::XM_PIDIV2;

		}
		//左側の壁に移動したら
		if (0.9f <= arg_nextUp.x) {

			m_cameraJumpLerpAmount += DirectX::XM_PIDIV2;

		}

	}

	//プレイヤーが上側の壁にいる場合
	if (arg_nowUp.y <= -0.9f) {

		//右側の壁に移動したら
		if (arg_nextUp.x <= -0.9f) {

			m_cameraJumpLerpAmount -= DirectX::XM_PI;

		}
		//左側の壁に移動したら
		if (0.9f <= arg_nextUp.x) {

			m_cameraJumpLerpAmount += DirectX::XM_PI;

		}

	}

}