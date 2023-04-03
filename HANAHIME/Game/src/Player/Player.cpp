#include "Player.h"
#include"Render/RenderObject/Camera.h"
#include"../OperationConfig.h"
#include"FrameWork/Importer.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"../Graphics/BasicDraw.h"
#include"../Stage/Stage.h"
#include"../Graphics/BasicDrawParameters.h"

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

		ImGui::DragFloat("GrassPosScatter : X", &m_grassPosScatter.x, 0.1f);
		ImGui::DragFloat("GrassPosScatter : Y", &m_grassPosScatter.y, 0.1f);

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

bool Player::HitCheckAndPushBack(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_newPos, const std::vector<Terrian>& arg_terrianArray, HitCheckResult* arg_hitInfo)
{
	/*
	arg_from … 移動前の座標
	arg_to … 移動後の座標
	arg_terrianArray … 地形の配列
	arg_terrianNormal … 当たった地形のメッシュの法線、格納先
	*/

	//当たり判定結果
	bool isHitWall = false;
	bool onGround = false;
	HitCheckResult hitResult;

	//CastRayに渡す引数
	Player::CastRayArgument castRayArgument(onGround, isHitWall, hitResult);

	m_debugTransform.clear();

	//地形配列走査
	for (auto& terrian : arg_terrianArray)
	{
		//モデル情報取得
		auto model = terrian.m_model.lock();
		//トランスフォーム情報
		auto& transform = terrian.m_transform;
		castRayArgument.m_targetTransform = terrian.m_transform;

		//メッシュを走査
		for (auto& modelMesh : model->m_meshes)
		{
			//メッシュ情報取得
			auto& mesh = modelMesh.mesh;

			//CastRayに渡す引数を更新。
			castRayArgument.m_mesh = terrian.m_collisionMesh[static_cast<int>(&modelMesh - &model->m_meshes[0])];

			//判定↓============================================


			//右方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_newPos, arg_newPos, m_transform.GetRight(), m_transform.GetScale().x, castRayArgument, RAY_ID::AROUND);

			//左方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_newPos, arg_newPos, -m_transform.GetRight(), m_transform.GetScale().x, castRayArgument, RAY_ID::AROUND);

			//後ろ方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_newPos, arg_newPos, -m_transform.GetFront(), m_transform.GetScale().z, castRayArgument, RAY_ID::AROUND);

			//正面方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_newPos, arg_newPos, m_transform.GetFront(), m_transform.GetScale().z, castRayArgument, RAY_ID::AROUND);

			//下方向にレイを飛ばす。これは地面との押し戻し用。
			CastRay(arg_newPos, arg_newPos, -m_transform.GetUp(), m_transform.GetScale().y, castRayArgument, RAY_ID::GROUND);

			//空中にいるトリガーの場合は崖の処理。
			if (!m_onGround && m_prevOnGround) {

				if (0 < m_moveSpeed.z) {

					//前に進んで崖に落ちた場合。
					CastRay(arg_newPos, arg_from - m_transform.GetUp() * m_transform.GetScale().y, -m_transform.GetFront(), m_transform.GetScale().x, castRayArgument, RAY_ID::CLIFF);

				}

				if (m_moveSpeed.z < 0) {

					//後ろに進んで崖に落ちた場合。
					CastRay(arg_newPos, arg_from - m_transform.GetUp() * m_transform.GetScale().y, m_transform.GetFront(), m_transform.GetScale().x, castRayArgument, RAY_ID::CLIFF);

				}

				if (0 < m_moveSpeed.x) {

					//左に進んで崖に落ちた場合。
					CastRay(arg_newPos, arg_from - m_transform.GetUp() * m_transform.GetScale().y, -m_transform.GetRight(), m_transform.GetScale().z, castRayArgument, RAY_ID::CLIFF);

				}

				if (m_moveSpeed.x < 0) {

					//右に進んで崖に落ちた場合。
					CastRay(arg_newPos, arg_from - m_transform.GetUp() * m_transform.GetScale().y, m_transform.GetRight(), m_transform.GetScale().z, castRayArgument, RAY_ID::CLIFF);

				}

			}


			//=================================================
		}
	}

	//周囲のレイが壁に当たっていなかったら、下方向のレイを姿勢として使用する。
	if (!isHitWall && onGround) {
		hitResult.m_terrianNormal = hitResult.m_bottmRayTerrianNormal;
		isHitWall = true;
	}

	//接地フラグを保存
	m_prevOnGround = m_onGround;
	m_onGround = onGround;

	//当たり判定がtrueなら当たった地形の法線を格納
	if (isHitWall && arg_hitInfo)
	{
		*arg_hitInfo = hitResult;
	}
	else {

		//どことも衝突していなかったら現在の上ベクトルを法線とする。(必ず回転の処理を通るようにするため)
		hitResult.m_bottmRayTerrianNormal = m_transform.GetUp();
		hitResult.m_terrianNormal = m_transform.GetUp();
		*arg_hitInfo = hitResult;
		isHitWall = true;

	}

	return isHitWall;
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
	m_isFlipMove = false;
}

