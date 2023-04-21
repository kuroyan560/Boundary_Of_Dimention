#pragma once
#include"Common/Vec.h"
#include"Common/Transform.h"
#include"Common/Angle.h"
#include<vector>
//�A����ɐB�������
class GrowPlantLight
{
private:
	//�V�[���ɑ��݂��郉�C�g�̔z��
	static std::vector<GrowPlantLight*>s_lightArray;

public:
	enum TYPE { POINT, SPOT };

	//�V�[���ɑ��݂��郉�C�g�̔z�񏉊���
	static void ResetRegisteredLight() { s_lightArray.clear(); }
	static const std::vector<GrowPlantLight*>& GrowPlantLightArray() { return s_lightArray; }

private:
	TYPE m_type;

protected:
	GrowPlantLight(TYPE arg_type) :m_type(arg_type) {}
	//�g�����X�t�H�[��
	KuroEngine::Transform m_transform;
	//�A�N�e�B�u���
	bool m_active = true;

public:
	//�o�^
	void Register()
	{
		s_lightArray.emplace_back(this);
	}

	//�e�̃g�����X�t�H�[���ݒ�
	void SetParent(KuroEngine::Transform* arg_parent)
	{
		m_transform.SetParent(arg_parent);
	}

	//�l�p�Ƃ̓����蔻��
	virtual bool HitCheckWithBox(KuroEngine::Vec3<float>arg_center, KuroEngine::Vec3<float>arg_size) = 0;

	//��ʂ̃Q�b�^
	const TYPE& GetType()const { return m_type; }
};

//�_����
class GrowPlantLight_Point : public GrowPlantLight
{
public:
	//�e���͈́i���a�j
	float m_influenceRange = 8.0f;

	GrowPlantLight_Point(float arg_influenceRange, KuroEngine::Transform* arg_parent = nullptr)
		:m_influenceRange(arg_influenceRange), GrowPlantLight(POINT)
	{
		SetParent(arg_parent);
	}

	//GPU�ɑ��M����ۂ̃t�H�[�}�b�g
	class ConstData
	{
	public:
		KuroEngine::Vec3<float>m_pos = { 0,0,0 };
		float m_influenceRange = 8.0f;
		unsigned int m_active = 1;
		int pad[3];
	};

	//GPU�ɑ��M����f�[�^�`���ɂȂ����ďo��
	ConstData GetSendData()
	{
		ConstData data;
		data.m_pos = m_transform.GetPosWorld();
		data.m_influenceRange = m_influenceRange;
		data.m_active = m_active ? 1 : 0;
		return data;
	}

	//�l�p�Ƃ̓����蔻��
	bool HitCheckWithBox(KuroEngine::Vec3<float>arg_center, KuroEngine::Vec3<float>arg_size)override;
};

//�X�|�b�g���C�g
class GrowPlantLight_Spot : public GrowPlantLight
{
public:
	GrowPlantLight_Spot(float arg_influenceRange, KuroEngine::Angle arg_apertureAngle, KuroEngine::Transform* arg_parent = nullptr) :GrowPlantLight(SPOT)
	{
		m_apertureAngle = arg_apertureAngle;
		m_influenceRange = arg_influenceRange;
		SetParent(arg_parent);
	}

	//�f�t�H���g��ԁi��]���Ȃ��j�ł̌���
	KuroEngine::Vec3<float>m_defaultVec = { 0,1,0 };
	//�e���͈́i�����j
	float m_influenceRange = 20.0f;
	//���̍i��̊p�x
	KuroEngine::Angle m_apertureAngle;

	//GPU�ɑ��M����ۂ̃t�H�[�}�b�g
	class ConstData
	{
	public:
		KuroEngine::Vec3<float>m_pos = { 0,0,0 };
		float m_influenceRange = 10.0f;
		KuroEngine::Vec3<float>m_vec = { 0,1,0 };
		float m_angle = 0.0f;
		unsigned int m_active = 1;
		int pad[3];
	};

	//GPU�ɑ��M����f�[�^�`���ɂȂ����ďo��
	ConstData GetSendData()
	{
		ConstData data;
		data.m_pos = m_transform.GetPosWorld();
		data.m_influenceRange = m_influenceRange;
		data.m_vec = KuroEngine::Math::TransformVec3(m_defaultVec, m_transform.GetRotateWorld());
		data.m_angle = m_apertureAngle;
		data.m_active = m_active ? 1 : 0;
		return data;
	}

	//�l�p�Ƃ̓����蔻��
	bool HitCheckWithBox(KuroEngine::Vec3<float>arg_center, KuroEngine::Vec3<float>arg_size)override;
};