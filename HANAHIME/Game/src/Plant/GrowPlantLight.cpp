#include "GrowPlantLight.h"
std::vector<GrowPlantLight*>GrowPlantLight::s_lightArray;

bool GrowPlantLight_Point::HitCheckWithBox(KuroEngine::Vec3<float> arg_center, KuroEngine::Vec3<float> arg_size)
{
	//���C�g�̃��[���h���W
	auto worldPos = m_transform.GetPosWorld();

	//�����蔻��̑ΏۂƂ̋���
	float distance = KuroEngine::Vec3<float>(arg_center - worldPos).Length();

	//�Փ˔���������Ȃ��B
	return distance <= arg_size.Length() + m_influenceRange;

}

bool GrowPlantLight_Spot::HitCheckWithBox(KuroEngine::Vec3<float> arg_center, KuroEngine::Vec3<float> arg_size)
{
	return false;
}
