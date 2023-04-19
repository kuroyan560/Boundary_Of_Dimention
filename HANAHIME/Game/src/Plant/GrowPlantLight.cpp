#include "GrowPlantLight.h"
std::vector<GrowPlantLight*>GrowPlantLight::s_lightArray;

bool GrowPlantLight_Point::HitCheckWithBox(KuroEngine::Vec3<float> arg_center, KuroEngine::Vec3<float> arg_size)
{
	return false;
}

bool GrowPlantLight_Spot::HitCheckWithBox(KuroEngine::Vec3<float> arg_center, KuroEngine::Vec3<float> arg_size)
{
	return false;
}