void Player::Init(KuroEngine::Transform arg_initTransform)
{
	m_transform = arg_initTransform;
	m_camController.Init();
	m_cameraRotY = 0;
	m_cameraQ = DirectX::XMQuaternionIdentity();

	m_moveSpeed = KuroEngine::Vec3<float>();
	m_isFlipMove = false;
}

void Player::Update(const std::weak_ptr<Stage>arg_nowStage)
{

	using namespace KuroEngine;

	//プレイヤーの回転をカメラ基準にする。(移動方向の基準がカメラの角度なため)
	m_transform.SetRotate(m_cameraQ);

	auto beforePos = m_transform.GetPos();
	auto newPos = beforePos;
	auto rotate = m_transform.GetRotate();

	//入力された移動量を取得
	m_rowMoveVec = OperationConfig::Instance()->GetMoveVecFuna(XMQuaternionIdentity());	//生の入力方向を取得。プレイヤーを入力方向に回転させる際に、XZ平面での値を使用したいから。

	//落下中は入力を無効化。
	if (!m_onGround) {
		m_rowMoveVec = KuroEngine::Vec3<float>();
	}
	m_moveSpeed += m_rowMoveVec * m_moveAccel;

	//移動速度をクランプ。
	m_moveSpeed.x = std::clamp(m_moveSpeed.x, -m_maxSpeed, m_maxSpeed);
	m_moveSpeed.z = std::clamp(m_moveSpeed.z, -m_maxSpeed, m_maxSpeed);

	//入力された値が無かったら移動速度を減らす。
	if (std::fabs(m_rowMoveVec.x) < 0.001f) {

		m_moveSpeed.x = std::clamp(std::fabs(m_moveSpeed.x) - m_brake, 0.0f, m_maxSpeed) * (std::signbit(m_moveSpeed.x) ? -1.0f : 1.0f);

	}

	if (std::fabs(m_rowMoveVec.z) < 0.001f) {

		m_moveSpeed.z = std::clamp(std::fabs(m_moveSpeed.z) - m_brake, 0.0f, m_maxSpeed) * (std::signbit(m_moveSpeed.z) ? -1.0f : 1.0f);

	}

	//プレイヤーがY-の壁に張り付いているかどうかでX軸の移動方向を反転させる。
	auto moveSpeed = m_moveSpeed;
	if (m_transform.GetUp().y < -0.9f && m_isFlipXinput) {
		moveSpeed.x *= -1.0f;
		moveSpeed.z *= -1.0f;
	}
	else if (-0.9f < m_transform.GetUp().y && m_isFlipXinput) {
		moveSpeed.z *= -1.0f;
	}
	else if (m_isFlipMove) {
		//X軸の動きを反転。Z軸の動きはカメラのクォータニオン側で反転させている。
		moveSpeed.x *= -1.0f;
	}

	//入力された視線移動角度量を取得
	auto scopeMove = OperationConfig::Instance()->GetScopeMove();

	//カメラの回転を保存。
	m_cameraRotY += scopeMove.x;

	//プレイヤーの回転を保存。入力があったときは。
	if (0 < m_rowMoveVec.Length()) {
		//Y-平面に張り付いていたときはZ軸を逆にする。
		if (m_isFlipMove) {
			m_playerRotY = atan2f(m_rowMoveVec.x, m_rowMoveVec.z);
		}
		else {
			m_playerRotY = atan2f(m_rowMoveVec.x, m_rowMoveVec.z);
		}
	}

	//入力が無かったら。
	if (m_moveSpeed.Length() < 0.001f) {
		//GetUpのY軸に応じて移動方向を反転させるかのフラグを切り替える。
		if (m_transform.GetUp().y < -0.9f) {
			m_isFlipMove = true;
		}
		else {
			m_isFlipMove = false;
		}
		m_isFlipXinput = false;
	}

	//ローカル軸の移動方向をプレイヤーの回転に合わせて動かす。
	auto moveAmount = KuroEngine::Math::TransformVec3(moveSpeed, rotate);

	//移動量加算
	newPos += moveAmount;

	//地面に張り付ける用の重力。
	if (!m_onGround) {
		newPos -= m_transform.GetUp() * (m_transform.GetScale().y / 2.0f);
	}

	//当たり判定
	HitCheckResult hitResult;
	if (HitCheckAndPushBack(beforePos, newPos, arg_nowStage.lock()->GetTerrianArray(), &hitResult))
	{

		//法線方向を見るクォータニオン
		auto spin = Math::GetLookAtQuaternion({ 0,1,0 }, hitResult.m_terrianNormal);

		//カメラ目線でY軸回転させるクォータニオン
		DirectX::XMVECTOR ySpin;
		//プレイヤーの移動方向でY軸回転させるクォータニオン
		DirectX::XMVECTOR playerYSpin;

		if (m_isFlipMove && -0.9f <= hitResult.m_terrianNormal.y) {

			//プレイヤーがY-の壁に張り付いている場合はカメラの回転をY+基準からY-基準に切り替える。
			ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_cameraRotY + DirectX::XM_PI);

			//プレイヤーの移動方向でY軸回転させるクォータニオン。移動方向に回転しているように見せかけるためのもの。
			playerYSpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_playerRotY + DirectX::XM_PI);

		}
		else if (m_isFlipMove) {

			//プレイヤーがY-の壁に張り付いている場合はカメラの回転をY+基準からY-基準に切り替える。
			ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, -m_cameraRotY + DirectX::XM_PI);

			//プレイヤーの移動方向でY軸回転させるクォータニオン。移動方向に回転しているように見せかけるためのもの。
			playerYSpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_playerRotY + DirectX::XM_PI);

		}
		else if (hitResult.m_terrianNormal.y < -0.9f) {

			//プレイヤーがY-の壁に張り付いている場合はカメラの回転をY+基準からY-基準に切り替える。
			ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, -m_cameraRotY);

			//プレイヤーの移動方向でY軸回転させるクォータニオン。移動方向に回転しているように見せかけるためのもの。
			playerYSpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_playerRotY);

		}
		else {

			ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_cameraRotY);

			//プレイヤーの移動方向でY軸回転させるクォータニオン。移動方向に回転しているように見せかけるためのもの。
			playerYSpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_playerRotY);
		}

		bool isPlayerCeil = m_transform.GetUp().y < -0.9f;
		bool isFlipNormalToCeil = 0 != m_rowMoveVec.x && !m_isFlipMove && hitResult.m_terrianNormal.y < -0.9f && !isPlayerCeil;
		bool isFlipCeilToNormal = 0 != m_rowMoveVec.x && m_isFlipMove && -0.9f <= hitResult.m_terrianNormal.y && isPlayerCeil;
		if (isFlipNormalToCeil || isFlipCeilToNormal) {
			m_isFlipXinput = true;
		}

		//カメラ方向でのクォータニオンを求める。進む方向などを判断するのに使用するのはこっち。Fの一番最初にこの値を入れることでplayerYSpinの回転を打ち消す。
		m_cameraQ = DirectX::XMQuaternionMultiply(spin, ySpin);

		//プレイヤーの移動方向でY軸回転させるクォータニオンをカメラのクォータニオンにかけて、プレイヤーを移動方向に向かせる。
		m_moveQ = DirectX::XMQuaternionMultiply(m_cameraQ, playerYSpin);
		m_transform.SetRotate(m_moveQ);

	}

	//座標変化適用
	m_transform.SetPos(newPos);
	m_ptLig.SetPos(newPos);

	//カメラ操作
	m_camController.Update(scopeMove, newPos);

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

	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model,
		m_transform,
		drawParam);

	for (auto& index : m_debugTransform) {
		BasicDraw::Instance()->Draw(
			arg_cam,
			arg_ligMgr,
			m_model,
			index);
	}

	/*KuroEngine::DrawFunc3D::DrawNonShadingModel(
		m_axisModel,
		m_transform,
		arg_cam);*/

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

