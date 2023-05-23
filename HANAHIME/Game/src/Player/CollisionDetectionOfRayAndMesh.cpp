#include "CollisionDetectionOfRayAndMesh.h"

CollisionDetectionOfRayAndMesh::MeshCollisionOutput CollisionDetectionOfRayAndMesh::MeshCollision(const KuroEngine::Vec3<float>& arg_rayPos, const KuroEngine::Vec3<float>& arg_rayDir, std::vector<TerrianHitPolygon>& arg_targetMesh) {


	/*===== メッシュとレイの当たり判定 =====*/


	/*-- ① ポリゴンを法線情報をもとにカリングする --*/

	const float CHECK_HIT_MESH_DEADLINE = 150.0f;

	//法線とレイの方向の内積が0より大きかった場合、そのポリゴンは背面なのでカリングする。
	for (auto& index : arg_targetMesh) {

		index.m_isActive = true;

		//反対側を向いていたら無効化。
		if (-0.0001f < index.m_p1.normal.Dot(arg_rayDir)) {

			index.m_isActive = false;
			continue;

		}

		//一定以上離れていたら無効化。
		bool isNear = false;

		isNear |= KuroEngine::Vec3<float>(arg_rayPos - index.m_p0.pos).Length() < CHECK_HIT_MESH_DEADLINE;
		isNear |= KuroEngine::Vec3<float>(arg_rayPos - index.m_p1.pos).Length() < CHECK_HIT_MESH_DEADLINE;
		isNear |= KuroEngine::Vec3<float>(arg_rayPos - index.m_p2.pos).Length() < CHECK_HIT_MESH_DEADLINE;

		if (!isNear) {

			index.m_isActive = false;
			continue;

		}

	}


	/*-- ② ポリゴンとレイの当たり判定を行い、各情報を記録する --*/

	// 記録用データ
	std::vector<std::pair<MeshCollisionOutput, TerrianHitPolygon>> hitDataContainer;

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
		MeshCollisionOutput data;
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

		////重心座標を求める。
		//KuroEngine::Vec3<float> bary = CalBary(hitDataContainer[min].second.m_p0.pos, hitDataContainer[min].second.m_p1.pos, hitDataContainer[min].second.m_p2.pos, hitDataContainer[min].first.m_pos);

		//KuroEngine::Vec3<float> baryBuff = bary;

		////UVWの値がずれるので修正。
		//bary.x = baryBuff.y;
		//bary.y = baryBuff.z;
		//bary.z = baryBuff.x;

		//KuroEngine::Vec2<float> uv = KuroEngine::Vec2<float>();

		////重心座標からUVを求める。
		//uv.x += hitDataContainer[min].second.m_p0.uv.x * bary.x;
		//uv.x += hitDataContainer[min].second.m_p1.uv.x * bary.y;
		//uv.x += hitDataContainer[min].second.m_p2.uv.x * bary.z;

		//uv.y += hitDataContainer[min].second.m_p0.uv.y * bary.x;
		//uv.y += hitDataContainer[min].second.m_p1.uv.y * bary.y;
		//uv.y += hitDataContainer[min].second.m_p2.uv.y * bary.z;

		//hitDataContainer[min].first.m_uv = uv;

		return hitDataContainer[min].first;
	}
	else {

		return MeshCollisionOutput();

	}


}

KuroEngine::Vec3<float> CollisionDetectionOfRayAndMesh::CalBary(const KuroEngine::Vec3<float>& PosA, const KuroEngine::Vec3<float>& PosB, const KuroEngine::Vec3<float>& PosC, const KuroEngine::Vec3<float>& TargetPos)
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