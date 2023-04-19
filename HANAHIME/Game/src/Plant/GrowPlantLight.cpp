#include "GrowPlantLight.h"
std::vector<GrowPlantLight*>GrowPlantLight::s_lightArray;

bool GrowPlantLight_Point::HitCheckWithBox(KuroEngine::Vec3<float> arg_center, KuroEngine::Vec3<float> arg_size)
{
	//ライトのワールド座標
	auto worldPos = m_transform.GetPosWorld();

	//当たり判定の対象との距離
	float distance = KuroEngine::Vec3<float>(arg_center - worldPos).Length();

	//衝突判定をおこなう。
	return distance <= arg_size.Length() + m_influenceRange;

}

bool GrowPlantLight_Spot::HitCheckWithBox(KuroEngine::Vec3<float> arg_center, KuroEngine::Vec3<float> arg_size)
{
	//ライトのワールド座標
	auto worldPos = m_transform.GetPosWorld() - m_transform.GetUpWorld();

	//当たり判定の対象との距離
	float distance = KuroEngine::Vec3<float>(arg_center - worldPos).Length();

	//衝突判定をおこなう。
	bool isHit = distance <= arg_size.Length() + m_influenceRange;

	//当たっていなかったら処理を飛ばす。
	if (!isHit) return false;

	//内積によってライトとオブジェクトまでの角度を求める。
	float cosAngleDiff = KuroEngine::Math::TransformVec3(m_defaultVec, m_transform.GetRotateWorld()).Dot(KuroEngine::Vec3<float>(arg_center - worldPos).GetNormal());

	//角度が規定されている値以下だったら当たっている。
	return acosf(cosAngleDiff) <= m_apertureAngle;

}