Player::MeshCollisionOutput Player::MeshCollision(const KuroEngine::Vec3<float>& arg_rayPos, const KuroEngine::Vec3<float>& arg_rayDir, std::vector<Terrian::Polygon>& arg_targetMesh, KuroEngine::Transform arg_targetTransform) {


	/*===== メッシュとレイの当たり判定 =====*/


	/*-- ① ポリゴンを法線情報をもとにカリングする --*/

	//法線とレイの方向の内積が0より大きかった場合、そのポリゴンは背面なのでカリングする。
	for (auto& index : arg_targetMesh) {

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

void Player::CastRay(KuroEngine::Vec3<float>& arg_charaPos, const KuroEngine::Vec3<float>& arg_rayCastPos, const KuroEngine::Vec3<float>& arg_rayDir, float arg_rayLength, CastRayArgument arg_collisionData, RAY_ID arg_rayID)
{

	/*===== 当たり判定用のレイを撃つ =====*/

	//レイを飛ばす。
	MeshCollisionOutput output = MeshCollision(arg_rayCastPos, arg_rayDir, arg_collisionData.m_mesh, arg_collisionData.m_targetTransform);

	//レイがメッシュに衝突しており、衝突地点までの距離がレイの長さより小さかったら衝突している。
	if (output.m_isHit && std::fabs(output.m_distance) < arg_rayLength) {

		//ぴったり押し戻してしまうと重力の関係でガクガクしてしまうので、微妙にめり込ませて押し戻す。
		static const float OFFSET = 0.01f;

		//レイの種類によって保存するデータを変える。
		switch (arg_rayID)
		{
		case Player::RAY_ID::GROUND:

			//外部に渡す用のデータを保存
			arg_collisionData.m_hitResult.m_interPos = output.m_pos;
			arg_collisionData.m_hitResult.m_bottmRayTerrianNormal = output.m_normal;
			arg_collisionData.m_onGround = true;

			//押し戻す。
			arg_charaPos += output.m_normal * (std::fabs(output.m_distance - arg_rayLength) - OFFSET);

			break;

		case Player::RAY_ID::DEBUG:
			//m_debugTransform.SetPos(output.m_pos);

		case Player::RAY_ID::CLIFF:
		case Player::RAY_ID::AROUND:

			//外部に渡す用のデータを保存。
			arg_collisionData.m_isHitWall = true;
			arg_collisionData.m_hitResult.m_interPos = output.m_pos;
			arg_collisionData.m_hitResult.m_terrianNormal = output.m_normal;

			//レイの衝突地点から法線方向に伸ばした位置に移動させる。
			arg_charaPos = output.m_pos + output.m_normal * (arg_rayLength - OFFSET);

			break;
		default:
			break;
		}
	}

}
