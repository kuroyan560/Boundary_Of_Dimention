#include "Player.h"
#include"Render/RenderObject/Camera.h"
#include"../OperationConfig.h"
#include"FrameWork/Importer.h"
#include"ForUser/DrawFunc/3D/DrawFunc3D.h"
#include"../Graphics/BasicDraw.h"
#include"../Stage/Stage.h"

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

	//地形配列走査
	for (auto& terrian : arg_terrianArray)
	{
		//モデル情報取得
		auto model = terrian.m_model.lock();
		//トランスフォーム情報
		auto& transform = terrian.m_transform;

		//メッシュを走査
		for (auto& modelMesh : model->m_meshes)
		{
			//メッシュ情報取得
			auto& mesh = modelMesh.mesh;

			//判定↓============================================


			//右方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_newPos, m_transform.GetRight(), m_transform.GetScale().x, modelMesh, terrian.m_transform, onGround, isHitWall, hitResult, RAY_ID::AROUND);

			//左方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_newPos, -m_transform.GetRight(), m_transform.GetScale().x, modelMesh, terrian.m_transform, onGround, isHitWall, hitResult, RAY_ID::AROUND);

			//後ろ方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_newPos, -m_transform.GetFront(), m_transform.GetScale().z, modelMesh, terrian.m_transform, onGround, isHitWall, hitResult, RAY_ID::AROUND);

			//正面方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_newPos, m_transform.GetFront(), m_transform.GetScale().z, modelMesh, terrian.m_transform, onGround, isHitWall, hitResult, RAY_ID::AROUND);

			//下方向にレイを飛ばす。これは地面との押し戻し用。
			CastRay(arg_newPos, -m_transform.GetUp(), m_transform.GetScale().y, modelMesh, terrian.m_transform, onGround, isHitWall, hitResult, RAY_ID::GROUND);

			//空中にいるトリガーの場合は崖の処理。
			if (!m_onGround && m_prevOnGround) {

				//前に進んで崖に落ちた場合。	*2しているのは、デフォルトのサイズをそのまま使うと移動速度が速すぎて飛び出してしまうから。
				CastRay(arg_newPos - m_transform.GetUp() * m_transform.GetScale().y, -m_transform.GetFront(), m_transform.GetScale().x * 2.0f, modelMesh, terrian.m_transform, onGround, isHitWall, hitResult, RAY_ID::CLIFF);

				//後ろに進んで崖に落ちた場合。
				CastRay(arg_newPos - m_transform.GetUp() * m_transform.GetScale().y, m_transform.GetFront(), m_transform.GetScale().x * 2.0f, modelMesh, terrian.m_transform, onGround, isHitWall, hitResult, RAY_ID::CLIFF);

				//右に進んで崖に落ちた場合。
				CastRay(arg_newPos - m_transform.GetUp() * m_transform.GetScale().y, m_transform.GetRight(), m_transform.GetScale().z * 2.0f, modelMesh, terrian.m_transform, onGround, isHitWall, hitResult, RAY_ID::CLIFF);

				//左に進んで崖に落ちた場合。
				CastRay(arg_newPos - m_transform.GetUp() * m_transform.GetScale().y, -m_transform.GetRight(), m_transform.GetScale().z * 2.0f, modelMesh, terrian.m_transform, onGround, isHitWall, hitResult, RAY_ID::CLIFF);

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
	return isHitWall;
}

Player::Player()
	:KuroEngine::Debugger("Player", true, true)
{
	//モデル読み込み
	m_model = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Player.glb");
	m_axisModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Axis.glb");
	m_camModel = KuroEngine::Importer::Instance()->LoadModel("resource/user/model/", "Camera.glb");

	//カメラ生成
	m_cam = std::make_shared<KuroEngine::Camera>("Player's Camera");
	//カメラのコントローラーにアタッチ
	m_camController.AttachCamera(m_cam);

	AddCustomParameter("MoveScalar", { "moveScalar" }, PARAM_TYPE::FLOAT, &m_moveScalar, "Player");
	AddCustomParameter("Sensitivity", { "camera", "sensitivity" }, PARAM_TYPE::FLOAT, &m_camSensitivity, "Camera");

	m_cameraRotY = 0;
	m_cameraQ = DirectX::XMQuaternionIdentity();
}

void Player::Init(KuroEngine::Transform arg_initTransform)
{
	m_transform = arg_initTransform;
	m_camController.Init();
	m_cameraRotY = 0;
	m_cameraQ = DirectX::XMQuaternionIdentity();
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
	auto moveVec = OperationConfig::Instance()->GetMoveVec(rotate);
	auto rowMoveVec = OperationConfig::Instance()->GetMoveVec(XMQuaternionIdentity());	//生の入力方向を取得。プレイヤーを入力方向に回転させる際に、XZ平面での値を使用したいから。
	//入力された視線移動角度量を取得
	auto scopeMove = OperationConfig::Instance()->GetScopeMove();

	//カメラの回転を保存。
	m_cameraRotY += scopeMove.x;

	//プレイヤーの回転を保存。入力があったときは。
	if (0 < rowMoveVec.Length()) {
		m_playerRotY = atan2f(rowMoveVec.x, rowMoveVec.z);
	}

	//移動量加算
	newPos += moveVec * m_moveScalar;

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
		auto spinMat = DirectX::XMMatrixRotationQuaternion(spin);

		//カメラ目線でY軸回転させるクォータニオン
		auto ySpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_cameraRotY);

		//カメラ方向でのクォータニオンを求める。進む方向などを判断するのに使用するのはこっち。Fの一番最初にこの値を入れることでplayerYSpinの回転を打ち消す。
		m_cameraQ = DirectX::XMQuaternionMultiply(spin, ySpin);

		//プレイヤーの移動方向でY軸回転させるクォータニオン。移動方向に回転しているように見せかけるためのもの。
		auto playerYSpin = DirectX::XMQuaternionRotationNormal(hitResult.m_terrianNormal, m_playerRotY);
		m_transform.SetRotate(DirectX::XMQuaternionMultiply(m_cameraQ, playerYSpin));

	}

	//座標変化適用
	m_transform.SetPos(newPos);

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

	BasicDraw::Instance()->Draw(
		arg_cam,
		arg_ligMgr,
		m_model,
		m_transform);

	KuroEngine::DrawFunc3D::DrawNonShadingModel(
		m_axisModel,
		m_transform,
		arg_cam);

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


Player::MeshCollisionOutput Player::MeshCollision(const KuroEngine::Vec3<float>& arg_rayPos, const KuroEngine::Vec3<float>& arg_rayDir, KuroEngine::ModelMesh arg_targetMesh, KuroEngine::Transform arg_targetTransform) {


	/*===== メッシュとレイの当たり判定 =====*/

	/*-- ① モデル情報から当たり判定用のポリゴンを作り出す --*/

	//当たり判定用ポリゴン
	struct Polygon {
		bool m_isActive;					//このポリゴンが有効化されているかのフラグ
		KuroEngine::ModelMesh::Vertex m_p0;	//頂点0
		KuroEngine::ModelMesh::Vertex m_p1;	//頂点1
		KuroEngine::ModelMesh::Vertex m_p2;	//頂点2
	};

	//当たり判定用ポリゴンコンテナを作成。
	std::vector<Polygon> checkHitPolygons;
	checkHitPolygons.resize(arg_targetMesh.mesh->indices.size() / static_cast<size_t>(3));

	//当たり判定用ポリゴンコンテナにデータを入れていく。
	for (auto& index : checkHitPolygons) {

		// 現在のIndex数。
		int nowIndex = static_cast<int>(&index - &checkHitPolygons[0]);

		// 頂点情報を保存。
		index.m_p0 = arg_targetMesh.mesh->vertices[arg_targetMesh.mesh->indices[nowIndex * 3 + 0]];
		index.m_p1 = arg_targetMesh.mesh->vertices[arg_targetMesh.mesh->indices[nowIndex * 3 + 1]];
		index.m_p2 = arg_targetMesh.mesh->vertices[arg_targetMesh.mesh->indices[nowIndex * 3 + 2]];

		// ポリゴンを有効化。
		index.m_isActive = true;

	}


	/*-- ② ポリゴンをワールド変換する --*/

	//ワールド行列
	DirectX::XMMATRIX targetRotMat = DirectX::XMMatrixRotationQuaternion(arg_targetTransform.GetRotate());
	DirectX::XMMATRIX targetWorldMat = DirectX::XMMatrixIdentity();
	targetWorldMat *= DirectX::XMMatrixScaling(arg_targetTransform.GetScale().x, arg_targetTransform.GetScale().y, arg_targetTransform.GetScale().z);
	targetWorldMat *= targetRotMat;
	targetWorldMat.r[3].m128_f32[0] = arg_targetTransform.GetPos().x;
	targetWorldMat.r[3].m128_f32[1] = arg_targetTransform.GetPos().y;
	targetWorldMat.r[3].m128_f32[2] = arg_targetTransform.GetPos().z;
	for (auto& index : checkHitPolygons) {
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

	/*-- ③ ポリゴンを法線情報をもとにカリングする --*/

	//法線とレイの方向の内積が0より小さかった場合、そのポリゴンは背面なのでカリングする。
	for (auto& index : checkHitPolygons) {

		if (index.m_p1.normal.Dot(arg_rayDir) < -0.0001f) continue;

		index.m_isActive = false;

	}


	/*-- ④ ポリゴンとレイの当たり判定を行い、各情報を記録する --*/

	// 記録用データ
	std::vector<std::pair<Player::MeshCollisionOutput, Polygon>> hitDataContainer;

	for (auto& index : checkHitPolygons) {

		//ポリゴンが無効化されていたら次の処理へ
		if (!index.m_isActive) continue;

		//レイの開始地点から平面におろした垂線の長さを求める
		KuroEngine::Vec3<float> planeNorm = -index.m_p0.normal;
		float rayToOriginLength = arg_rayPos.Dot(planeNorm);
		float planeToOriginLength = index.m_p0.pos.Dot(planeNorm);
		//視点から平面におろした垂線の長さ
		float perpendicularLine = rayToOriginLength - planeToOriginLength;

		//三角関数を利用して視点から衝突点までの距離を求める
		float dist = planeNorm.Dot(arg_rayDir);
		float impDistance = perpendicularLine / -dist;

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


	/*-- ⑤ 記録した情報から最終的な衝突点を求める --*/

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

void Player::CastRay(KuroEngine::Vec3<float>& arg_rayPos, KuroEngine::Vec3<float>& arg_rayDir, float arg_rayLength, KuroEngine::ModelMesh arg_targetMesh, KuroEngine::Transform arg_targetTransform, bool& arg_onGround, bool& arg_isHitWall, HitCheckResult& arg_hitResult, RAY_ID arg_rayID)
{

	/*===== 当たり判定用のレイを撃つ =====*/

	//レイを飛ばす。
	MeshCollisionOutput output = MeshCollision(arg_rayPos, arg_rayDir, arg_targetMesh, arg_targetTransform);

	//レイがメッシュに衝突しており、衝突地点までの距離がレイの長さより小さかったら衝突している。
	if (output.m_isHit && std::fabs(output.m_distance) < arg_rayLength) {

		//ぴったり押し戻してしまうと重力の関係でガクガクしてしまうので、微妙にめり込ませて押し戻す。
		static const float OFFSET = 0.01f;


		//レイの種類によって保存するデータを変える。
		switch (arg_rayID)
		{
		case Player::RAY_ID::GROUND:

			//外部に渡す用のデータを保存
			if (!arg_isHitWall) {/*
				arg_hitResult.m_interPos = output.m_pos;
				arg_hitResult.m_terrianNormal = output.m_normal;*/
			}
			arg_hitResult.m_interPos = output.m_pos;
			arg_hitResult.m_bottmRayTerrianNormal = output.m_normal;
			arg_onGround = true;
			//arg_isHitWall = true;

			//押し戻す。
			arg_rayPos += output.m_normal * (std::fabs(output.m_distance - arg_rayLength) - OFFSET);

			break;

		case Player::RAY_ID::CLIFF:

			//外部に渡す用のデータを保存。
			arg_isHitWall = true;
			arg_hitResult.m_interPos = output.m_pos;
			arg_hitResult.m_terrianNormal = output.m_normal;

			//レイの衝突地点から法線方向に伸ばした位置に移動させる。 /2しているのはレイの長さをあらかじめ二倍にしているから。
			arg_rayPos = output.m_pos + output.m_normal * (arg_rayLength / 2.0f - OFFSET);

			break;

		case Player::RAY_ID::AROUND:

			//外部に渡す用のデータを保存。
			arg_isHitWall = true;
			arg_hitResult.m_interPos = output.m_pos;
			arg_hitResult.m_terrianNormal = output.m_normal;

			//レイの衝突地点から法線方向に伸ばした位置に移動させる。
			arg_rayPos = output.m_pos + output.m_normal * (arg_rayLength - OFFSET);

			break;
		default:
			break;
		}
	}
}

