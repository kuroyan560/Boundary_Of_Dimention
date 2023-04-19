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
	//���C�g�̃��[���h���W
	auto worldPos = m_transform.GetPosWorld() - m_transform.GetUpWorld();

	//�����蔻��̑ΏۂƂ̋���
	float distance = KuroEngine::Vec3<float>(arg_center - worldPos).Length();

	//�Փ˔���������Ȃ��B
	bool isHit = distance <= arg_size.Length() + m_influenceRange;

	//�������Ă��Ȃ������珈�����΂��B
	if (!isHit) return false;

	//���ςɂ���ă��C�g�ƃI�u�W�F�N�g�܂ł̊p�x�����߂�B
	float cosAngleDiff = KuroEngine::Math::TransformVec3(m_defaultVec, m_transform.GetRotateWorld()).Dot(KuroEngine::Vec3<float>(arg_center - worldPos).GetNormal());

	//�p�x���K�肳��Ă���l�ȉ��������瓖�����Ă���B
	return acosf(cosAngleDiff) <= m_apertureAngle;

}
