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
	return false;
}
