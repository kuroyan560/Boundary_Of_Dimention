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

	}

	//カメラ
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Camera"))
	{
		auto pos = m_cam->GetPos();
		auto target = m_cam->GetGazePointPos();

		// ImGui::DragFloat3("Target", (float*)&target, 0.5f);
		ImGui::DragFloat("Sensitivity", &m_camSensitivity, 0.05f);

		ImGui::TreePop();
	}
}

bool Player::HitCheck(const KuroEngine::Vec3<float>arg_from, KuroEngine::Vec3<float>& arg_to, const std::vector<Terrian>& arg_terrianArray, KuroEngine::Vec3<float>* arg_terrianNormal)
{
	/*
	arg_from … 移動前の座標
	arg_to … 移動後の座標
	arg_terrianArray … 地形の配列
	arg_terrianNormal … 当たった地形のメッシュの法線、格納先
	*/

	//当たり判定結果
	bool isHit = false;
	m_onGround = false;
	KuroEngine::Vec3<float> hitNormal;

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
			CastRay(arg_to, m_transform.GetRight(), modelMesh, terrian.m_transform, isHit, hitNormal, RAY_ID::AROUND);

			//左方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_to, -m_transform.GetRight(), modelMesh, terrian.m_transform, isHit, hitNormal, RAY_ID::AROUND);

			//後ろ方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_to, -m_transform.GetFront(), modelMesh, terrian.m_transform, isHit, hitNormal, RAY_ID::AROUND);

			//正面方向にレイを飛ばす。これは壁にくっつく用。
			CastRay(arg_to, m_transform.GetFront(), modelMesh, terrian.m_transform, isHit, hitNormal, RAY_ID::AROUND);

			//下方向にレイを飛ばす。これは地面との押し戻し用。
			CastRay(arg_to, -m_transform.GetUp(), modelMesh, terrian.m_transform, isHit, hitNormal, RAY_ID::GROUND);


			//=================================================
		}
	}

	//当たり判定がtrueなら当たった地形の法線を格納
	if (isHit && arg_terrianNormal)
	{
		*arg_terrianNormal = hitNormal;
	}
	return isHit;
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

	AddCustomParameter("MoveScalar", { "moveScalar" }, PARAM_TYPE::FLOAT, &m_moveScalar, "Player");
	AddCustomParameter("Sensitivity", { "camera", "sensitivity" }, PARAM_TYPE::FLOAT, &m_camSensitivity, "Camera");
}

void Player::Init(KuroEngine::Transform arg_initTransform)
{
	m_transform = arg_initTransform;
	m_camController.Init();
}

void Player::Update(const std::weak_ptr<Stage>arg_nowStage)
{
	using namespace KuroEngine;

	auto beforePos = m_transform.GetPos();
	auto newPos = beforePos;
	auto rotate = m_transform.GetRotate();

	//入力された移動量を取得
	auto moveVec = OperationConfig::Instance()->GetMoveVec(rotate);
	//入力された視線移動角度量を取得
	auto scopeMove = OperationConfig::Instance()->GetScopeMove();

	//移動量加算
	newPos += moveVec * m_moveScalar;

	if (!m_onGround) {
		newPos.y -= 0.2f;
	}

	//視線移動角度量加算（Y軸：左右）
	auto yScopeSpin = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), scopeMove.x);
	rotate = XMQuaternionMultiply(yScopeSpin, rotate);

	//当たり判定
	Vec3<float>hitTerrianNormal;
	if (HitCheck(beforePos, newPos, arg_nowStage.lock()->GetTerrianArray(), &hitTerrianNormal))
	{
		//当たり判定に基づいて移動を修正
	}

	//トランスフォームの変化を適用
	m_transform.SetPos(newPos);
	m_transform.SetRotate(rotate);

	//カメラ操作
	auto front = m_transform.GetFront();
	m_camController.Update(m_cam, newPos, front);

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
			camTransform.GetWorldMat(),
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

	/*-- ② ポリゴンを法線情報をもとにカリングする --*/

	//法線とレイの方向の内積が0より小さかった場合、そのポリゴンは背面なのでカリングする。	当たり判定をより強度にするために一旦コメントアウトするが、処理負荷問題が発生したら復活させる。
	for (auto& index : checkHitPolygons) {

		if (index.m_p1.normal.Dot(arg_rayDir) < -0.0001f) continue;

		index.m_isActive = false;

	}


	/*-- ③ ポリゴンをワールド変換する --*/

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
		index.m_p0.pos = MulMat(index.m_p0.pos, targetWorldMat);
		index.m_p1.pos = MulMat(index.m_p1.pos, targetWorldMat);
		index.m_p2.pos = MulMat(index.m_p2.pos, targetWorldMat);
		//法線を回転行列分だけ変換
		index.m_p0.normal = MulMat(index.m_p0.normal, targetRotMat);
		index.m_p0.normal.Normalize();
		index.m_p1.normal = MulMat(index.m_p1.normal, targetRotMat);
		index.m_p1.normal.Normalize();
		index.m_p2.normal = MulMat(index.m_p2.normal, targetRotMat);
		index.m_p2.normal.Normalize();
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

		//三角関数を利用して視点から衝突店までの距離を求める
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

inline KuroEngine::Vec3<float> Player::CalBary(const KuroEngine::Vec3<float>& PosA, const KuroEngine::Vec3<float>& PosB, const KuroEngine::Vec3<float>& PosC, const KuroEngine::Vec3<float>& TargetPos)
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

inline KuroEngine::Vec3<float> Player::MulMat(const KuroEngine::Vec3<float>& arg_target, const DirectX::XMMATRIX arg_mat)
{

	/*===== ベクトルに行列を乗算する =====*/

	//乗算する。
	DirectX::XMVECTOR resultVec = DirectX::XMVector3Transform(arg_target, arg_mat);

	//KuroEngine::Vec3<float>になおす。
	KuroEngine::Vec3<float> returnVec = { resultVec.m128_f32[0], resultVec.m128_f32[1], resultVec.m128_f32[2] };

	return returnVec;

}

void Player::CastRay(KuroEngine::Vec3<float>& arg_rayPos, KuroEngine::Vec3<float>& arg_rayDir, KuroEngine::ModelMesh arg_targetMesh, KuroEngine::Transform arg_targetTransform, bool& arg_isHit, KuroEngine::Vec3<float>& arg_hitNormal, RAY_ID arg_rayID)
{

	/*===== 当たり判定用のレイを撃つ =====*/

	//レイを飛ばす。
	MeshCollisionOutput output = MeshCollision(arg_rayPos, arg_rayDir, arg_targetMesh, arg_targetTransform);

	//レイがメッシュに衝突しており、衝突地点までの距離がプレイヤーの大きさより小さかったら衝突している。
	if (output.m_isHit && std::fabs(output.m_distance) <= m_transform.GetScale().x) {

		//ぴったり押し戻してしまうと重力の関係でガクガクしてしまうので、微妙にめり込ませて押し戻す。
		static const float OFFSET = 0.01f;

		//押し戻す。
		arg_rayPos = output.m_pos + output.m_normal * (m_transform.GetScale().x - OFFSET);

		//レイの種類によって保存するデータを変える。
		switch (arg_rayID)
		{
		case Player::RAY_ID::GROUND:

			//接地判定
			m_onGround = true;

			break;
		case Player::RAY_ID::AROUND:

			//外部に渡す用のデータを保存。
			arg_isHit = true;
			arg_hitNormal = output.m_normal;

			break;
		default:
			break;
		}

	}

}
